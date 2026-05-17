# Rendering

The renderer is **threaded**: a dedicated render thread owns the OpenGL context. Game code never issues GL calls — it pushes data via `Renderer2D::PushQuad` / draws lines+wires via system glue, and the render thread consumes the queue, batches, and submits.

## Threading

| Thread | Owns | Allowed to |
|---|---|---|
| Main / game | EnTT registry, game state | Push quads, create textures via `Renderer2D::CreateSubTexture`, change camera, change layers. **Never** issues raw GL. |
| Render | OpenGL context | Run all GL. Driven by `Renderer2D::Process()` called once per frame from `Systems`. |
| Worker (`WorkerThread`) | One queue at a time | Generic background tasks (e.g. queued in `RenderQueue::thread`). |

Synchronisation:
- `Renderer2D::Wait()` — block calling thread until the render thread is idle. Engine calls this during shutdown.
- Functions named `*Thread` (e.g. `InitThread`, `RenderThread`, `OnWindowResizeThread`) are private and run on the render thread.
- `System::OnImGui()` is called **on the render thread** — every other System hook runs on the main thread. Don't touch ECS state from `OnImGui` without external sync; only do ImGui calls.

## Boot sequence (rendering bits)

`Engine::Init` (in [src/Engine.cpp](../src/Engine.cpp)) does:

1. `Window::Init` — create SDL window + GL contexts.
2. `Renderer2D::Init` — boot render thread, create batch buffers, default shaders, white texture, scene UBO.
3. `Audio::Init`.
4. `Assets::Init` — scan asset folder, compile shaders, load textures/sprites/animations/textuals.
5. `Audio::PostAssetInit`.
6. `Renderer2D::PostAssetInit` — pick up the default batch and layer materials from the now-loaded shaders.
7. `Renderer2D::AddLayer("DebugLayer")` — engine-owned debug layer.
8. `Physics::Instance()`, `Inputs::Instance()`, `Ui::Instance()`.
9. `Systems::Init`.
10. `ImGuiLoader::LoadImGui` (when `SC_IMGUI`).
11. Push default ECS systems: `SpriteRenderSystem`, `LineRendererSystem`, `WireRenderSystem`, `AnimationSystem`, `PhysicsSystem`, `ParticleSystem`.

Client `Systems::Push<T>()` calls **before** `Engine::Init` are scheduled before these defaults; calls after are scheduled after.

## Layers

Defined in [Render/Renderer2D.h](../include/SandCastle/Render/Renderer2D.h). Every quad lives on a `LayerID` (`unsigned char`).

| Method | Purpose |
|---|---|
| `Renderer2D::AddLayer(name)` | Visible layer, full screen, default layer material. |
| `Renderer2D::AddLayer(name, height)` | Visible layer with fixed pixel height (preserves aspect ratio). |
| `Renderer2D::AddLayer(name, material)` | Visible layer with custom layer material (composited at end). |
| `Renderer2D::AddOffscreenLayer(name, samplerIndex, [material])` | Doesn't display. Becomes available as `sampler2D` at `samplerIndex` (1..15) in another layer's shader. |
| `Renderer2D::SetLayerSortZ(id, true)` | Z-sort quads on this layer. Off by default. |
| `Renderer2D::SetLayerScreenSpace(id, vec4Verts)` | Constrain layer to a normalized screen quad (split screen, picture-in-picture). |
| `Renderer2D::SetLayerMaterial(id, material)` | Swap the layer's compositing material at runtime. |
| `Renderer2D::SetLayerHeight(id, height)` | Resize a fixed-height layer. |

**Layer order is push order.** Layers cannot be removed once added.

Constants in [Renderer2D.h](../include/SandCastle/Render/Renderer2D.h):
- `MAX_TEXTURE_INDEX = 16` — GL 3.3 multi-bind limit.
- `MAX_LAYERS = 32`
- `MAX_OFF_LAYERS = 15`

## Quad batching

Each batch ties a `Material` + a `LayerID` and accumulates up to `MAX_TEXTURE_INDEX` distinct textures. When a quad arrives that breaks the batch (different material, layer, or 17th texture), the current batch is flushed and a new one starts.

