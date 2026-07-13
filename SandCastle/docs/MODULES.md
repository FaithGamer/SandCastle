# Modules

Per-module breakdown of every public header. Source is mirrored under `src/` (e.g. `include/SandCastle/Render/Texture.h` ↔ `src/Render/Texture.cpp`). Two stray files implement headers from a different module:

- `src/Render/Bindings.cpp` → `include/SandCastle/Input/Bindings.h`
- `src/Physics/Bitmask.cpp` → `Filter16` from `include/SandCastle/Core/Bitmask.h`

Open the header for the `/// @brief` of every class and method — these tables are just navigation aids.

---

## Core (`include/SandCastle/Core/`)

Foundation utilities used everywhere. Aggregate include: [Core.h](../include/SandCastle/Core.h).

| Header | Key types / functions | Notes |
|---|---|---|
| [Assets.h](../include/SandCastle/Core/Assets.h) | `Assets` (singleton), `Asset<T>`, `OpaqueAsset` | Scans asset folder, owns Textures/Sprites/Shaders/Animations/Textuals. `HotReload()`, `SetLang()`. Specialized `Get<T>` for Sprite/Texture/Shader/Animation/Textual only. |
| [Bitmask.h](../include/SandCastle/Core/Bitmask.h) | `Bitmask<T>`, `Bitmask8`/`16`/`32`/`64`, `Filter16` | `Filter16` = named flag registry, used for physics layers. |
| [Container.h](../include/SandCastle/Core/Container.h) | `Container::Remove`, `RemoveAt`, `PushIfDoesntContains`, `Contains`, `FindIndex`, `OrderedSet<T>` | `Remove*` swap with `back()`, do not preserve order. |
| [Delegate.h](../include/SandCastle/Core/Delegate.h) | `Delegate<Ret, Obj, Args...>`, `FunctionDelegate` | Free fn / member fn wrapper. Optional pre-bound object and/or args. `Call()`, `CallArg(...)`, `CallOn(obj)`, `CallArgOn(obj, ...)`. |
| [Easing.h](../include/SandCastle/Core/Easing.h) | `Easing::SineInOut`, `Quad*`, `Cubic*`, `Circ*`, `Expo*`, `ElasticOut` | All take `t ∈ [0,1]`, return `[0,1]`. |
| [Fancy.h](../include/SandCastle/Core/Fancy.h) | `sys(T)` macro = `Systems::Get<T>()` | Only enabled when client defines `SC_FANCY`. |
| [Files.h](../include/SandCastle/Core/Files.h) | `Files::IfstreamToString` | One-liner reader. |
| [Geometry.h](../include/SandCastle/Core/Geometry.h) | `Geometry::LinesIntersection` | Returns `(0,0)` and warns when lines are parallel. |
| [Hardware.h](../include/SandCastle/Core/Hardware.h) | `Hardware::IsSteamDeck` | Detects via DMI vendor/product strings. |
| [Int128.h](../include/SandCastle/Core/Int128.h) | `Int128 = boost::int128_t` | Plus a `Math::FormatCompact(Int128)` overload. |
| [LangSignal.h](../include/SandCastle/Core/LangSignal.h) | `LangSignal { lang }` | Emitted by `Assets::langSignal` on language change. |
| [Log.h](../include/SandCastle/Core/Log.h) | `Log` (singleton), `LOG_INFO/WARN/ERROR/TRACE`, `ASSERT_LOG_ERROR`, `LogSDLError` | Wraps spdlog. Macros become no-ops if `SandCastle_NO_LOGGING` is defined. |
| [Math.h](../include/SandCastle/Core/Math.h) | `Math::Abs/Min/Max/Sin/Cos/Lerp/Clamp/Clamp01/Sign/Repeat`, `MoveTowards`, `MoveTowardsAngle`, `DeltaAngle`, `NearestMultiple`/`FloorMultiple`/`CeilMultiple`, `RoundPow2`/`FloorPow2`/`CeilPow2`, `FloorToEven`, `Radians`/`Degrees`, `AngleToVec`/`VecToAngle`, `ScaleRangeTo`, `FormatCompact(double, int=2)`, `ToString(double/float, precision)` | Engine works in degrees. `FormatCompact` supports `1.5k`, `2.34M`, `9.99B`, ..., scientific notation past `Q`. |
| [Noise.h](../include/SandCastle/Core/Noise.h) | `Noise` (instance) + `Noise::Noise2D(x, y, freq)` static | Wraps `siv::PerlinNoise`. |
| [Print.h](../include/SandCastle/Core/Print.h) | `Print::mat4` | Debug-pretty-prints a 4×4 matrix to the log. |
| [Profiling.h](../include/SandCastle/Core/Profiling.h) | `Profiling::StartClock/StopClock/LogAllClocks/GetLastCycleMs`, `START_PROFILING`/`STOP_PROFILING` macros | Macros compile out unless `SC_PROFILING` defined. |
| [Random.h](../include/SandCastle/Core/Random.h) | `Random::Seed`, `Range(min,max)` (int/uint64/int64/float/double/Int128), `Pick`, `PickAndRemove` | `Range` bounds inclusive. |
| [Rate.h](../include/SandCastle/Core/Rate.h) | `Rate { rate, minRate, Tick(amount), Update(delta), SetSampleMax/AutoMaxPeriod/RateUpdatePeriod }` | Smoothed events-per-window tracker. Drives `HighrateSound` crossfades. |
| [Roaming.h](../include/SandCastle/Core/Roaming.h) | `Roaming(subPath)` | Absolute path under per-user save folder for this app (`%LOCALAPPDATA%Low/<appName>/...` on Windows). |
| [Serialization.h](../include/SandCastle/Core/Serialization.h) | `Serialized` (json wrapper), `Serializable` interface, `Json` alias for `nlohmann::json` | `LoadFromDisk`, `WriteOnDisk`, typed `Get*`/`TryGet`/`GetArray`, `HadLoadError`/`HadParseError`/`HadGetError`. |
| [Signal.h](../include/SandCastle/Core/Signal.h) | `Signal<T>`, `SignalPriority` | `Listen(method, obj, prio)` / `Listen(fn, prio)` / `Send(payload)`. Duplicate (obj,method) ignored. |
| [Task.h](../include/SandCastle/Core/Task.h) | `OpaqueTask`, `Task<Obj, Args...>` | Type-erased Delegate wrapper for `WorkerThread`. |
| [Textual.h](../include/SandCastle/Core/Textual.h) | `Textual = std::string` | Distinct alias so `Assets::Get<Textual>` resolves localized blobs. |
| [Time.h](../include/SandCastle/Core/Time.h) | `Time`, `Clock`, `Time::Delta()/FixedDelta()/UnscaledDelta()/UnscaledFixedDelta()`, `Time::SetTimeScale()/GetTimeScale()` | `Time` implicitly converts to seconds (float). Built on `std::chrono::microseconds`. |
| [TypeId.h](../include/SandCastle/Core/TypeId.h) | `TypeId::GetId<T>()` | Process-wide unique int per type. **Not stable across builds — never serialize.** |
| [Vec.h](../include/SandCastle/Core/Vec.h) | `Vec2<T>`, `Vec3<T>`, `Vec2f/i/u`, `Vec3f/i/u`, `Vec4f`, `Mat3`, `Mat4`, `Vec` (static helpers) | Implicit conversion to `glm::vec*` and Box2D `b2Vec2`. |
| [Versioning.h](../include/SandCastle/Core/Versioning.h) | `Versioning::Push/Get/GetAll`, `Version = std::string` | Push your release version before `Engine::Init`. Gets baked into the window title. |
| [Worker.h](../include/SandCastle/Core/Worker.h) | `WorkerThread` | Single-thread task queue. `StartThread`, `Queue(method, obj, args...)` / `Queue(fn, args...)`, `Wait()`. |
| [std_macros.h](../include/SandCastle/Core/std_macros.h) | `String`, `sptr`, `uptr`, `wptr`, `makesptr`, `makeuptr`, `MakePair` | Aliases used everywhere. |

