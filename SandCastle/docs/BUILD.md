# Build

`SandCastle.vcxproj` produces a static library that the consuming game project links against.

## Configurations (x64 only)

| Configuration | Defines | Output |
|---|---|---|
| `x64-Debug` | `SC_PROFILING`, `SC_IMGUI`, `_DEBUG`, `_CONSOLE` | `bin/x64-Debug/SandCastle.lib` |
| `x64-Release` | `SC_PROFILING`, `SC_IMGUI`, `NDEBUG`, `_CONSOLE` | `bin/x64-Release/SandCastle.lib` |
| `x64-Distrib` | `SANDCASTLE_DISTRIB`, `NDEBUG` | `bin/x64-Distrib/SandCastle.lib` |

The vcxproj also has stray `Application` (`x64`) configs left over from an earlier setup — ignore them; the `StaticLibrary` configs above are the live ones.

## Preprocessor defines

| Define | Where set | Effect |
|---|---|---|
| `SC_IMGUI` | Debug, Release | Enables Dear ImGui integration. `Engine::Init` calls `ImGuiLoader::LoadImGui`; `System::OnImGui` is dispatched. |
| `SC_PROFILING` | Debug, Release | `START_PROFILING(name)` / `STOP_PROFILING(name)` macros expand. Otherwise compile to no-op. |
| `SC_FANCY` | client-defined | Enables `sys(T)` macro = `Systems::Get<T>()` in [Core/Fancy.h](../include/SandCastle/Core/Fancy.h). |
| `SANDCASTLE_DISTRIB` | Distrib | Marker for shipping builds. |
| `SandCastle_NO_LOGGING` | client-defined | All `LOG_*` and `ASSERT_LOG_ERROR` macros become no-ops. |

The PCH is `src/pch.h` (included as `pch.h`). New `.cpp` files must include it as the first include.

## Vendored dependencies

All shipped under `include/` (headers) + `vendor/` (prebuilt `.lib`). No package manager.

| Library | Use | Header root |
|---|---|---|
| SDL3 | Window, input, events, audio device | `include/SDL3/` |
| glad | OpenGL 3.3 loader | `include/glad/` |
| glm | Math (vec / mat / quat) | `include/glm/` |
| EnTT | ECS registry | `include/entt/` |
| Box2D | 2D physics | `include/box2d/` |
| miniaudio | Audio engine | `include/miniaudio/` |
| FreeType | Font rasterizer (used by `Writer`) | `include/freetype/`, `ft2build.h` |
| Dear ImGui | Debug UI (`SC_IMGUI` only) | `include/imgui/` (with SDL3 + OpenGL3 backends) |
| nlohmann/json | JSON | `include/json/` |
| spdlog | Logger | `include/spdlog/` |
| stb_image | PNG/JPG/etc. decode (`stb_impl.cpp`) | `include/stb/` |
| PerlinNoise | Procedural noise | `include/PerlinNoise/` |
| earcut | Polygon triangulation (used by `Polygon2D`) | `include/earcut/` |
| boost int128 | 128-bit integer (`Int128`) | `include/boost/` |
| float16_t | 16-bit float for sprite origin packing | `include/float16_t.hpp` |
| Steamworks SDK | Optional Steam integration (`Steam` class) | `include/steam/` |

Updating a dependency: drop new headers under `include/`, rebuild the dep's `.lib` into `vendor/`, and update `done.txt`.

Steamworks is opt-in per game: `vendor/steam_api64.lib` is merged into `SandCastle.lib` by the Librarian step, so games never link it themselves. Games that call `Steam::*` only need to ship `vendor/steam_api64.dll` next to the executable (plus a `steam_appid.txt` for dev runs launched outside Steam). Games that never reference the class have no Steam dependency (the linker drops the object and the import stubs).

## Engine settings

`EngineSettings` ([include/SandCastle/EngineSettings.h](../include/SandCastle/EngineSettings.h)) is a `Serializable`. `Engine::Init` accepts either an in-memory struct or a JSON file path.

JSON keys (see [src/EngineSettings.cpp](../src/EngineSettings.cpp)):

| Key | Type | Default | Meaning |
|---|---|---|---|
| `app_name` | string | `"SandCastle Application"` | Window title prefix and roaming-folder name. |
| `resolution` | int[2] | `[1280, 720]` | Startup window size in pixels. |
| `fullscreen` | bool | `false` | Start fullscreen. |
| `fixed_time_step` | float | `0.01` | Seconds between `FixedUpdate` calls. |
| `texture_import` | object | `TextureImportSettings()` | Default texture import settings (filtering, wrapping, PPU, mipmaps, keepData, lod range). |
| `default_lang` | string | `""` | Active language code (matches a folder under `assets/localized/`). |
| `asset_folder` | string | `"assets/"` | Asset root folder. |

## Build artefact layout

```
bin/
├── x64-Debug/
│   ├── SandCastle.lib
│   └── int/                ← intermediate object files
├── x64-Release/...
└── x64-Distrib/...
```

Game projects link the appropriate `.lib` and add `include/` (the engine's `include/`) plus the relevant `vendor/*.lib` files for the linked dependencies (SDL3, Box2D, miniaudio static parts, etc. — most ship as headers but a few have prebuilt libs).

## Adding a new engine source file

1. Create `include/SandCastle/<Module>/Foo.h`. Include needed engine headers; add `/// @brief` to every public class/method.
2. Create `src/<Module>/Foo.cpp`. First include is `#include "pch.h"`.
3. Add both to `SandCastle.vcxproj` and `SandCastle.vcxproj.filters` (any IDE handles this).
4. If the new header should appear in an aggregate, add the include line in `include/SandCastle/<Module>.h`.
5. Rebuild.

Two existing exceptions to "source mirrors header path": `src/Render/Bindings.cpp` implements `Input/Bindings.h`, and `src/Physics/Bitmask.cpp` implements `Filter16` from `Core/Bitmask.h`. Don't reproduce these — keep new files mirrored.