Per-frame flow:

```
Game loop pushes QuadRenderData -> RenderQueue (double-buffered, mutex-light)
LateUpdate finishes -> Renderer2D::Process()
  swap queues
  for each layer (in order):
    if zsort enabled: sort by .pos.z descending
    for each quad: build/extend batches, flush on break
  composite layers via layer materials
  present
```

`QuadRenderData` is hand-tuned to ≤64 bytes ([QuadRenderData.h](../include/SandCastle/Render/QuadRenderData.h)).

## Materials

Created via `Renderer2D::CreateMaterial(shader, isLayer)`. `isLayer=true` flags a material as a per-layer compositor (not a quad shader). Materials own their uniform values via `SetFloat/Int/Vec*/...` so the same shader can serve many materials cheaply.

Quad/sprite shaders read from a shared scene UBO (`SceneBufferData`):
- `mat4 camProjView`
- `float camZoom`, `camAspectRatio`
- `vec2 winSize`, `targetSize`
- `int cropMask`
- `float reduction`

UBO binding is owned by `Renderer2D` (`m_sceneUniformBinding`). Custom shaders that want this data should call `Shader::BindUniformBlock("Scene", binding)`.

## Render targets

`RenderTarget` is the interface, implemented by `Window` and `RenderTexture`. The active target is set with `Renderer2D::SetRenderTarget(target)`. Switching to a `RenderTexture` lets you render the scene into a texture you can sample later (or hand to a sprite for in-world UI).

## Textures and sub-textures

`Texture` constructors:
- From file path
- From in-memory encoded buffer (`unsigned char*`, size)
- From a region of another texture — **prefer `Renderer2D::CreateSubTexture(source, rect)`** for thread safety.
- Empty `(width, height)` for fonts/atlases that you fill later via `UpdateRegion`.

PBO streaming:
- `texture.SetPBOStreaming(true)` — uses two PBOs round-robined for asynchronous uploads.
- `texture.UpdateRegion(x, y, w, h, rgba8Bytes)` — partial upload.

Get pixels back on the CPU only if `TextureImportSettings::keepData = true`.

## Sprites and animations

`Sprite` = texture + atlas rect + normalized origin (`orgX/orgY`, in `numeric::float16_t`). Origin (0,0) is centered.

`Animation` (struct) is a `Serializable` loaded from `.anim` JSON files. Each `Keyframe` has a `Sprite*`, a `timeToNext`, and an optional `sendSignal` flag. `Animator` plays states by name with optional transitions on completion.

## Lines and wires

| Helper | Use when |
|---|---|
| `LineRenderer` | Variable-width polylines, configurable end caps (round/flat). |
| `WireRender` | 1-pixel-width quick lines (e.g. collider debug). |

Both are component types; the matching engine systems (`LineRendererSystem`/`WireRenderSystem`) submit them. **Z-ordering is not guaranteed** between lines/wires and quads on the same layer — put debug lines on a dedicated layer if precise ordering matters.

## Particles

`ParticleSystem` (engine, pushed by `Engine::Init`) spawns and updates `Particle` entities. Two ways to use it, both available at the same time:

1. **Imperative** — `Make(p1, p2, ...)` / `MakeWithSprite(sprite, p1, p2, ...)` build a Bezier trajectory from `p1` to `p2` and tween a single sprite along it. Good for one-off shots.
2. **Component-driven** — `MakeEmitter(pos)` returns an `EmitterHandle` you configure with a fluent chain. The system bursts particles every frame based on the emitter's rules. Good for sustained effects, randomized look, persistent sources.

A particle dies automatically when its parametric `t` reaches `1.0`.

### Setup

- `SetDefaultSprite(sprite)` once during system init — without it, `Make()` and emitters without an explicit `UseSprite` produce nothing.
- `SetLimit(n)` caps live particles (default `1000`). **Spawns above the limit are silently dropped**, including emitter bursts — bump the limit for bursty effects, or check `GetCount()` if you need to gate the call yourself.
- `Activate(false)` destroys every live particle and resets every emitter's timer until re-enabled.

