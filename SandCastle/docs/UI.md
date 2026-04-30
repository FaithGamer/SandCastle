# UI

Builder-style retained UI. Build a tree with `Ui::Begin` / `Ui::End` and the widget helpers; styling flows through a stack of contexts (font, colors, frame templates, padding…). Widgets are real ECS entities living in the renderer's UI layer — interaction (hover/click) is hooked through the `Hoverable`-style signals on `UiElem`.

Implementation: [UI/Ui.h](../include/SandCastle/UI/Ui.h), [UI/Ui.cpp](../src/UI/Ui.cpp).

## Build a screen

```cpp
Ui::Context("default");                                 // pop a saved styling snapshot
Ui::Begin({0, 0}, true);                                // open a canvas (auto-sized)
  Ui::Text("Hello, world");
  Ui::Button("Buy")->ListenClickReleased(&MySys::OnBuy, this);
  Ui::LoadBar({200, 16}, 100.0, 25.0);
Ui::End();                                              // close current canvas
```

`Begin` / `End` are stack-based: each `Begin` pushes a canvas, every widget you create until the matching `End` becomes a child of that canvas.

## Widgets

Returned by static `Ui::*` calls. All inherit `UiElem`.

| Helper | Returns | Notes |
|---|---|---|
| `Ui::Begin(size = {0,0}, frame = true)` | `UiCanvas*` | Layout container. Pass `{0,0}` to auto-size to children. |
| `Ui::End()` | — | Closes the most recent canvas. |
| `Ui::Text(utf8, width = -1)` | `UiTxt*` | Static text. |
| `Ui::Text(utf8, width, ptr0, ptr1, …)` | `UiTxt*` | Format-bound. Each pointer is read every frame; `std::format` placeholders `{0}`, `{1}` substitute dereferenced values. Uses `Math::FormatCompact` when `SetCompact(true)`. |
| `Ui::TextLoc(key, width)` | `UiTxt*` | Localized: looks up `Assets::Get<Textual>(key)` and re-renders on language change. |
| `Ui::TextLoc(key, width, ptrs…)` | `UiTxt*` | Localized + format-bound. |
| `Ui::Image(name)` / `Ui::Image(sprite)` | `UiImg*` | Single sprite. |
| `Ui::Button(utf8)` / `Ui::ButtonLoc(key)` | `UiBtn*` | 4-state frame button. Listen with `ListenClickReleased(&Sys::Method, this)`. |
| `Ui::AnimButton(utf8)` / `Ui::AnimButtonLoc(key)` | `UiAnimBtn*` | Same shape, animations per state. |
| `Ui::Checkbox(boolPtr = nullptr)` | `UiCheckbox*` | Optional `bool*` binding kept in sync each frame. `checkSignal` fires on flip. |
| `Ui::LoadBar(size, goal = 1.0, current = 0.0)` | `UiLoadBar*` | Contour + fill 9-slice. Update via `SetProgress(current, goal)`. |
| `Ui::Destroy(elem)` | — | Destroys a widget and its children (each widget type has an overload). |
| `Ui::UpdateText` / `Ui::UpdateBtn` / `Ui::UpdateLoadBar` | — | Update content without rebuilding. |

`UiElem` exposes per-listener templates: `ListenHover`, `ListenUnhover`, `ListenClickPressed`, `ListenClickReleased`. Each registers the element as hoverable automatically.

## Init: fonts, frames, contexts

A typical `InitUI()` does these once, before any `Ui::Begin`:

```cpp
// Fonts (once per fancy name; languages list selects which lang variants to bake)
Ui::MakeFont("Silver.ttf", "p", 19, 1.f, .7f, Assets::GetAvailableLangs());

// Inline icons that text strings can reference as |id|
auto writer = Ui::GetWriter();
writer->AddIcon("cash", Assets::Get<Sprite>("currency_icons.png_0_0"), Vec2f(-1, 0));

// 9-slice frame templates from a 3x3 spritesheet
Ui::MakeFrameTemplate("frame.png");
Ui::MakeFrameTemplate("frame_dash.png", true);   // fixedStep = repeat in integer multiples

// Snapshot-able context for each visual style
Ui::SetButtonFont("p");
Ui::SetButtonFrame("btn_blue_idle.png");
Ui::SetButtonFrameHover("btn_blue_hover.png");
Ui::SetButtonFramePressed("btn_blue_pressed.png");
Ui::SetButtonFrameDisabled("btn_disabled.png");
Ui::SnapshotBtnContext("blue");

// Top-level snapshot
Ui::SetCanvasFrame("frame_black.png");
Ui::SetCanvasPadding(4);
Ui::SetSpacing(4);
Ui::SetTextFont("p");
Ui::SetRootAnchor(CanvasAnchor::MiddleCenter);
Ui::SnapshotContext("default");
```

