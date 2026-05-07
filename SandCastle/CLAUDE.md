# SandCastle — AI Index

C++ 2D game engine. Static library (`x64-Debug`, `x64-Release`, `x64-Distrib`). OpenGL 3.3 + SDL3 + EnTT + Box2D + miniaudio + FreeType + Dear ImGui (debug only).

**Read this file before searching.** Every public class / method has a `/// @brief` in its header — open the header, do not grep. This document maps every concept and file so you can navigate without trial-and-error.

## Repo layout

```
SandCastle/
├── include/                  ← public headers (this is the API surface)
│   ├── SandCastle/           ← engine headers, organised by module
│   │   ├── SandCastle.h      ← umbrella include (game code uses this)
│   │   ├── Core.h, Render.h, ECS.h, Input.h,
│   │   │ PhysicsEngine.h, UIheader.h, Audioheader.h
│   │   ├── Engine.h, EngineSettings.h, Entt.h
│   │   ├── Core/   Render/  ECS/  Input/
│   │   ├── Physics/  UI/  Audio/  Tools/  Internal/
│   ├── KHR/  PerlinNoise/  SDL3/  boost/  box2d/  earcut/  entt/
│   ├── freetype/  glad/  glm/  imgui/  json/  miniaudio/  spdlog/  stb/
│   └── float16_t.hpp, ft2build.h
├── src/                      ← implementation, mirrors include layout
├── vendor/                   ← compiled .lib for vendored dependencies
├── bin/                      ← build output (per-config subfolders)
├── docs/                     ← sub-docs linked from this file
├── SandCastle.vcxproj        ← MSVC project (StaticLibrary)
└── done.txt                  ← chronological dev log (skim, don't trust)
```

Source is mirrored: `include/SandCastle/Render/Texture.h` ↔ `src/Render/Texture.cpp`. Two stray exceptions:
- `src/Render/Bindings.cpp` actually implements `include/SandCastle/Input/Bindings.h`.
- `src/Physics/Bitmask.cpp` implements `Filter16` from `include/SandCastle/Core/Bitmask.h`.

## Sub-docs

- [docs/MODULES.md](docs/MODULES.md) — header → class → source map for every module
- [docs/RENDERING.md](docs/RENDERING.md) — render thread, layers, materials, batching, render targets, PBO streaming
- [docs/ASSETS.md](docs/ASSETS.md) — Assets singleton, asset folder layout, `.texture`/`.anim`/`.textual` files, hot reload, localization
- [docs/UI.md](docs/UI.md) — `Ui::*` builder, contexts, frames, fonts, hover/click signals, coordinate spaces
- [docs/PATTERNS.md](docs/PATTERNS.md) — singletons, ECS, Signal/Delegate, Worker threads, init/shutdown order
- [docs/BUILD.md](docs/BUILD.md) — vendored deps, configurations, preprocessor defines, build artefacts

Real-world client: `C:/dev/meat` (idle clicker). Use it for usage examples — its [meat/CLAUDE.md](../../meat/CLAUDE.md) lists how every engine API is consumed in practice.

## Git rules

- Never create new branches.
- Never commit unless explicitly asked.
- No worktrees.

## Build rules

- Never try to build the project (no MSBuild, no compile invocations). The user builds in the IDE.

---

## "Where do I find...?"

### Engine lifecycle

