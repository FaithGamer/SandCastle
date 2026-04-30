# Assets

`Assets` is the asset registry. Game code never loads files directly: drop them in the asset folder and grab them by name with `Assets::Get<T>("filename")`.

Implementation: [Core/Assets.h](../include/SandCastle/Core/Assets.h), [Core/Assets.cpp](../src/Core/Assets.cpp).

## Folder layout

```
<assetFolder>/                  ← path from EngineSettings::assetFolder, default "assets/"
├── textures/
│   ├── foo.png                 ← image file
│   ├── foo.texture             ← sidecar JSON: import settings + spritesheet layout
│   └── ...
├── animations/
│   └── walk.anim               ← JSON describing keyframes
├── shaders/
│   ├── default.vert
│   ├── default.frag
│   └── default.geom            ← optional
├── audio/
│   └── click.wav               ← .wav / .mp3 / .ogg
├── fonts/
│   └── Silver.ttf              ← TTF/OTF, used by Writer
├── localized/                  ← per-language overrides
│   ├── en/
│   │   ├── textuals/
│   │   │   └── en_<domain>.textual    ← JSON key→string
│   │   └── (textures, animations…)    ← any localized art lives in mirrored folders
│   ├── fr/...
│   └── jp/...
└── (any other folder you want — Assets walks recursively)
```

`Assets::GetFolder()` returns the configured root. `Assets::GetAvailableLangs()` lists every language code discovered under `localized/`.

## Asset types

| Type | Get with | Backed by |
|---|---|---|
| `Texture` | `Assets::Get<Texture>("foo.png")` | image file via stb_image; settings from `.texture` sidecar |
| `Sprite` | `Assets::Get<Sprite>("foo.png_<col>_<row>")` | auto-generated from spritesheet entries in the `.texture` file |
| `Shader` | `Assets::Get<Shader>("name.shader")` | `.vert` + `.frag` (+ optional `.geom`) compiled at boot |
| `Animation` | `Assets::Get<Animation>("walk.anim")` | JSON keyframe list |
| `Textual` | `Assets::Get<Textual>("key")` | localized string blob (the "key" is a JSON key inside one of the `.textual` files) |

Anything else (audio files) is registered with the relevant subsystem (`Audio::MakeSound` etc.) — `Assets` just learns where the file lives.

## `.texture` sidecar (JSON)

A `.texture` file lives next to its image and configures import + spritesheet partitioning. Default settings come from `EngineSettings::textureImport` ([Render/TextureSettings.h](../include/SandCastle/Render/TextureSettings.h)). The asset loader generates one if missing.

Fields recognised:
- `filtering` — `Linear` or `Nearest` (use Nearest for pixel art).
- `wrapping` — `Clamp` or `Repeat`.
- `pixelPerUnit` — texels per world unit (drives sprite world dimensions).
- `useMipmaps`, `keepData`, `lodMin`, `lodMax`.
- A spritesheet block describing rect grid → `Sprite` ids `<file>_<col>_<row>`.

## `.anim` files

JSON with a frequency (1/fps) and a list of keyframes. Each keyframe references a sprite name, a `timeToNext`, and an optional `sendSignal` flag. Loaded as `Animation` (Serializable, see [Render/Animation.h](../include/SandCastle/Render/Animation.h)).

## Shader compilation

Three sibling files define one shader: `name.vert`, `name.frag`, optional `name.geom`. They're compiled in `Assets::CompileShaders()` and addressable via `Assets::Get<Shader>("name.shader")` (yes, the suffix is `.shader` — synthetic).

Quad shaders see a fixed vertex layout (location 0 vec3 pos, 1 vec2 uv, 2 vec4 color, 3 float texIndex) and a 16-slot sampler array. See [RENDERING.md#custom-shaders](RENDERING.md#custom-shaders).

## Hot reload

`Assets::HotReload()` re-walks the folder and refreshes changed assets in place. Texture pointers and Sprite atlas slices are re-uploaded, so existing references stay valid. Use it from a debug tool (e.g. `DevSys::OnImGui`).

## Localization

Three moving parts:

1. **Folder structure.** Anything under `localized/<lang>/...` is loaded into the registry tagged with `<lang>`. Currently the `textuals/` folder is the typical use, but the same path works for swapping textures/animations between languages.
2. **Textual files.** `localized/<lang>/textuals/<lang>_<domain>.textual` is a flat JSON `key → string`. Read code-side via `Assets::Get<Textual>(key)`.
3. **Active language.** `Assets::SetLang("en")` swaps the active set atomically. Listeners on `Assets::Instance()->langSignal` (a `Signal<LangSignal*>`) re-read their text. UI elements created with `Ui::TextLoc(key)` and buttons created with `Ui::ButtonLoc(key)` subscribe automatically.

Inline icons in text: register with `writer->AddIcon(id, sprite, offset)` ([RENDERING.md#text](RENDERING.md#text)) then write `\|id\|` inside any localized string. The Writer expands them to glyph entities.

The `Textual` typedef ([Core/Textual.h](../include/SandCastle/Core/Textual.h)) is just `std::string` — it exists so `Assets::Get<Textual>` resolves to localized blobs instead of raw strings.

## Loading order

`Engine::Init` calls `Assets::Init(folder, defaultLang)` after the renderer is up but before `Audio::PostAssetInit` and `Renderer2D::PostAssetInit`. This sequencing matters because:

- `Renderer2D::PostAssetInit` picks up the default batch shader and default layer shader from the now-loaded shader assets.
- `Audio::PostAssetInit` registers any sound files discovered during the asset walk.

Conclusion: do not reference `Assets::Get<Shader>` from `Renderer2D::Init`-time code; do it in `PostAssetInit` or later.

## Persistent / save folder

Use `Roaming::AddPath(id, relPath)` once at startup, then `Roaming::GetPath(id)` to resolve absolute paths under the platform's roaming/save folder. `Roaming::RoamingFolder()` returns the absolute folder path (e.g. `%APPDATA%/<appName>` on Windows).

Per-platform implementation in [Internal/PersistentDataPath.h](../include/SandCastle/Internal/PersistentDataPath.h).
