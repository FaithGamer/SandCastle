# 15 — Texture::Copy leaks GL textures, PBOs, and sync fences

**Severity:** Tier 3
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** GL handle exhaustion after long sessions (especially many language toggles or asset hot-reloads); on AMD drivers, can manifest as black textures or driver-side hangs.

## What's broken

`Texture::Copy` is used by the localization path (`Assets::ChangeLocaTexture`) to replace a texture's contents with another's. The current implementation just copies fields — the GL handle (`m_id`), PBO ids (`m_pbos`), and pending fence (`m_pendingFence`) of the destination texture are silently overwritten without being freed first.

## Where (agent-reported, verify lines)

`src/Render/Texture.cpp:262-275` (`Texture::Copy`):
```cpp
m_id = texture.m_id;          // previous m_id leaked (no glDeleteTextures)
m_pbos[0] = texture.m_pbos[0]; // previous PBOs leaked
m_pbos[1] = texture.m_pbos[1];
m_pendingFence = texture.m_pendingFence;  // previous fence leaked (no glDeleteSync)
// no cleanup of the prior values
```

## Why it's wrong

GL handles (textures, buffers, sync objects) are server-side resources that must be explicitly released via `glDeleteTextures`/`glDeleteBuffers`/`glDeleteSync`. Overwriting the CPU-side id without calling delete leaks the GPU-side object.

Sync objects are particularly bad: drivers may impose a low cap on outstanding fences (a few thousand). Once exhausted, `glFenceSync` starts returning 0 and rendering becomes incorrect.

## Why it scales with entity count (sort of)

This is per-language-toggle (or per-asset-reload), not per-entity. But long sessions accumulate the leaks. In dev workflows that toggle language frequently, the GL handle pool drains quickly.

## Suggested fix

```cpp
void Texture::Copy(const Texture& other) {
    // Release previous GL resources before overwriting handles
    WaitIfPending();   // already does fence cleanup
    if (m_id) glDeleteTextures(1, &m_id);
    if (m_pbos[0]) glDeleteBuffers(2, m_pbos);

    m_id = other.m_id;
    m_pbos[0] = other.m_pbos[0];
    m_pbos[1] = other.m_pbos[1];
    // ... other fields ...
}
```

But this only works on the render thread (these are GL calls). If `Copy` is currently called on the main thread (audit `Assets::ChangeLocaTexture` to confirm), queue the delete onto `Renderer2D::m_queue.thread`.

Long-term: track ownership explicitly. Most uses of `Copy` are "swap source ownership in for the destination" — i.e. transfer, not copy. Add an explicit `bool m_owns` flag and only delete owned handles. Or refactor to use `std::shared_ptr<GLTextureHandle>` so handle reference-counting handles the cleanup automatically.

## Verify before fixing

```
grep -n "Texture::Copy\|texture.Copy\|->Copy(" SandCastle/src/ SandCastle/include/
grep -n "ChangeLocaTexture\|SetLang" SandCastle/src/Core/Assets.cpp
```

Confirm what thread `Copy` is currently called from. Check whether any caller relies on the old GL handle remaining valid after Copy (it shouldn't — copy means transfer).

## Context for a fresh session

- Easy to reproduce: load any localized texture asset, toggle language many times, monitor GL texture handle count via a debug callback.
- Related to file 07 (Engine shutdown missing Assets::Kill) — both involve GL resource ownership.
- Low priority unless your project actually changes languages at runtime; if it doesn't, this never fires.