---

## Render (`include/SandCastle/Render/`)

2D batched renderer + camera/window/text/particles. Aggregate: [Render.h](../include/SandCastle/Render.h). Threading model: see [RENDERING.md](RENDERING.md).

| Header | Key types | Notes |
|---|---|---|
| [Renderer2D.h](../include/SandCastle/Render/Renderer2D.h) | `Renderer2D` (singleton), `QuadData`, `RenderLayer`, `RenderQueue`, `OffscreenRenderLayer`, `QuadBatch`, `Statistics` | Push quads via `PushQuad`. Per-layer Z sort optional via `SetLayerSortZ`. Up to `MAX_LAYERS=32` visible layers and `MAX_OFF_LAYERS=15` offscreen layers. |
| [Window.h](../include/SandCastle/Render/Window.h) | `Window` (singleton, `RenderTarget`) | SDL3 window + GL context. `SetSize/Fullscreen/Vsync/ClearColor/Cursor`, resize/focus/minimize signals, `PixelMatchPoint` for hi-DPI. |
| [Camera.h](../include/SandCastle/Render/Camera.h) | `Camera`, `Camera::Constraints`, `Camera::main` | Orthographic by default. Pixel-perfect via `Constraints` (`pxStep`, `targetRatio`, `cropW/H`). |
| [Shader.h](../include/SandCastle/Render/Shader.h) | `Shader` | Loaded by `Assets`. Direct `SetUniform` for low-level usage; prefer `Material`. |
| [Material.h](../include/SandCastle/Render/Material.h) | `Material`, `MaterialID`, `MaterialProperty`, `MaterialPropertyArray` | Created via `Renderer2D::CreateMaterial`. Setters: `SetFloat/Int/Vec2f/Vec3f/Vec4f/FloatArray/IntArray`. |
| [RenderOptions.h](../include/SandCastle/Render/RenderOptions.h) | `RenderOptions` | Per-material GL state (depth test). |
| [Texture.h](../include/SandCastle/Render/Texture.h) | `Texture` | Disk/memory/empty/region constructors. PBO streaming (`SetPBOStreaming`, `UpdateRegion`). `GetPixels()` requires `keepData=true`. |
| [TextureSettings.h](../include/SandCastle/Render/TextureSettings.h) | `TextureImportSettings`, `TextureFiltering`, `TextureWrapping` | JSON-serializable per-texture import config. |
| [Sprite.h](../include/SandCastle/Render/Sprite.h) | `Sprite` | Texture + rect + origin. Auto-generated by Assets from spritesheets. |
| [SpriteRender.h](../include/SandCastle/Render/SpriteRender.h) | `SpriteRender` (component), `SpriteRender::defaultLayer` | Pair with `Transform`. Drawn by `SpriteRenderSystem`. |
| [SpriteRenderSystem.h](../include/SandCastle/Render/SpriteRenderSystem.h) | `SpriteRenderSystem` | Auto-pushed by `Engine::Init`. `SetZSort(true)` for clean transparency. `MakeQuadRenderDataFromSpriteRender` static helper. |
| [Animation.h](../include/SandCastle/Render/Animation.h) | `Animation` (Serializable), `Keyframe`, `KeyframeSignal` | Loaded from `.anim` JSON. |
| [Animator.h](../include/SandCastle/Render/Animator.h) | `Animator` (component), `AnimationState` | Multi-state with transitions. `SetAnimation(name)`, `AddAnimation(name, anim, transition="")`, per-frame `Listen` callbacks. |
| [AnimationSystem.h](../include/SandCastle/Render/AnimationSystem.h) | `AnimationSystem` | Auto-pushed by `Engine::Init`. |
| [LineRenderer.h](../include/SandCastle/Render/LineRenderer.h) | `LineRenderer` | Variable-width polyline; `LINE_WIDTH_INDICES = 5` width samples. End-cap vertex count tunable. |
| [LineRendererSystem.h](../include/SandCastle/Render/LineRendererSystem.h) | `LineRendererSystem` | Auto-pushed. |
| [WireRender.h](../include/SandCastle/Render/WireRender.h) | `WireRender` | 1px-width lines. |
| [WireRenderSystem.h](../include/SandCastle/Render/WireRenderSystem.h) | `WireRenderSystem` | Auto-pushed. |
| [Particle.h](../include/SandCastle/Render/Particle.h) | `Particle` (component), `ParticleEmitter` (component), `ParticleEasingFn`, `ParticleTraj` (Straight/CubicIn/CubicOut/CubicInOut) | t goes 0→1 along a `Beziers` curve. `ParticleEmitter` has chainable setters (`UseSprite`, `Trajectory`, `Curve(min,max)` for a gradient or `Curve({-0.5f, 0.5f})` for discrete choices, `Distance`, `Angle`, `Lifetime`, `CountPerBurst`, `BurstRate` (bursts/sec, 0 pauses), `Tint`, `Fade`, `Scale`, `Bursts`, `Play`/`Pause`, `BurstOnce`, `DestroyWhenDone`). First burst fires with no warm-up delay. |
| [ParticleSystem.h](../include/SandCastle/Render/ParticleSystem.h) | `ParticleSystem` (system), `EmitterHandle` | `Make(p1, p2, ...)` returns the spawned `Entity` (one-shot, original API). `MakeEmitter(pos)` creates an entity carrying a `ParticleEmitter` and returns `EmitterHandle` — stores only the `Entity`, mirrors every `ParticleEmitter` setter, returns `EmitterHandle&` so the whole chain stays dot-style and is safe to keep across frames (re-resolves the component per call). `Burst(pos, spec)` fires one burst from a temporary spec. Auto-pushed. |
| [Beziers.h](../include/SandCastle/Render/Beziers.h) | `Beziers` | Cubic. Factories: `Straight`, `CubicIn`, `CubicOut`, `CubicInOut`. `Step(t)`, `LengthFast()`. |
| [Writer.h](../include/SandCastle/Render/Writer.h) | `Writer`, `Font`, `Glyph`, `Character`, `GlyphRange`, `Sentence` | FreeType-backed. `MakeFont`, `Write(utf8, ...)`. Inline icons: `AddIcon(id, sprite)` then write `\|id\|` in the text. |
| [Text.h](../include/SandCastle/Render/Text.h) | `TextAlign`, `FontID`, `Sentence` | `Sentence` = root entity + per-glyph child entities. |
| [Color.h](../include/SandCastle/Render/Color.h) | `Color`, `Color::White/Black/Red/Green/Blue` | 8-bit RGBA, packs to/from uint32 and Vec4f. |
| [Layer.h](../include/SandCastle/Render/Layer.h) | `LayerID = unsigned char` | |
| [Rect.h](../include/SandCastle/Render/Rect.h) | `Rect { left, top, width, height }` | Y+ up. `Bottom() = top - height`. `PointInside`, static `Inside(outer, inner)`. |
| [RenderTarget.h](../include/SandCastle/Render/RenderTarget.h) | `RenderTarget` interface | Implemented by `Window` and `RenderTexture`. |
| [RenderTexture.h](../include/SandCastle/Render/RenderTexture.h) | `RenderTexture : RenderTarget` | FBO+RBO+texture for offscreen rendering. |
| [Buffer.h](../include/SandCastle/Render/Buffer.h) | (umbrella) `VertexBuffer.h` + `IndexBuffer.h` + `UniformBuffer.h` | |
| [VertexBuffer.h](../include/SandCastle/Render/VertexBuffer.h) | `VertexBuffer`, `AttributeLayout`, `AttributeElement`, `ShaderDataType` | Static- or dynamic-draw. |
| [IndexBuffer.h](../include/SandCastle/Render/IndexBuffer.h) | `IndexBuffer` | Element array. |
| [UniformBuffer.h](../include/SandCastle/Render/UniformBuffer.h) | `UniformBuffer` | Bound to a fixed UBO binding point. Used for the shared scene data. |
| [VertexArray.h](../include/SandCastle/Render/VertexArray.h) | `VertexArray` | VAO bundling VBOs + IBO. |
| [QuadRenderData.h](../include/SandCastle/Render/QuadRenderData.h) | `QuadRenderData` | Wire format pushed into `Renderer2D::PushQuad`. ≤64 bytes. |
| [Transform.h](../include/SandCastle/Render/Transform.h) | `Transform` (component) | 3D position, 2D scale, Z rotation. Hierarchy via `SetParent(EntityId)`. |

