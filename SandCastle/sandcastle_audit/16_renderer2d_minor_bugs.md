# 16 — Renderer2D minor bugs (m_renderLayers dangling, GetLayers off-by-one, OnWindowResize TOCTOU)

**Severity:** Tier 3 (latent / narrow)
**Engine:** `C:/dev/SandCastle/SandCastle/`

Three small Renderer2D issues, each independent. None are likely current crash causes; all should be fixed when you're in the file anyway.

---

## 16A — m_renderLayers stores invalidated pointers

**Crash signature:** would be access violation if anyone iterates `m_renderLayers`. Currently silent because no reader exists.

`src/Render/Renderer2D.cpp:461-462` (`AddLayerThread`):
```cpp
m_layers.push_back(RenderLayer(...));
m_renderLayers.push_back(&m_layers.back());
```

`m_layers` is `std::vector<RenderLayer>` (header decl ~`Renderer2D.h:295`). Each `push_back` may reallocate and invalidate every previous `&m_layers.back()` stored in `m_renderLayers`. With multiple `AddLayer` calls, every entry except the most recent is a dangling pointer.

**Fix:** either delete the unused `m_renderLayers` vector entirely, or store `LayerID` (an index) and resolve via `&m_layers[i]` at point of use.

---

## 16B — GetLayers loop writes off the end of a too-small vector

**Crash signature:** access violation on first call to GetLayers; currently silent because no caller exists.

`src/Render/Renderer2D.cpp:290-300` (approximate — verify):
```cpp
auto layerCount = m_layers.size();
std::vector<uint32_t> layers(layerCount - 1);   // size = layerCount - 1
for (size_t i = 1; i < layerCount; i++)
    layers[i] = ...;                            // i goes up to layerCount - 1, but layers.size() is layerCount - 1
```

Index `layerCount - 1` is one past the end of a vector sized `layerCount - 1`. Off-by-one.

**Fix:** either resize to `layerCount` and start `i = 0`, or rewrite as `push_back` loop:
```cpp
std::vector<uint32_t> layers;
layers.reserve(layerCount);
for (size_t i = 0; i < layerCount; i++)
    layers.push_back(...);
```

---

## 16C — OnWindowResize: Window::GetSize read on render thread without snapshot

**Crash signature:** very rare TOCTOU during window resize while a heavy frame is in flight.

`src/Render/Renderer2D.cpp:835-839`:
```cpp
void Renderer2D::OnWindowResize(Vec2u size) {
    Wait();
    m_queue.thread.Queue(&Renderer2D::OnWindowResizeThread, this);
}
```

`OnWindowResizeThread` (`Renderer2D.cpp:419-440`) calls `auto windowSize = Window::GetSize();` on the render thread. `OnWindowResize` is called from the main thread when SDL fires the resize event. `Wait()` drains the previous queue, but between `Wait()` returning and `OnWindowResizeThread` actually executing, the main thread could resize again — `Window::m_size` mutates. The new value is racy.

**Fix:** pass the resize size as a payload through the queue:
```cpp
void Renderer2D::OnWindowResize(Vec2u size) {
    Wait();
    Vec2u snapshot = size;  // (already a value)
    m_queue.thread.Queue([this, snapshot]() { this->OnWindowResizeThread(snapshot); });
}
```
And update `OnWindowResizeThread` to take `Vec2u` instead of re-reading `Window::GetSize()`.

This same pattern (snapshot main-thread state into render-thread payload) is the fix shape for file 02.

---

## Verify before fixing

```
grep -n "m_renderLayers\|GetLayers\|OnWindowResize" SandCastle/src/Render/Renderer2D.cpp SandCastle/include/SandCastle/Render/Renderer2D.h
```

All three are obvious from the code. The most likely "real" hazard is 16A — defensive to fix it even if currently unused, since "currently unused" rots into "someone added a debug overlay" later.
