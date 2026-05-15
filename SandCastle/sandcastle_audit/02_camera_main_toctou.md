# 02 — Renderer2D Camera::main TOCTOU between Process() and Begin()

**Severity:** Tier 1
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside `Renderer2D::Begin()` dereferencing a null or freed `Camera*`. Stack trace shows `camera->GetProjectionMatrix()` or similar. Intermittent, biased toward state transitions.

## What's broken

`Renderer2D::Process()` checks `Camera::main` for null on the main thread, then queues `RenderThread` onto a WorkerThread. The render thread later re-reads `Camera::main` inside `Begin()` and dereferences it. The two reads are not synchronized — anything that nulls or replaces `Camera::main` on the main thread between them crashes the render thread.

## Where (verified)

1. `src/Render/Renderer2D.cpp:846-855` (`Process()`, main thread):
   ```cpp
   void Renderer2D::Process()
   {
       if (!Window::GetRenderWhenMinimized() && Window::GetMinimized())
           return;
       if (Camera::main == nullptr)        // ← check
           return;
       Wait();
       m_queue.Swap();
       m_queue.thread.Queue(&Renderer2D::RenderThread, this);  // returns immediately
   }
   ```
2. `src/Render/Renderer2D.cpp:498-520` (`Begin()`, render thread, runs LATER):
   ```cpp
   auto camera = Camera::main;             // ← re-read raw pointer
   ...
   m_sceneUniform.camProjView = camera->GetProjectionMatrix() * camera->GetViewMatrix();
   m_sceneUniform.camZoom = camera->zoom * 2.f;
   ...
   ```
3. `include/SandCastle/Render/Camera.h:115` — `Camera::main` is a plain static raw pointer (`static Camera* main;`), no synchronization.

## Why it's wrong

Classic TOCTOU. Process() promises "camera is non-null at the time I queue", but Begin() runs at an unbounded later point on the render thread. Heavier frames widen the window. State transitions, Camera destruction, or any `Camera::main = X` assignment can land between the two reads. Even if `Camera::main` is still non-null, it may now point at a Camera whose owner has been destroyed (raw pointer, no ref count).

## Why it scales with entity count

More entities → more quads pushed → longer render-thread frames → wider gap between Process()'s check and Begin()'s deref → higher probability of a main-thread Camera mutation falling inside the gap.

## Suggested fix

Snapshot every value Begin() needs from Camera on the main thread inside Process(), and pass it through the queued task instead of re-reading static state on the render thread.

```cpp
struct FrameInputs {
    Mat4 projView;
    float camZoom;
    float camAspectRatio;
    Vec2u targetSize;
    Camera::Constraints constraints;
    float reduction;
    Vec2u windowSize;
};

void Renderer2D::Process() {
    if (...minimized...) return;
    if (Camera::main == nullptr) return;
    FrameInputs in {
        Camera::main->GetProjectionMatrix() * Camera::main->GetViewMatrix(),
        Camera::main->zoom,
        Camera::main->GetAspectRatio(),
        Camera::main->GetTargetSize(),
        Camera::main->GetConstraints(),
        Camera::main->GetReduction(),
        Window::GetSize(),
    };
    Wait();
    m_queue.Swap();
    m_queue.thread.Queue([this, in]() { this->RenderThread(in); });
}
```

Then `Begin()` consumes `FrameInputs` instead of `Camera::main`. The render thread never reads the static.

Alternative (less invasive): make `Camera::main` a `std::atomic<Camera*>` AND extend Camera lifetime guarantees (e.g. pin to a `sptr`). This still races on Camera destruction without lifetime guarantees, so the snapshot approach is preferred.

## Verify before fixing

```
grep -n "Camera::main" SandCastle/src/ SandCastle/include/ -r
grep -n "Camera::main" madman/src/ -r
```

Make sure no other code path reads `Camera::main` on the render thread; if so, fix those too.

## Context for a fresh session

- `Window::GetSize()`, `Window::GetMinimized()` may also race — audit if needed.
- `docs/RENDERING.md` documents the main/render thread split; this is the canonical example of why the contract needs explicit snapshots, not "trust the static".
- `Camera::main` is initialized in `Systems::Init` (`Systems.cpp:27`) and may be reassigned by client code via `Camera::main = &myCamera;`.