### Imperative API — key parameters on `Make()` / `MakeWithSprite()`

- `speed` — t-rate multiplier. `1.0` ≈ 1 second to traverse `p1→p2`. Higher = faster = shorter lifetime.
- `traj` — `ParticleTraj` shape:

  | Value | Feel |
  |---|---|
  | `Straight` | constant velocity |
  | `CubicOut` | snappy decel — fast start, slow finish |
  | `CubicIn` | slow start, fast finish |
  | `CubicInOut` | smooth ease in and out |

- `curveIntensity` — Bezier control-point offset (default `0.3`). `0` collapses to a straight line.
- `fade` — signed alpha ramp as a fraction of lifetime: positive fades out over the final portion, negative fades in over the initial portion, `0` = no fade.
- `easing` — optional `double(double)` curve applied to `t` before sampling the trajectory (helpers in [Core/Easing.h](../include/SandCastle/Core/Easing.h)).
- `color`, `scale` — tint and size override on the sprite.

### Emitter API — `MakeEmitter(pos)` / `EmitterHandle`

`MakeEmitter(pos)` creates an entity with `Transform + ParticleEmitter` and returns an `EmitterHandle`. The handle stores **only the entity** — every chained setter re-resolves the component, so storing the handle as a class member is safe across frames and EnTT relocations (no `PointableComponent` needed).

The whole chain is dot-style; `operator->` and `operator*` exist for read access to the component, but every fluent setter (`UseSprite`, `Trajectory`, `Curve`, `Distance`, `Angle`, `Lifetime`, `Scale`, `Fade`, `Tint`, `EasingFn`, `SpawnArea`/`NoSpawnArea`, `BurstRate`, `CountPerBurst`, `Bursts`, `Play`/`Pause`, `BurstOnce`, `DestroyWhenDone`) is mirrored on `EmitterHandle` and returns `EmitterHandle&`.

| Group | Setters | Notes |
|---|---|---|
| Visual | `UseSprite(Sprite*)`, `OnLayer(LayerID)`, `WithMaterial(MaterialID)` | Falls back to `ParticleSystem::GetDefaultSprite()` / `SpriteRender::defaultLayer` / `0` if unset. |
| Trajectory | `Trajectory(ParticleTraj)`, `Curve(min,max)` / `Curve(v)` / `Curve({-0.5f, 0.5f})` / `Curve(std::vector<float>)`, `CurveBothSides(bool)` | Range form: random in `[min,max]`. List form: discrete pick — supersedes range and `curveBothSides`. |
| Travel | `Distance(min,max)` / `Distance(v)`, `Angle(min,max)` / `Angle(v)`, `Lifetime(min,max)` / `Lifetime(v)`, `Scale(min,max)` / `Scale(v)` | Angle is in degrees, `0°` points up, clockwise. |
| Look | `Fade(f)`, `Tint(c)` / `Tint(a, b)` / `Tint({ {color, weight}, ... })`, `EasingFn(fn)` | `Tint(a, b)` lerps per-particle through `[a, b]` component-wise. List form: weighted random pick — weights are relative, plain `Color` entries default to weight 1, supersedes the gradient. |
| Spawn area | `SpawnArea(w, h)` / `SpawnArea(Rect)` / `NoSpawnArea()` | Each particle picks its own uniform random point in the box (one burst scatters across the whole area). `(w,h)` centers the box on the emitter; `Rect` is an arbitrary offset box relative to the emitter position. Default = point emission. |
| Scheduling | `BurstRate(min,max)` / `BurstRate(v)` (bursts per second), `CountPerBurst(min,max)` / `CountPerBurst(v)`, `Bursts(n)`, `Play()` / `Pause()`, `BurstOnce()`, `DestroyWhenDone(bool=true)` | First burst fires on the next `Update()` with no warm-up. Rate `<= 0` idles the emitter without flipping `playing`. `Bursts(-1)` = infinite (default). `BurstOnce()` fires immediately regardless of timer. |