Deep dive: [RENDERING.md](RENDERING.md).

---

## ECS (`include/SandCastle/ECS/`)

EnTT-backed. Aggregate: [ECS.h](../include/SandCastle/ECS.h).

| Header | Key types | Notes |
|---|---|---|
| [Entity.h](../include/SandCastle/ECS/Entity.h) | `Entity`, `Children`, `Parent`, `entt::registry registry` (static) | `Create()`, `CreateSprite(name)`, `CreateAnimatedSprite(animName, stateName)`, `Add<T>(args...)`, `AddGet<T>`, `Get<T>`, `Remove<T>`, `View<T...>(exclude_t<>)`, `GetFirst<T>`, `Destroy()`, `AddChild`/`RemoveChild`/`Unparent`. |
| [EntityId.h](../include/SandCastle/ECS/EntityId.h) | `EntityId = entt::entity` | |
| [Components.h](../include/SandCastle/ECS/Components.h) | `PointableComponent` macro | Add to a component for stable pointers (`in_place_delete = true`). |
| [System.h](../include/SandCastle/ECS/System.h) | `System` base, `System::Method` bitmask | Override `Start`, `PostStart`, `Update`, `FixedUpdate`, `LateUpdate`, `OnImGui`, `OnEvent(SDL_Event)`, `OnRemove`. Override `GetUsedMethod()` to opt-in only what you use. **`OnImGui` runs on the render thread.** |
| [Systems.h](../include/SandCastle/ECS/Systems.h) | `Systems` (singleton), `SystemIdPriority` | `Push<T>(args...)`, `Get<T>`, `Remove<T>`, `HasSystem<T>`, `SetFixedUpdateTime`. Priority-ordered; priority 0 → assigned in push order. |
| [StateMachine.h](../include/SandCastle/ECS/StateMachine.h) | `StateMachine<T>`, `StateMachineOpaque` | Per-enum FSM. `SetState`, `GetState`, `PushEnter/Exit/Update/LateUpdate/FixedUpdate(state, &C::method, this)`, `PushScope(states, onEnterScope, onExitScope)` (enter fires before Enter callbacks, exit after Exit callbacks). Driven by `States`. |
| [States.h](../include/SandCastle/ECS/States.h) | `States` (System) | Static registry of `StateMachine<T>` keyed by `TypeId`. `Push<T>()`, `Get<T>()`, `Set<T>(value)`, `Fetch<T>()`. Push the system into `Systems` to drive every machine. |
| [GameSys.h](../include/SandCastle/ECS/GameSys.h) | `GameSys<D, A>`, `NoData`, `NoAsset` | System base bundling a per-run data entity (`_d : D*`) and an assets entity (`_a : A*`). Override `LoadAssets`/`LinkCallbacks`/`OnStart`. `PushEnter/Exit/Update/LateUpdate/FixedUpdate(state, method)` forwards to the matching `StateMachine<T>`. `SetScope(state)` / `SetScope({states})` declares the states `_d` lives in — the engine then drives `CreateData`/`DeleteData`; without it the data lifetime is manual. See [PATTERNS.md#data-scope](PATTERNS.md#data-scope). Inherits `Serializable` (no-op defaults). |

---

## Input (`include/SandCastle/Input/`)

Aggregate: [Input.h](../include/SandCastle/Input.h).

| Header | Key types | Notes |
|---|---|---|
| [Input.h](../include/SandCastle/Input/Input.h) | `Input` base, `InputType` (Button/Directional/Textual), `InputSignal`, `Input::InputEventFlag` | Each input owns a `Signal<InputSignal*>`. Subclasses override SDL handlers. |
| [Inputs.h](../include/SandCastle/Input/Inputs.h) | `Inputs` (singleton), `InputMapContainer`, `Inputs::PeripheralFlag` (Mouse/Keyboard/Gamepad) | `CreateInputMap("name")`, `Get(map, input)`, rebinding workflow (`StartRebind`/`EndRebind`/`AddForbiddenBinding`). Gamepad-mode tracking: `IsGamepadMode()` / `SetGamepadMode()` / `AutoToggleGamepadMode()` / `gamepadModeSignal` — hides the OS cursor in gamepad mode and gates the Ui gamepad-nav selector. `CreateDefaultGamepadNav("map")` builds the standard `ui_left/right/up/down/select/cancel` bindings. |
| [InputMap.h](../include/SandCastle/Input/InputMap.h) | `InputMap` (Serializable), `EventType` enum | `CreateButtonInput`/`CreateDirectionalInput`. `SetActive(false)` to disable a whole map. `SetPassThroughImGui(true)` to capture even when ImGui has focus. |
| [Bindings.h](../include/SandCastle/Input/Bindings.h) | `Button`, `DirectionalButton`, `DirectionalStick`, `Direction`, `ButtonBindings`, `DirectionalBindings` | Data model for `SetBindings`. |
| [ButtonInput.h](../include/SandCastle/Input/ButtonInput.h) | `ButtonInput`, `ButtonInputState` | `BindKey/Mouse/GamepadButton/GamepadTrigger`. `SetSignalOnPress/Release` selects edges. |
| [DirectionalInput.h](../include/SandCastle/Input/DirectionalInput.h) | `DirectionalInput`, `DirectionalInputState` | Buttons-as-direction or analog stick. `BindWASD()` shortcut. |
| [TextualInput.h](../include/SandCastle/Input/TextualInput.h) | `TextualInput` | Reserved, not yet wired. |
| [Keyboard.h](../include/SandCastle/Input/Keyboard.h) | `Key::Scancode` enum, `Key::ScancodeName`, `KeycodeFromScancode`, `ScancodeFromName` | Scancodes are physical positions. |
| [Mouse.h](../include/SandCastle/Input/Mouse.h) | `Mouse::Button`, `ButtonName`, `ButtonFromName`, `GetPosition`, `GetWorldPos` | `GetWorldPos` uses `Camera::main`. |
| [Gamepad.h](../include/SandCastle/Input/Gamepad.h) | `Gamepad::Axis`/`Button`/`Trigger`/`Stick`, name lookups | `Stick::Left/Right/None` static instances. |

---

## Physics (`include/SandCastle/Physics/`)

Box2D v3 wrapper (id-based C API: `b2WorldId`/`b2BodyId`/`b2ShapeId`). Aggregate: [PhysicsEngine.h](../include/SandCastle/PhysicsEngine.h).

| Header | Key types | Notes |
|---|---|---|
| [Physics.h](../include/SandCastle/Physics/Physics.h) | `Physics` (singleton) | `RaycastClosest`, `RaycastAll`, `CircleOverlap`, `PointInside`, `BodyOverlap`, `Step(dt)`, `SetGravity`/`GetGravity` (default `{0, -9.81}`), `SetSubStepCount`/`GetSubStepCount` (default 4), `AddLayer(name)`, `GetLayerMask(names...)`, `DrawColliders(bool)`, `GetB2World()` (returns `b2WorldId`). Query callbacks live in `Physics.cpp`. |
| [Body.h](../include/SandCastle/Physics/Body.h) | `Body` (component), `StaticBody`, `KinematicBody`, `DynamicBody`, `RaycastResult`, `OverlapResult` | Multi-collider supported. Layer/mask can be changed any time (live shapes updated). `GetB2Body()` returns a `b2BodyId`. `DynamicBody`: simulated (velocity, forces, impulses, gravity scale, fixed rotation, linear/angular damping); its entity Transform is written back by the PhysicsSystem. |
| [Collider.h](../include/SandCastle/Physics/Collider.h) | `Collider` interface, `Box2D`, `Circle2D`, `Polygon2D`, `ColliderRender` | `Polygon2D::SetPoints` then `AddCollider`. `SetMaterial(density, friction, restitution)` before `AddCollider` (not retroactive). Overlap/point tests (`PointInside`, `CircleOverlap`, `ColliderOverlap`) are implemented on the base class. |
| [PhysicsSystem.h](../include/SandCastle/Physics/PhysicsSystem.h) | `PhysicsSystem` | Auto-pushed by engine. Each fixed timestep: steps the world (`Physics::Step`) and snapshots `DynamicBody` state. Each frame: syncs `KinematicBody` from `Transform`, and writes `DynamicBody` state to `Transform` interpolated between the last two fixed steps (`Systems::GetFixedUpdateAlpha`). |
| [ColliderRenderDebugSystem.h](../include/SandCastle/Physics/ColliderRenderDebugSystem.h) | `ColliderRenderDebugSystem` | Push manually for debug wireframes. `UpdateQueueAuto(false)` for thread-safe deferred construction. Wire drawing currently dormant (WireRenderSystem disabled). |

---

## UI (`include/SandCastle/UI/`)

Builder-style retained UI. Aggregate: [UIheader.h](../include/SandCastle/UIheader.h). Deep dive: [UI.md](UI.md).

| Header | Key types | Notes |
|---|---|---|
| [Ui.h](../include/SandCastle/UI/Ui.h) | `Ui` (singleton), `UiGroupSignal` | Static-method API (`Ui::Begin/End/Text/Button/...`), context stack (`SnapshotContext`/`Context`), interaction groups (`EnableGroup`/`DisableGroup`). |
| [UiElem.h](../include/SandCastle/UI/UiElem.h) | `UiElem` base, `UiElem::ID/State/Type` | Hover/click signals, parent/child, `Disable`/`Enable`, `SetAbsolutePos`, `SetZOffset`. |
| [UiCanvas.h](../include/SandCastle/UI/UiCanvas.h) | `UiCanvas`, `UiCanvas::SizeLimit` | Layout container. Root canvas honors `rootAnchor`; children anchor top-left. |
| [UiTxt.h](../include/SandCastle/UI/UiTxt.h) | `UiTxt` | Text. Supports static, localized (`Ui::TextLoc`), or live data binding through std::format pointers. |
| [UiBtn.h](../include/SandCastle/UI/UiBtn.h) | `UiBtn` | 4-state frame button (idle/hover/pressed/disabled), optional press/release sounds. |
| [UiAnimBtn.h](../include/SandCastle/UI/UiAnimBtn.h) | `UiAnimBtn : UiBtn` | Same but with sprite animations per state. |
| [UiImg.h](../include/SandCastle/UI/UiImg.h) | `UiImg` | Single sprite. |
| [UiCheckbox.h](../include/SandCastle/UI/UiCheckbox.h) | `UiCheckbox` | 3-sprite (unchecked/hovered/checked). Optional `bool*` binding. |
| [UiLoadBar.h](../include/SandCastle/UI/UiLoadBar.h) | `UiLoadBar` | Contour + filling 9-slice. Label modes: None / Percent / ValueGoal. |
| [UiFrame.h](../include/SandCastle/UI/UiFrame.h) | `UiFrame`, `UiFrame::Template`, `BorderSprite`, `BorderSprites` | 9-slice with optional `fixedStep`. `MakeTemplate(template, texturePath, fixedStep)`. |
| [UiContext.h](../include/SandCastle/UI/UiContext.h) | `UiContext`, `ButtonContext`, `AnimButtonContext`, `CheckboxContext`, `CanvasContext`, `TextContext`, `LoadBarContext`, `LoadBarTextMode` | Styling state pushed/popped via context snapshots. |
| [UiEnum.h](../include/SandCastle/UI/UiEnum.h) | `CanvasAnchor`, `LayoutDir`, `LayoutAlign` | |

---

## Audio (`include/SandCastle/Audio/`)

miniaudio wrapper. Aggregate: [Audioheader.h](../include/SandCastle/Audioheader.h).

| Header | Key types | Notes |
|---|---|---|
| [Audio.h](../include/SandCastle/Audio/Audio.h) | `Audio` (singleton) | `AddChannel(name, parent)`, `SetChannelVolume`, `MakeSound(file, channel)`, `MakeHighrateSound(...)`, `MakeHandle(...)`. Spike protection ducks output on runaway volume. |
| [Sound.h](../include/SandCastle/Audio/Sound.h) | `Sound` | High-level. `AddVariant(file)` for random selection. `pitchVariation`, `timeBetweenPlay` rate-limit identical triggers. |
| [HighrateSound.h](../include/SandCastle/Audio/HighrateSound.h) | `HighrateSound : Sound`, `HighrateSound::Range` | Crossfades layers as `Play()` rate climbs. Used for guns / engines / pours. |
| [SoundHandle.h](../include/SandCastle/Audio/SoundHandle.h) | `SoundHandle` | Refcounted `ma_sound*` wrapper. Created by `Audio`, copy-safe. |

---

## Steam (`include/SandCastle/Steam/`)

Optional Steamworks integration. Opt-in at runtime: call `Steam::Init(SteamSettings)` before `Engine::Launch`, or never touch the class and the game has no Steam dependency (the object file and import stubs are dropped at link time, no DLL needed). `vendor/steam_api64.lib` is merged into `SandCastle.lib`, so games using Steam only ship `vendor/steam_api64.dll` next to the executable (plus `steam_appid.txt` for dev runs launched outside Steam — never ship that file).

| Header | Key types | Notes |
|---|---|---|
| [Steam.h](../include/SandCastle/Steam/Steam.h) | `Steam` (static class), `SteamSettings` | `Init` handles `RestartAppIfNecessary` + optional basic DRM (`stopOnInitFail`); returns false when the game must exit. Achievements (`UnlockAchievement`, `IsAchievementUnlocked`, `ShowAchievementProgress`, `ResetAllAchievements`), int stats (`SetStat`/`GetStat`/`StoreStats` + periodic auto-upload), `GetPlayerId`/`GetPlayerName`, `GetLanguage` (mapped to engine lang codes) / `GetSteamLanguage` (raw), DLC/app queries (`IsDlcInstalled`, `IsSubscribed`, `IsAppInstalled`, `GetAppInstallDir`), `IsOnSteamDeck`, `OpenUrlOverlay`. Every call is a safe no-op when Steam is disabled. Internal `SteamSystem` (defined in [src/Steam/Steam.cpp](../src/Steam/Steam.cpp)) pumps `SteamAPI_RunCallbacks` on FixedUpdate and calls `SteamAPI_Shutdown` on engine shutdown. |

---

## Tools (`include/SandCastle/Tools/`)

| Header | Key types | Notes |
|---|---|---|
| [SpriteExport.h](../include/SandCastle/Tools/SpriteExport.h) | `SpriteExportConfig`, `ShowSpriteExport()` | Inline ImGui widget that drives the Aseprite CLI to export `.png` + `.texture` + per-tag `.anim`. |

---

## Internal (`include/SandCastle/Internal/`)

Engine-private. Don't include from game code.

| Header | Key types | Notes |
|---|---|---|
| [Singleton.h](../include/SandCastle/Internal/Singleton.h) | `Singleton<T>` CRTP base | `Instance()` lazily constructs. Engine destroys via `Kill()` during shutdown. |
| `ImGuiLoader.h` | `ImGuiLoader` | Bootstraps Dear ImGui with SDL3+OpenGL3 backends when `SC_IMGUI` is defined. |
| `PersistentDataPath.h` | Per-platform persistent path resolver | |
| `PlatformDetection.h` | Platform macros | |

---

## Top-level

| Header | Notes |
|---|---|
| [SandCastle.h](../include/SandCastle/SandCastle.h) | Umbrella — game code's only include. |
| [Engine.h](../include/SandCastle/Engine.h) | `Engine::Init/Launch/Stop/IsInit`. |
| [EngineSettings.h](../include/SandCastle/EngineSettings.h) | `EngineSettings` (Serializable). JSON keys in [BUILD.md#engine-settings](BUILD.md#engine-settings). |
| [Entt.h](../include/SandCastle/Entt.h) | Pulls in `entt/entt.hpp` and routes `ENTT_ASSERT` through the engine logger. |
