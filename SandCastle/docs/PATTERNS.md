# Patterns

Conventions and runtime patterns common across the engine. Read once before extending the engine; cross-link from this file when in doubt.

## Init order

`Engine::Init` boots subsystems in this exact order ([src/Engine.cpp](../src/Engine.cpp)):

```
Log
└─ Window (SDL3 window + GL contexts)
   └─ Renderer2D::Init        ← spawns the render thread, builds default batch buffers
      └─ Audio::Init
         └─ Assets::Init      ← scans asset folder, compiles shaders, loads textures/anims/textuals
            ├─ Audio::PostAssetInit
            ├─ Renderer2D::PostAssetInit  ← picks up default shaders/materials
            ├─ Renderer2D::AddLayer("DebugLayer")
            ├─ Physics
            ├─ Inputs
            ├─ Ui
            ├─ Systems::Init
            ├─ ImGuiLoader (when SC_IMGUI)
            └─ default ECS systems pushed:
               SpriteRenderSystem, LineRendererSystem, WireRenderSystem,
               AnimationSystem, PhysicsSystem, ParticleSystem, States
```

Then `Engine::Launch()` runs `Systems::Update()` until `Engine::Stop()`. Shutdown order:

```
Renderer2D::Wait()      ← drain the render thread
Inputs::Kill
Systems::Kill
Ui::Kill
Renderer2D::Kill
Window::Kill
ImGuiLoader::ExitImGui  (when SC_IMGUI)
```

Implications:
- Push your client systems via `Systems::Push<T>()` **before** `Engine::Init` to land before the engine defaults; push **after** to land after them. Most game logic is fine after.
- Don't reference `Assets::Get<T>` from a static initialiser — Assets is empty until `Init`.
- Don't use `Renderer2D::Get*` (default materials, etc.) from `Renderer2D::Init`-time code — those exist only after `PostAssetInit`.

## Singletons

Pattern: CRTP `Singleton<T>` ([Internal/Singleton.h](../include/SandCastle/Internal/Singleton.h)). `T::Instance()` lazily constructs. Engine destroys via `T::Kill()` during shutdown — never call `Kill()` from game code.

Engine singletons: `Log`, `Assets`, `Window`, `Renderer2D`, `Audio`, `Physics`, `Inputs`, `Ui`, `Systems`. Plus the static `Engine` class itself.

`sys(T)` macro from [Core/Fancy.h](../include/SandCastle/Core/Fancy.h) expands to `Systems::Get<T>()` — but only when client code defines `SC_FANCY`. The engine itself never uses it.

## ECS

EnTT registry under `Entity::registry` (one global). Any component type is registered the first time it's used. Components are plain structs — engine ones live in their owning module's headers (`SpriteRender`, `Transform`, `Animator`, `Body`, `LineRenderer`, etc.).

| Operation | API |
|---|---|
| Create entity | `Entity::Create()` (uninitialised), `Entity::CreateSprite(spriteName)`, `Entity::CreateAnimatedSprite(animName, stateName)` |
| Add component | `e.Add<T>(args...)` — no-op if component already present |
| Add and get | `e.AddGet<T>(args...)` — returns pointer, gets the existing one if any |
| Get | `e.Get<T>()` (nullptr if absent), `e.GetNoCheck<T>()` (assumes present) |
| Remove | `e.Remove<T>()` |
| Destroy | `e.Destroy()` — also destroys every child |
| Query | `Entity::View<T1, T2, ...>(entt::exclude_t<E1, E2, ...>{})` |
| First-only | `Entity::GetFirst<T>()` (handy for "data" singleton components) |
| Count | `Entity::Count()`, `Entity::CountOf<T>()` |
| Hierarchy | `parent.AddChild(child)`, `parent.RemoveChild(id)`, `child.Unparent()` |

`PointableComponent` macro (see [ECS/Components.h](../include/SandCastle/ECS/Components.h)) tags a component with `in_place_delete = true`, guaranteeing stable pointers — slightly slower iteration, but required if you intend to cache `T*` somewhere.

## Systems

Inherit `System` ([ECS/System.h](../include/SandCastle/ECS/System.h)). Override only what you need:

```cpp
class MySystem : public System {
public:
    void Start() override;          // before first update
    void Update() override;         // every frame
    void FixedUpdate() override;    // fixed step (Systems::SetFixedUpdateTime)
    void LateUpdate() override;     // after every Update/FixedUpdate
    void OnImGui() override;        // RENDER THREAD — only ImGui calls
    bool OnEvent(SDL_Event& e) override; // return true to consume
    int  GetUsedMethod() override { return Updt | LateUpdt; } // optional opt-in
};
```

Push with `Systems::Push<MySystem>(args...)` — returns the created instance. `Systems::Get<T>()` retrieves it later. `Systems::Remove<T>()` queues for removal at the next safe point.

**Priority**: higher priority runs first within each phase. `SetPriority(n)` from the constructor; default 0 means "auto-assigned in negative based on push order" (so push order is preserved).

`OnImGui` is the only hook that runs on the render thread — every other hook runs on the main thread.

## Signals

`Signal<T>` ([Core/Signal.h](../include/SandCastle/Core/Signal.h)) is the engine's pub/sub primitive.

```cpp
Signal<MyPayload*> mySignal;

// listener
mySignal.Listen(&MyClass::OnIt, this, SignalPriority::medium);

// broadcaster
MyPayload p{...};
mySignal.Send(&p);
```