`Burst(pos, spec)` fires one burst from a temporary `ParticleEmitter` value without creating an entity — handy when the source isn't long-lived.

### Patterns

**Confetti emitter** (sustained, dot-chain):

```cpp
sys(ParticleSystem)->MakeEmitter({x, y, 0.f})
    .UseSprite(Assets::Get<Sprite>("square.png_0_0"))
    .Tint(Palette::Green1, Palette::Green2)
    .Distance(40.f, 80.f)
    .Angle(0.f, 360.f)
    .CountPerBurst(8, 12)
    .Lifetime(0.6f, 1.0f)
    .BurstRate(10.f)
    .Curve({ -2.5f, 2.5f })
    .Fade(0.4f)
    .Scale(1.5f, 2.5f);
```

**One-shot burst** (no persistent entity left behind):

```cpp
sys(ParticleSystem)->MakeEmitter(pos)
    .UseSprite(spark)
    .CountPerBurst(20)
    .Distance(30.f, 60.f)
    .Lifetime(0.4f, 0.7f)
    .Trajectory(ParticleTraj::CubicOut)
    .Fade(0.5f)
    .Bursts(1)
    .DestroyWhenDone();
```

**Radial burst (imperative form)** — equivalent loop if you don't want an emitter entity:

```cpp
auto* ps = Systems::Get<ParticleSystem>();
for (int i = 0; i < count; ++i)
{
    float angle = Random::Range(0.f, 360.f);
    float dist  = Random::Range(20.f, 60.f);
    Vec2f end   = origin + Math::AngleToVec(angle) * dist;
    ps->MakeWithSprite(spark, origin, end,
        Random::Range(2.f, 4.f), Color::White,
        ParticleTraj::CubicOut, 0.3f, 0.5f);
}
```

**Trail** — one particle per frame behind a moving entity, drifting backward:

```cpp
Vec3f tip  = entity.gtr()->GetPosition();
Vec3f back = tip - Vec3f(velocity.x, velocity.y, 0.f) * 0.04f;
Systems::Get<ParticleSystem>()->Make(tip, back,
    1.5f, Color::White, ParticleTraj::Straight, 0.f, 0.3f);
```

For bursty effects, raise `SetLimit` once at startup rather than per-spawn — silently dropped particles are the most common surprise.

## Text

`Writer` (accessible through `Ui::GetWriter()`) bakes glyphs lazily into atlas pages built from a configurable max size (default 4096). Inline icons: `writer->AddIcon(id, sprite, offset)` then write `\|id\|` in any string passed to `Write()`.

`Sentence` returned by `Write` is a root entity + per-glyph child entities — destroying the root destroys the whole sentence.

## Camera (briefly — see [Camera.h](../include/SandCastle/Render/Camera.h))

- `Camera::main` is the active camera the renderer samples from.
- Default is orthographic; toggle with `SetOrthographic(false)`.
- `Camera::Constraints` for pixel-perfect rendering — call `SetDefault()` for a 16:9 360p baseline.
- `WorldToScreen` / `ScreenToWorld` for hit testing.

## Stats

`Renderer2D::GetStats()` → `{ drawCalls, quadCount }` from the previous frame. Wire it to a debug overlay.

## Custom shaders

Drop a `.vert` / `.frag` (and optional `.geom`) into the asset folder. Assets compiles them at startup. Reference by name through `Assets::Get<Shader>("name.shader")`.

For a quad-shader, your vertex layout is fixed (see `QuadData` in [Renderer2D.h](../include/SandCastle/Render/Renderer2D.h)):

```
location 0 vec3 vertexPos
location 1 vec2 uv
location 2 vec4 color
location 3 float texIndex
```

Sampler array is bound automatically (`uniform sampler2D u_textures[16]`) — the renderer assigns texture units; sample with `texture(u_textures[int(v_texIndex)], v_uv)`.

For a layer compositing material, the input is the layer's offscreen texture; bind it with `Shader::BindUniformBlock("Scene", ...)` to access the shared scene UBO.