| Need | File |
|---|---|
| `Engine::Init`, default systems pushed by engine, shutdown order | [src/Engine.cpp](src/Engine.cpp) |
| `EngineSettings` JSON keys (`app_name`, `resolution`, `fullscreen`, `fixed_time_step`, `texture_import`, `default_lang`, `asset_folder`) | [src/EngineSettings.cpp](src/EngineSettings.cpp) |
| Main loop `Systems::Update()` | [src/ECS/Systems.cpp](src/ECS/Systems.cpp) |
| `pch.h` (engine PCH, distinct from client PCH) | [src/pch.h](src/pch.h) |
| Boot sequence diagram | [docs/PATTERNS.md](docs/PATTERNS.md#init-order) |

### Singletons (all live behind `Singleton<T>::Instance()`; `sys(T)` is **NOT** part of the engine — it's defined by client code via `SC_FANCY` in [Core/Fancy.h](include/SandCastle/Core/Fancy.h))

| Singleton | Header | Source |
|---|---|---|
| `Engine` (static lifecycle) | [Engine.h](include/SandCastle/Engine.h) | [Engine.cpp](src/Engine.cpp) |
| `Log` | [Core/Log.h](include/SandCastle/Core/Log.h) | [Core/Log.cpp](src/Core/Log.cpp) |
| `Assets` | [Core/Assets.h](include/SandCastle/Core/Assets.h) | [Core/Assets.cpp](src/Core/Assets.cpp) |
| `Window` | [Render/Window.h](include/SandCastle/Render/Window.h) | [Render/Window.cpp](src/Render/Window.cpp) |
| `Renderer2D` | [Render/Renderer2D.h](include/SandCastle/Render/Renderer2D.h) | [Render/Renderer2D.cpp](src/Render/Renderer2D.cpp) |
| `Audio` | [Audio/Audio.h](include/SandCastle/Audio/Audio.h) | [Audio/Audio.cpp](src/Audio/Audio.cpp) |
| `Physics` | [Physics/Physics.h](include/SandCastle/Physics/Physics.h) | [Physics/Physics.cpp](src/Physics/Physics.cpp) |
| `Inputs` | [Input/Inputs.h](include/SandCastle/Input/Inputs.h) | [Input/Inputs.cpp](src/Input/Inputs.cpp) |
| `Ui` | [UI/Ui.h](include/SandCastle/UI/Ui.h) | [UI/Ui.cpp](src/UI/Ui.cpp) |
| `Systems` | [ECS/Systems.h](include/SandCastle/ECS/Systems.h) | [ECS/Systems.cpp](src/ECS/Systems.cpp) |

### Modules at a glance (deep dive in [docs/MODULES.md](docs/MODULES.md))

| Module | Aggregate header | What's inside |
|---|---|---|
| Core | [Core.h](include/SandCastle/Core.h) | Assets, Container, Delegate, Easing, Files, Log, Math, Noise, Random, Serialization, Signal, Task/Worker, Time, TypeId, Vec, Bitmask, Profiling, Versioning, Roaming, Rate, Int128, Geometry, Hardware, Fancy, std_macros |
| Render | [Render.h](include/SandCastle/Render.h) | Renderer2D, Camera, Shader, Material, RenderOptions, Texture, Sprite, Animation/Animator, SpriteRender, LineRenderer, WireRender, Particle, ParticleSystem, Writer (text), Window, RenderTarget/RenderTexture, VertexBuffer/IndexBuffer/UniformBuffer/VertexArray, Rect, Color, Layer, Beziers |
| ECS | [ECS.h](include/SandCastle/ECS.h) | `Entity`, `EntityId`, `System`, `Systems`, `PointableComponent` macro, `StateMachine<T>`, `States`, `GameSys<D, A>` |
| Input | [Input.h](include/SandCastle/Input.h) | `Inputs`, `InputMap`, `Input`/`ButtonInput`/`DirectionalInput`/`TextualInput`, `Bindings`, `Key`/`Mouse`/`Gamepad` namespaces |
| Physics | [PhysicsEngine.h](include/SandCastle/PhysicsEngine.h) | `Physics`, `Body` (+ Static/Kinematic), `Collider` (`Box2D`/`Circle2D`/`Polygon2D`), AABB queries, `PhysicsSystem`, `ColliderRenderDebugSystem` |
| UI | [UIheader.h](include/SandCastle/UIheader.h) | `Ui` builder + `UiCanvas`/`UiTxt`/`UiBtn`/`UiAnimBtn`/`UiImg`/`UiCheckbox`/`UiLoadBar`/`UiFrame` widgets, `UiContext` |
| Audio | [Audioheader.h](include/SandCastle/Audioheader.h) | `Audio`, `Sound`, `HighrateSound`, `SoundHandle` |
| Tools | [Tools/SpriteExport.h](include/SandCastle/Tools/SpriteExport.h) | `ShowSpriteExport()` ImGui widget (Aseprite → spritesheet) |
| Internal | `Internal/` | `Singleton<T>` CRTP, `ImGuiLoader`, `PersistentDataPath`, `PlatformDetection` |

### Cross-cutting concepts

| Concept | Where it lives |
|---|---|
| Engine init/shutdown sequence | [docs/PATTERNS.md#init-order](docs/PATTERNS.md#init-order), [src/Engine.cpp](src/Engine.cpp) |
| Render thread (Renderer2D runs work on its own thread) | [docs/RENDERING.md#threading](docs/RENDERING.md#threading) |
| `WorkerThread` for background tasks | [Core/Worker.h](include/SandCastle/Core/Worker.h) — used by `RenderQueue` |
| Signal / Listener pattern | [Core/Signal.h](include/SandCastle/Core/Signal.h), see [docs/PATTERNS.md#signals](docs/PATTERNS.md#signals) |
| Delegate (free + member function wrapper, optional pre-bound args) | [Core/Delegate.h](include/SandCastle/Core/Delegate.h) |
| ECS via EnTT | [ECS/Entity.h](include/SandCastle/ECS/Entity.h) — `Entity::Create`, `Add<T>`, `View<T...>`, `GetFirst<T>`, `CreateSprite`, `CreateAnimatedSprite`. Macro `PointableComponent` for stable pointers. |
| System scheduling, priority, hooks | [ECS/Systems.h](include/SandCastle/ECS/Systems.h), [ECS/System.h](include/SandCastle/ECS/System.h) — `Updt`/`FixedUpdt`/`Event`/`ImGui`/`LateUpdt` bitmask in `GetUsedMethod()` |
| Time + Clock + delta accessors | [Core/Time.h](include/SandCastle/Core/Time.h) — `Time::Delta`, `FixedDelta`, `UnscaledDelta` |
| Localization + lang signal | [Core/Assets.h](include/SandCastle/Core/Assets.h) `langSignal`, [Core/LangSignal.h](include/SandCastle/Core/LangSignal.h), [Core/Textual.h](include/SandCastle/Core/Textual.h). See [docs/ASSETS.md#localization](docs/ASSETS.md#localization) |
| Number formatting (`Math::FormatCompact`, `ToString`) | [Core/Math.h](include/SandCastle/Core/Math.h) |
| 128-bit integer for big counters | [Core/Int128.h](include/SandCastle/Core/Int128.h) (boost) |
| Smoothed event-rate tracker | [Core/Rate.h](include/SandCastle/Core/Rate.h) (drives `HighrateSound`) |
| Easing curves | [Core/Easing.h](include/SandCastle/Core/Easing.h) — `SineInOut`, `QuadIn/Out/InOut`, `CubicIn/Out/InOut`, `CircIn/Out/InOut`, `ExpoIn/Out/InOut`, `ElasticOut` |
| RNG (seeded, ranged) | [Core/Random.h](include/SandCastle/Core/Random.h) — `Range(min, max)`, `Pick`, `PickAndRemove` |
| Perlin noise | [Core/Noise.h](include/SandCastle/Core/Noise.h) |
| JSON serialization wrapper | [Core/Serialization.h](include/SandCastle/Core/Serialization.h) — `Serialized`, `Serializable` |
| Type-id hash for templated dispatch | [Core/TypeId.h](include/SandCastle/Core/TypeId.h) |
| Hot-reloadable assets (textures, sprites, shaders, animations, textuals) | `Assets::HotReload()` |
| Steam Deck detection | [Core/Hardware.h](include/SandCastle/Core/Hardware.h) `Hardware::IsSteamDeck()` |
| Versioning stack (push per release for save migration) | [Core/Versioning.h](include/SandCastle/Core/Versioning.h) |
| Roaming/save folder paths | [Core/Roaming.h](include/SandCastle/Core/Roaming.h) |
| Profiling clocks | [Core/Profiling.h](include/SandCastle/Core/Profiling.h), `START_PROFILING`/`STOP_PROFILING` macros (compiled out unless `SC_PROFILING` defined) |
| Logging macros (`LOG_INFO`/`WARN`/`ERROR`/`TRACE`, `ASSERT_LOG_ERROR`) | [Core/Log.h](include/SandCastle/Core/Log.h) |

### Components added by `Entity::CreateSprite` / `CreateAnimatedSprite`

`CreateSprite` adds: `Transform`, `SpriteRender`. `CreateAnimatedSprite` also adds `Animator`. See [docs/PATTERNS.md#ecs](docs/PATTERNS.md#ecs).

### Default systems pushed by `Engine::Init`

After client-side `Init()` returns, the engine pushes (in this order): `SpriteRenderSystem`, `LineRendererSystem`, `WireRenderSystem`, `AnimationSystem`, `PhysicsSystem`, `ParticleSystem`. Client systems pushed *before* `Engine::Init` end up scheduled before these.

### Render layers (concept)

`Layer = unsigned char`. Created by name with `Renderer2D::AddLayer(name, [material])`. Two more flavors: fixed-height layers preserve aspect ratio; offscreen layers (`AddOffscreenLayer`) don't display but feed sampler slots in another layer's shader (e.g. mask, normal map). Engine adds a `DebugLayer` automatically. Full table in [docs/RENDERING.md#layers](docs/RENDERING.md#layers).

### Asset types recognised by `Assets`

| File extension | Loaded as | Get with |
|---|---|---|
| `.png`/`.jpg`/etc. + `.texture` JSON | `Texture` + auto-`Sprite`s from spritesheet | `Assets::Get<Texture>("name.png")`, `Assets::Get<Sprite>("name.png_<col>_<row>")` |
| `.anim` (JSON) | `Animation` | `Assets::Get<Animation>("foo.anim")` |
| `.vert` / `.frag` / `.geom` | `Shader` (compiled at boot) | `Assets::Get<Shader>("name.shader")` |
| `.wav`/`.mp3`/`.ogg` | Audio file path registered with `Audio` | `Audio::MakeSound("file.wav", "Channel")` |
| `.textual` (JSON, in `localized/<lang>/textual/`) | `Textual` (per-language) | `Assets::Get<Textual>(key)` |

Details in [docs/ASSETS.md](docs/ASSETS.md).

### Vendored dependencies (under `include/`)

SDL3, glad (OpenGL 3.3 loader), glm, EnTT, Box2D, miniaudio, FreeType, Dear ImGui (with SDL3+OpenGL3 backends), nlohmann/json, spdlog, stb (image), PerlinNoise, earcut, boost int128, float16_t. All ship as headers + prebuilt `.lib` in `vendor/`. See [docs/BUILD.md#vendored](docs/BUILD.md#vendored).

### Preprocessor defines

| Define | Effect | Configs that set it |
|---|---|---|
| `SC_IMGUI` | Enables Dear ImGui integration (`ImGuiLoader`, `System::OnImGui`) | Debug, Release |
| `SC_PROFILING` | Enables `START_PROFILING`/`STOP_PROFILING` macros | Debug, Release |
| `SC_FANCY` | Enables `sys(T)` macro = `Systems::Get<T>()` in [Core/Fancy.h](include/SandCastle/Core/Fancy.h) | client-defined |
| `SANDCASTLE_DISTRIB` | Distrib build flag | Distrib only |
| `SandCastle_NO_LOGGING` | Compiles all `LOG_*`/`ASSERT_LOG_ERROR` macros to no-ops | client-defined |

### Build configurations

`SandCastle.vcxproj` produces a static library. Three configs: `x64-Debug`, `x64-Release`, `x64-Distrib`. Output in `bin/x64-<Config>/`. Details in [docs/BUILD.md](docs/BUILD.md).

---

## Conventions

- Public API lives in `include/SandCastle/<Module>/*.h` only. Anything under `include/SandCastle/Internal/` is engine-private.
- Every public class/method has `/// @brief`. **Read briefs instead of guessing.**
- `String = std::string`, `sptr = std::shared_ptr`, `uptr = std::unique_ptr` — defined in [Core/std_macros.h](include/SandCastle/Core/std_macros.h).
- Engine API uses **degrees** for angles. Convert with `Math::Radians`/`Math::Degrees` when crossing into glm/raw math.
- Y+ is up (sprites, UI, world). `Rect::Bottom()` returns the smaller Y.
- Singletons constructed lazily on first `Instance()`. The Engine destroys them in a fixed order during `Launch()` shutdown — don't manually `Kill()` them.
- The render thread owns OpenGL context. Don't issue GL calls from game logic. Use `Renderer2D::PushQuad`, `Renderer2D::CreateSubTexture`, etc.
- Avoid backwards-compat shims. The engine is single-consumer (game) — change the API and update callers.
- 4-space indent. No emojis in code or docs unless explicitly asked.