- `Listen(memberFn, obj, priority)` and `Listen(freeFn, priority)` are both supported.
- Duplicate `(obj, method)` pairs are silently ignored on listen.
- `StopListen(obj)` removes every listener bound to that object — common pattern in destructors.
- `T` is whatever payload type you choose; engine uses pointers (`InputSignal*`, `UiElem*`, `LangSignal*`, `Vec2u`, `bool`, etc.).
- Priority enum: `high < medium < low`. Within a priority level, dispatch order is the insertion order.

`Delegate<Ret, Obj, Args...>` ([Core/Delegate.h](../include/SandCastle/Core/Delegate.h)) is the lower-level wrapper used by `Signal` and `WorkerThread`. Use it when you need to call a method/function later with optional pre-bound args.

## State machines

Generic `StateMachine<EnumT>` is a client-side pattern — see [meat/src/StateMachine.h](../../../meat/src/StateMachine.h). The engine doesn't ship a state machine class; pick whichever fits your game.

The engine's own subsystems (Cash::State, Slave::State, etc. in `meat`) follow the convention of nesting the enum inside the owning component / system (`Cash::State::Cashout`, `Slave::State::Walking`).

## Threading

Two concurrent threads in a typical run:

| Thread | Responsibilities |
|---|---|
| Main | Game logic. `System::Start/Update/FixedUpdate/LateUpdate/OnEvent`, ECS, Audio API, Input dispatch, Ui creation. |
| Render | OpenGL context. `Renderer2D::*Thread()` private functions, `System::OnImGui()`. |

`WorkerThread` ([Core/Worker.h](../include/SandCastle/Core/Worker.h)) is a generic single-thread queue you can spawn for background tasks. Used internally by `RenderQueue`.

Rules:
- Main thread queues data for the render thread via `Renderer2D::PushQuad`, `Renderer2D::CreateSubTexture`, `Renderer2D::AddLayer`, etc. Each one is internally posted as a Worker task — safe to call from game logic.
- Don't issue raw GL from the main thread. Don't touch ECS from `OnImGui`.
- `Renderer2D::Wait()` drains the render thread. Engine calls it once at shutdown; you rarely need to.

## Serialization

`Serializable` ([Core/Serialization.h](../include/SandCastle/Core/Serialization.h)) is the interface — `void Deserialize(Serialized&)` + `Serialized Serialize()`.

`Serialized` wraps `nlohmann::json` plus error tracking (`HadLoadError`, `HadParseError`, `HadGetError`). Typical pattern:

```cpp
Serialized cfg("save.json");
if (cfg.HadLoadError()) { /* fallback */ }
auto value = cfg.GetFloat("hp");
if (cfg.HadGetError()) { /* missing field */ }
auto sub = cfg.GetObj("stats");
auto list = cfg.GetObjArray("inventory");
```

`Serializable::DeserializationError()` lets implementations report semantic failures back to callers.

Engine types implementing `Serializable`: `EngineSettings`, `TextureImportSettings`, `Animation`, `Button`, `ButtonBindings`, `InputMap`.

## Hot-paths and pitfalls

- **Don't capture `Sprite*` then reload assets** without going through `Assets::HotReload` — pointers stay valid because Assets keeps the sprite map stable.
- **Don't capture `Texture*` returned by `Renderer2D::CreateSubTexture` until the next frame.** The actual creation is deferred onto the render thread; the pointer is filled in once the render thread services the request. Wait one `Renderer2D::Process()` cycle (or call `Renderer2D::Wait()`).
- **`TypeId::GetId<T>()` is not stable across builds** — never serialize it.
- **Add components in deterministic order** when you cache pointers across systems. EnTT relocates non-pointable components on growth.
- **Set `Body` layer/mask BEFORE adding colliders**. Changing them after has no effect today.
- **`Camera::main` is a raw pointer.** Engine sets it; if you replace it, the renderer follows the pointer at draw time.

## Versioning + saves

`Versioning::Push(version)` before `Engine::Init` records your build version. The engine bakes it into the window title (`appName - <version>`). Use `Versioning::GetAll()` for save-file migration logic.

Save folder: `Roaming(subPath)` returns an absolute path under the platform's per-user save root for this app (e.g. `%LOCALAPPDATA%Low/<appName>/<subPath>` on Windows). The app name comes from `EngineSettings::appName`, with spaces stripped.

## Logging

```cpp
LOG_INFO("Loaded {0} entities in {1}ms", count, ms);
LOG_WARN(...); LOG_ERROR(...); LOG_TRACE(...);
ASSERT_LOG_ERROR(condition, "message {0}", arg);
```

All macros use `fmt`-style. Compiled out when `SandCastle_NO_LOGGING` is defined ([Core/Log.h](../include/SandCastle/Core/Log.h)).

`LOG_ERROR` and `LOG_WARN` flush immediately — `LOG_INFO`/`LOG_TRACE` don't. Errors bring the logger up to an `assert` via `ASSERT_LOG_ERROR`.

## Profiling

Wrap a region:

```cpp
START_PROFILING("PhysicsStep");
physics.Step(dt);
STOP_PROFILING("PhysicsStep");
```

Macros compile out unless `SC_PROFILING` is defined. Read back with `Profiling::GetLastCycleMs(name)` or dump everything with `Profiling::LogAllClocks()`.