Then anywhere in game code:

```cpp
Ui::Context("default");          // re-applies the saved styling
Ui::BtnContext("blue");          // and the saved button styling
```

See [meat/src/InitUI.cpp](../../../meat/src/InitUI.cpp) for a complete real-world setup.

## Context fields (`UiContext` in [UiContext.h](../include/SandCastle/UI/UiContext.h))

The full context bag covers: canvas (frame, padding, spacing, layout dir/align), text (font, color, align), button (font, colors, padding, 4 frame templates, sounds), animButton (4 animations), checkbox (texture), load bar (contour+fill frames, fill margin/color, label mode/font/color), plus margin / rootMargin / layer / material / rootAnchor / z / interactionGroup.

Setters under `Ui::Set*` mutate the live context. `Ui::SnapshotContext(name)` deep-copies it; `Ui::Context(name)` restores. Same pair for buttons: `SnapshotBtnContext` / `BtnContext`.

## Frames (9-slice)

A `UiFrame::Template` wraps a 3×3 spritesheet (4 corners + 4 sides + center repeating texture). Make once via `Ui::MakeFrameTemplate("file.png", fixedStep)`; pass the same name into context setters.

`fixedStep = true` clamps growth to integer multiples of the border textures — useful when the borders have a repeating pattern that mustn't shear.

## Coordinate spaces

All UI math is in **UI units**, where the screen height is `360 / Ui::PPU()` (default `Ui::PPU(1.f / 360.f)`). Helpers in `Ui`:

| Function | Goes from → to |
|---|---|
| `Ui::WorldToUi(worldPos)` | world → UI |
| `Ui::UiToWorld(uiPos)` | UI → world (returns `Vec3f`) |
| `Ui::ScreenToUi(screenPx)` | window pixels → UI |
| `Ui::MousePos()` | current mouse position in UI units |

Root canvases anchor by `rootAnchor` (3×3 grid). All non-root canvases anchor top-left.

## Interaction groups

`Ui::SetInteractionGroup(n)` tags every newly created element with group `n`. Then `EnableGroup(n)` / `DisableGroup(n)` / `EnableOnlyGroup(n)` / `EnableAllGroups()` toggle which groups receive hover/click events. `Ui::GetGroupSignal()` broadcasts group enable/disable changes.

## Ordering

`Ui::SetOrder(z)` sets the `z` value applied to subsequent elements. Lower z renders in front. Use this for tooltips and overlays.

## Hoverables

`Ui` registers anything with a click/hover listener on its hoverable list and runs hit testing internally. To make a non-UI sprite hoverable, the engine doesn't ship a generic `Hoverable` — game code typically defines its own component (`meat`'s `Hoverable.h` is a good reference) and wires up an SDL event listener. `Ui::RegisterHoverable(elem)` is for UI elements only.

## Localization integration

`UiTxt::langSignal` and `UiBtn::langSignal` re-render their content when `Assets::langSignal` fires (i.e. on `Assets::SetLang(lang)`). `TextLoc` / `ButtonLoc` plug them in automatically.

Inline icon syntax `\|cash\|` inside any localized string expands to a glyph entity using the icon registered via `writer->AddIcon`.

## Material override

`Ui::SetMaterial(material)` overrides the material used for newly created widgets — typically pointing to a UI shader that handles outlines / drop shadows. Reset with `Ui::ResetMaterial(defaultMaterial)`.

`Ui::DefaultMaterial(material)` sets the engine-wide default once during init.

## Common pitfalls

- **Don't call `Ui::*` from `OnImGui()`** — that's the render thread; widgets are real ECS entities and must be built on the main thread.
- **Don't `Begin` without `End`** — the canvas stack is shared, missing pairs will parent later widgets to the wrong canvas.
- **Frame templates must be made before referencing them in a snapshot.** `Ui::MakeFrameTemplate` looks up the sprites at call time; doing it after `SnapshotContext` is fine for new widgets but the snapshot still records the pointer.
- **Listening to clicks on a `UiTxt`/`UiImg`** requires explicitly calling `ListenClickPressed`/`Released`; it implicitly registers the element as hoverable.
