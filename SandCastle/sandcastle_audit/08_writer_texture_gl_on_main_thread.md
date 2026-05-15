# 08 — Writer / Texture::UpdateRegion runs raw GL on main thread

**Severity:** Tier 2
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** flickering glyphs, black textures, GL_INVALID_OPERATION storms, or sporadic access violations inside the GPU driver. Scales with text label count.

## What's broken

The render thread owns the GL context (`docs/RENDERING.md:7-12`). But every `Ui::Text(...)` / `writer->Write(...)` call made on the main thread runs `EnsureGlyphs` → `BakeOneGlyph` → `Texture::UpdateRegion(...)`, which issues raw GL calls. `Writer` may also allocate a brand-new atlas page via `makesptr<Texture>(side, side, tis)`, which runs `glGenTextures`/`glTexImage2D`/`glTexParameteri`/`glFenceSync` on the calling thread.

If the GL context is not current on the main thread (it isn't, per Renderer2D's `SDL_GL_MakeCurrent` on the render thread only), these calls hit undefined GL state.

## Where (verified)

`src/Render/Texture.cpp:359-409` (`UpdateRegion`):
```cpp
glBindTexture(GL_TEXTURE_2D, m_id);
...
glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
glBufferData(GL_PIXEL_UNPACK_BUFFER, ..., GL_STREAM_DRAW);
glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, regionSize, ...);
memcpy(dst, rgba8, regionSize);
glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
glBindTexture(GL_TEXTURE_2D, m_id);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)0);
if (m_importSettings.useMipmaps)
    glGenerateMipmap(GL_TEXTURE_2D);
```

`src/Render/Writer.cpp:864, 1029, 1078` — `tex->UpdateRegion(pos.x, pos.y, reqW, reqH, ...);` called from `BakeOneGlyph` / `BakeFallbackGlyph`.

`src/Render/Writer.cpp:711` — `makesptr<Texture>(side, side, tis)` constructs new atlas pages (calls `GenerateEmpty` → `glGenTextures`/`glTexImage2D`).

`src/Render/Writer.cpp:143` — Writer constructor calls `glGetIntegerv(GL_MAX_TEXTURE_SIZE, ...)`.

## Why it's wrong

Per `docs/RENDERING.md`: "Game code never issues GL calls". The Writer/Texture path silently violates this. On NVIDIA drivers it sometimes "works" because GL state changes from a non-current context degrade gracefully (commands silently dropped). On AMD/Intel it crashes or corrupts. Behavior also varies based on whether the render thread happens to be inside `glDrawElements` at the same instant.

## Why it scales with entity count

15k units → potentially many per-unit labels (health bars, damage popups, gamepad cursor labels, name tags). Every label issues at least one `UpdateRegion`. More labels = more concurrent GL command racing the render thread = higher crash probability. Even without crashes, font corruption is visible.

## Suggested fix

Route texture creation and uploads through the render thread's queue, like `Renderer2D::CreateSubTexture` does.

1. **Atlas allocation:** wrap `makesptr<Texture>(side, side, tis)` in a `Renderer2D::CreateAtlasPage(...)` that queues onto `m_queue.thread` and `Wait()`s. Or refactor Writer to allocate atlases eagerly at boot, sized for worst case (e.g. enough for all glyphs of the ASCII range × all loaded fonts).

2. **UpdateRegion:** add `Renderer2D::QueueTextureUpdate(Texture* tex, int x, int y, int w, int h, std::vector<uint8_t> data)` that queues a task running `tex->UpdateRegion(...)` on the render thread. Writer calls this instead.

3. **Bonus:** the Writer constructor's `glGetIntegerv(GL_MAX_TEXTURE_SIZE, ...)` should run on the render thread too. Easiest fix: cache the value in `Renderer2D` after `PostAssetInit` and expose `Renderer2D::GetMaxTextureSize()`.

Trade-off: glyph baking is now asynchronous. Callers of `Writer::Write` may need to wait for glyphs before rendering this frame. Easiest: have `Write` flush the queue synchronously (`m_queue.thread.Wait()` after the bake task is queued). Latency is per-glyph, but you bake once per glyph for the app's lifetime — acceptable.

## Verify before fixing

```
grep -n "gl[A-Z][a-zA-Z]*\|Update\|Generate" SandCastle/src/Render/Writer.cpp
grep -n "gl[A-Z]" SandCastle/src/Render/Texture.cpp | head -50
```

Identify every Writer code path that touches GL. Trace which call paths can hit them from main thread (`Ui::Text*`, `Ui::AnimButton*`, etc.).

## Context for a fresh session

- `Renderer2D::CreateSubTexture` (`src/Render/Renderer2D.cpp:494-497, 625-632`) is the existing pattern for "queue a GL op, wait for result". Mirror it.
- `Texture::WaitIfPending` (`Texture.cpp:411-417`) handles the PBO fence wait; this works fine cross-thread but doesn't help here since UpdateRegion is the one issuing the upload.
- A useful intermediate step: add a runtime assert in `Texture::UpdateRegion` and `Texture::Generate*` that checks `std::this_thread::get_id() == Renderer2D::GetRenderThreadId()`. Will catch every violation immediately during development.
