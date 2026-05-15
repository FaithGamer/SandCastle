# 07 — Engine shutdown destroys textures after GL context is gone (no Assets::Kill())

**Severity:** Tier 2
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation on exit inside `nvoglv64.dll` / `atioglxx.dll` / `opengl32.dll`. Worse after long sessions or large asset sets.

## What's broken

`Engine::Launch()` kills subsystems in this order:
```
Renderer2D::Wait();
Inputs::Kill();
Systems::Kill();
Ui::Kill();
Renderer2D::Kill();   // GL render thread torn down
Window::Kill();       // GL contexts destroyed
```
There is **no `Assets::Kill()` call**. The `Assets` singleton (holder of every `Texture`, `Shader`, `Material`, `Animation`) survives until C++ static-destructor teardown — which happens after `Renderer2D::Kill()` and `Window::Kill()`. At that point, `~Texture` calls `glDeleteTextures(...)`, `~Shader` calls `glDeleteProgram(...)`, etc., against a destroyed context. Most drivers crash here.

## Where (verified)

`src/Engine.cpp:117-128`:
```cpp
while (play) { ... }

Renderer2D::Instance()->Wait();

Inputs::Kill();
Systems::Kill();
Ui::Kill();
Renderer2D::Kill();
Window::Kill();
#ifdef SC_IMGUI
ImGuiLoader::ExitImGui();
#endif
```

`src/Render/Texture.cpp:277` — `~Texture` calls `glDeleteTextures` / `glDeleteBuffers`.

## Why it's wrong

GL objects must be deleted with a live GL context current. Static-destructor order in C++ is reverse-of-construction within a translation unit and unspecified across translation units. The Assets singleton's `unordered_map<String, Texture>` (or similar) gets destroyed at process exit with no guarantee the GL context still exists.

## Why it scales with entity count (sort of)

This is a shutdown-only crash, not a runtime crash. But "bigger atlas / more loaded sprites / more font glyphs baked into pages" all make the teardown longer and increase the surface area of dead-context GL calls.

## Suggested fix

Add an explicit `Assets::Kill()` between `Systems::Kill()` and `Renderer2D::Kill()`:

```cpp
Renderer2D::Instance()->Wait();

Inputs::Kill();
Systems::Kill();
Ui::Kill();
Assets::Kill();          // <-- ADD: drop textures/shaders while context is live
Renderer2D::Kill();
Window::Kill();
#ifdef SC_IMGUI
ImGuiLoader::ExitImGui();
#endif
```

Confirm `Assets::Kill()` exists (via the `Singleton<T>::Kill()` CRTP base — see `Internal/Singleton.h`). If `~Assets` exists, audit that it explicitly destroys textures/shaders rather than relying on `unordered_map<String, Texture>` destructor order at end-of-translation-unit.

Also audit the `Writer` (held by `Ui`): font atlases are `sptr<Texture>` inside `Font.atlases`. `Ui::Kill()` runs BEFORE `Renderer2D::Kill()` so atlas Texture destructors happen with the context live — currently OK. After your fix, double-check.

## Verify before fixing

```
grep -n "Assets::Kill\|~Assets\|Singleton<Assets>" SandCastle/include SandCastle/src -r
grep -n "Kill();" SandCastle/src/Engine.cpp
```

Then test: run the app, exit cleanly, check for "GL context is not current" warnings in any debug output. If you have a driver debug callback wired (KHR_debug), it'll be very loud.

## Context for a fresh session

- `Singleton<T>::Kill()` is in `include/SandCastle/Internal/Singleton.h`.
- Other singletons (Audio, Physics, Inputs, etc.) may have analogous teardown order issues — audit while you're in there.
- One-line fix, very low risk if `Assets::Kill()` is well-defined.
