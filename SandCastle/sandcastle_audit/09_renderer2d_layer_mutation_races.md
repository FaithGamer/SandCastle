# 09 — Renderer2D layer-mutation APIs race the render thread

**Severity:** Tier 2
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation reading `m_layers[i].material` during render, or GL error inside `glUniform*` when a uniform is rewritten mid-draw.

## What's broken

`Renderer2D` queues most state changes onto its render-thread `WorkerThread` (e.g. `AddLayer` → `AddLayerThread`). But several layer mutators directly write `m_layers[...]` from the calling thread (typically main) AND issue raw GL calls. The render thread reads `m_layers` and the affected shader programs every frame in `Begin`/`RenderLayers`/`Flush`.

## Where (agent-reported, recommend verifying line numbers before fixing)

- `src/Render/Renderer2D.cpp:317-322` — `SetLayerMaterial`:
  ```cpp
  void Renderer2D::SetLayerMaterial(LayerID layer, Material* material) {
      auto ins = Instance();
      ins->SetShaderUniformSampler(material->GetShader(), MAX_TEXTURE_INDEX);  // GL call
      ins->m_layers[layer].material = material;                                 // m_layers write
  }
  ```
- `src/Render/Renderer2D.cpp:253-272` — `SetLayerScreenSpace`: builds a `VertexArray` (GL) on calling thread, writes `m_layers[layer]`.
- `src/Render/Renderer2D.cpp:324-331` — `SetLayerHeight`: writes `m_layers[layer]`.
- `src/Render/Renderer2D.cpp:212-215` — `SetLayerSortZ`: writes the `zsort` `unordered_map`; the render thread reads via `operator[]` in `Sort` (which can insert and rehash on miss).

## Why it's wrong

`m_layers` is iterated on the render thread in:
- `Begin()` (`Renderer2D.cpp:506`): `for (auto& layer : m_layers)`
- `RenderLayers` / `End`: `layer->material->Bind()`, `layer->target->Bind()` reads

Concurrent main-thread writes to `m_layers[i]` (a struct with several fields) can be torn — render thread reads a half-updated `RenderLayer`. The GL call from `SetLayerMaterial` runs on the wrong thread with the wrong context current; even if the context is shared, modifying program uniforms while the render thread is mid-`glDrawElements` with that program is undefined per the GL spec.

`SetLayerSortZ` is worse: `unordered_map::operator[]` may insert (default-constructed) and rehash. If the render thread is also walking the same map in `Sort`, a rehash invalidates the reader's buckets mid-iteration.

## Why it scales with entity count

More entities → more time spent in the render-thread layer loop per frame → wider race window. Setups that swap materials at gameplay events (e.g. damage flash, slow-mo post-processing) trigger this regularly during stress.

## Suggested fix

Route every layer mutator through `m_queue.thread.Queue(...)` so it runs on the render thread between frames — exactly the pattern `AddLayer` already uses:

```cpp
void Renderer2D::SetLayerMaterial(LayerID layer, Material* material) {
    Instance()->m_queue.thread.Queue([layer, material]() {
        auto ins = Instance();
        ins->SetShaderUniformSampler(material->GetShader(), MAX_TEXTURE_INDEX);
        ins->m_layers[layer].material = material;
    });
}
```
Same shape for `SetLayerScreenSpace`, `SetLayerHeight`, `SetLayerSortZ`.

For `zsort`, also replace `std::unordered_map<LayerID, bool>` with `std::array<std::atomic<bool>, MAX_LAYERS>`. Cleaner, fixed-size, atomic, no rehash.

## Verify before fixing

```
grep -n "SetLayer\|m_layers\|zsort\|RenderLayer" SandCastle/src/Render/Renderer2D.cpp SandCastle/include/SandCastle/Render/Renderer2D.h
```

Check whether any test or sample relies on the layer mutator being synchronous (i.e. effect visible the same frame). If so, the queued version will lag by one frame; acceptable for most cases but document it.

## Context for a fresh session

- `AddLayer` (which queues onto the worker) is the right model.
- The render thread loop is `RenderThread()` at `src/Render/Renderer2D.cpp:390+`.
- This is closely related to `02_camera_main_toctou.md` (snapshot main-thread state into render-thread payload).
