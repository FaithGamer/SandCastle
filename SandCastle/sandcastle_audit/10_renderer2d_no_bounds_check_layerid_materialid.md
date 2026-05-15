# 10 — Renderer2D::PushQuad / DrawQuad has no bounds check on LayerID / MaterialID

**Severity:** Tier 2
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside `Renderer2D::DrawQuad` (read or write off the end of `m_batches[layerID]`). Looks "random" because the corrupted memory varies per run.

## What's broken

`LayerID` is `unsigned char` (0..255). `MaterialID` is also a small int. The push and draw paths index into fixed-size arrays of `MAX_LAYERS = 32` with no bounds check. A single sprite with a stale or uninitialized `m_layer` (e.g. an entity whose memory was reused after destroy) writes off the end of the queue every frame, thousands of times.

## Where (agent-reported, recommend verifying)

- `include/SandCastle/Render/Renderer2D.h:104` (size declaration):
  ```cpp
  std::vector<QuadRenderData> _queues[2][MAX_LAYERS];
  ```
- `include/SandCastle/Render/Renderer2D.h:66` (`RenderQueue::Push`):
  ```cpp
  _queues[!_current][data.layerID].emplace_back(...)
  ```
- `src/Render/Renderer2D.cpp:635` (`DrawQuad`):
  ```cpp
  auto& batch = m_batches[(size_t)quad.layerID][(size_t)quad.materialID];
  ```
- `src/Render/SpriteRender.cpp:33` (`SpriteRender::SetLayer`) — stores layer value with no validation.

## Why it's wrong

`LayerID` (unsigned char) can hold 0..255. The queue arrays are size 32. There is no clamp anywhere on the path from `SpriteRender::SetLayer` to `Push` to `DrawQuad`. A bug elsewhere that produces a corrupt layer id (uninitialized component memory after pool relocation, stale data after entity destroy, off-by-one in user code) writes one OOB index per quad pushed.

## Why it scales with entity count

Per-frame, the offending component runs through `SpriteRenderSystem::LateUpdate` once → `Push` once. So a single corrupt sprite is one OOB per frame. With 15k entities, the probability that some sprite has gotten into a bad state (after years of code changes) is much higher. Once one does, the corruption is silent until something tries to use the memory clobbered by the OOB write — typically a separate vector elsewhere in `Renderer2D`.

## Suggested fix

Three layered defenses:

1. **In `SpriteRender::SetLayer`:**
   ```cpp
   void SpriteRender::SetLayer(LayerID layer) {
       ASSERT_LOG_ERROR(layer < Renderer2D::GetLayerCount(), "Layer {0} out of range", (int)layer);
       m_layer = layer;
   }
   ```

2. **In `RenderQueue::Push`:**
   ```cpp
   void Push(QuadRenderData& data) {
       if (data.layerID >= MAX_LAYERS) {
           LOG_ERROR("PushQuad: layerID {0} >= MAX_LAYERS", (int)data.layerID);
           return;
       }
       _queues[!_current][data.layerID].emplace_back(data);
   }
   ```

3. **In `DrawQuad`:**
   ```cpp
   if (quad.layerID >= MAX_LAYERS) return;
   auto& layerBatches = m_batches[quad.layerID];
   if (quad.materialID >= layerBatches.size()) return;  // or grow the vector
   ```

Even one of these (the `Push` guard) is enough to stop the OOB write. All three give defense in depth.

## Verify before fixing

```
grep -n "MAX_LAYERS\|LayerID\|materialID" SandCastle/include/SandCastle/Render/Renderer2D.h SandCastle/src/Render/Renderer2D.cpp
grep -n "SetLayer\|m_layer" SandCastle/src/Render/SpriteRender.cpp
```

Run with the guard and check the logs — if you see "layerID X >= MAX_LAYERS" messages, you've found the upstream source of the corruption.

## Context for a fresh session

- `Renderer2D` ctor pre-fills `zsort[0..MAX_LAYERS-1]` (`Renderer2D.cpp:55-58`), implying the design intent is "use the full 0..MAX_LAYERS-1 range with no guard". Add guards anyway — defense in depth is cheap here.
- `MAX_OFF_LAYERS = 15` is referenced too; offscreen layers may use a separate range. Audit `AddLayer` / `AddOffscreenLayer` to confirm what ranges are actually valid at runtime.
