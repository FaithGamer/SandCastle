# SandCastle UI — Expert Guide

**Mandatory read before writing any UI code in a SandCastle game.** Every statement below was verified against the engine source (`include/SandCastle/UI/`, `src/UI/`, `src/Render/Writer.cpp`, `src/ECS/Systems.cpp`) and against the three shipped clients (`madman` = canonical style, `meat`, `TheReclaim`). Where the engine has a bug or a trap, it is stated as such — do not "fix" your code to match wishful behavior.

Golden rules, expanded in the sections below:

1. UI space is **screen-centered, Y+ up**: `(0,0)` is the middle of the screen, top edge is `y=+180`, bottom `y=-180`. Horizontal extent is `±180*aspect` (`±320` at 16:9).
2. A canvas **anchor does NOT attach the canvas to a screen edge**. It selects *which point of the canvas* sits at the canvas's `position`. A `BotRight`-anchored canvas at position `(0,0)` has its bottom-right corner at the **center of the screen**, not at the screen corner. To pin it to the screen corner you must also `SetPosition({320,-180})`.
3. Layout is **deferred one frame** (resolved in `Ui::Update`, which runs after all `Updt` systems). `GetSize()`/`GetPosition()` return stale/zero values until then. `LateUpdt` the same frame sees fresh values.
4. `Ui::Destroy` is **deferred** too, and it **fires `unhover` on reaped elements one frame later**.
5. Lower `z` renders **in front**. Every child of a canvas gets `z = parent.z - 5` automatically.
6. Left-click over **any** root canvas (even an invisible, frame-less one) is swallowed before `Inputs` and game systems see it.
7. Never put a `UiLoadBar` and a sibling in the same canvas (known use-after-free). Give every load bar its own single-child canvas.
8. Never call `Ui::*` from `OnImGui()` (render thread). Everything else (Updt/FixedUpdt/Event callbacks) is fine — widgets are main-thread ECS entities.

---

## 1. Coordinate system and units

- **1 UI unit = 1 texture pixel** (at the default texture import `pixelPerUnit = 1`). Sprite widgets take their size directly from the sprite's pixel dimensions.
- The UI layer is projected straight to NDC by the game's `ui.vert` shader: `ndc = pos * uReduction * uPpu`, then `x /= aspect`. `uPpu = 2/360`, so the visible vertical extent is **360 units** (`y ∈ [-180, +180]`), horizontal is **360 × window aspect** (`x ∈ [-320, +320]` at 16:9), both divided by the camera *reduction*.
- With the standard pixel-perfect constraints (`Camera::Constraints::SetDefault()`: `pxStep=360`, `targetRatio=16/9`, crop both), reduction = `targetHeight / windowHeight`, and the crop mask in `ui.frag` letterboxes anything outside the 16:9 target. Net effect: **the games treat the UI screen as a fixed 640×360 box, center origin** — the hardcoded `{320,180}`, `{-320,174}`, `{-318,178}` positions all over meat/madman/TheReclaim are exactly this. Without those constraints the horizontal extent varies with window aspect; only ±180 vertical is guaranteed.
- The UI does **not** move with the world camera. `Camera::main` pan/zoom has no effect on UI positions (it only enters through `WorldToUi`/`UiToWorld` conversions).
- `Ui::PPU(h)` changes the visible vertical extent from the default 360 (e.g. `Ui::PPU(720)` → an element of height 720 fills the screen). It updates both the shader uniform and the internal scale used by `ScreenToUi`/`MousePos`/fonts. No shipped game calls it — they all design for the 360-unit height.

### Required game-side assets

The `Ui` singleton constructor (runs during `Engine::Init`) does `Assets::Get<Shader>("ui.shader")` and creates the `"ui"` render layer (z-sorted, excluded from Scene-stage post-processing). Every game **must ship** `assets/shaders/ui.vert` + `ui.frag` (copy them from an existing game — meat's are canonical). Fonts go in `assets/fonts/`.

---

## 2. The anchor model (read twice)

Root canvases have a `CanvasAnchor` (from `Ui::SetRootAnchor`, part of the context; default `MiddleCenter`). **Nested canvases are always `TopLeft` — the context anchor is ignored for them.** All non-canvas widgets (text, image, button…) are always top-left anchored.

The anchor answers one question: *when I say `SetPosition(p)` (or leave the default `p = (0,0)`), which point of the canvas rectangle lands on `p`?*

| Anchor | Point of the canvas placed at `position` | `margin` applied |
|---|---|---|
| `TopLeft` | top-left corner | `+x`, `-y` (pushes right/down) |
| `TopCenter` | middle of top edge | `-y` only |
| `TopRight` | top-right corner | `-x`, `-y` (pushes left/down) |
| `MiddleLeft` | middle of left edge | `+x` only |
| `MiddleCenter` | center | none |
| `MiddleRight` | middle of right edge | `-x` only |
| `BotLeft` | bottom-left corner | `+x`, `+y` (pushes right/up) |
| `BotCenter` | middle of bottom edge | `+y` only |
| `BotRight` | bottom-right corner | `-x`, `+y` (pushes left/up) |

(Source: `UiCanvas::AnchorOffset()`, [src/UI/UiCanvas.cpp](../src/UI/UiCanvas.cpp).)

Consequences:

- `Ui::SetRootAnchor(CanvasAnchor::BotRight)` + `Begin` + `End` **does not put the canvas in the bottom-right of the screen.** It puts the canvas's bottom-right corner at `(0,0)` = screen center. The standard HUD-corner idiom is anchor + explicit corner position:

```cpp
Ui::SetRootAnchor(CanvasAnchor::TopRight);
auto canvas = Ui::Begin({0,0}, true);
// ... children ...
Ui::End();
canvas->SetPosition({320.f, 180.f});     // now it hugs the top-right screen corner
```

- The anchor is what makes a corner-pinned canvas **grow away from the screen edge**: a `TopRight` canvas at `{320,180}` keeps its top-right corner fixed while auto-size growth extends left/down. Pick the anchor for *which corner must stay put*, then position that corner.
- `margin` on a root canvas (`Ui::SetRootMargin`) is an inward inset from the anchor point, per the table above. It does nothing on the centered axes. With `MiddleCenter` it is ignored entirely.
- `SetPosition` on an anchored root is the *anchor point*, not the top-left. `GetPosition()` on a canvas, however, returns the resolved **top-left** (position + anchor offset + parent chain) — the asymmetry is intentional; treat `GetPosition()` as "laid-out top-left, for measuring" and `SetPosition()` as "place the anchor point".
- Real-world anchor map (madman): `MiddleCenter` modals/menus, `TopLeft` HUD panels at `{-318,178}`, `TopRight` settings gear at `{320,180}`, `TopCenter` XP bar, `BotCenter` bottom prompts, `MiddleLeft/Right` floating description panels. meat drives per-building panels with an anchor + offset stored in data (`uiAnchor`, `uiPos`).

---

## 3. Z-ordering

The `"ui"` layer has z-sorting enabled. **Lower z = in front.**

- `Ui::SetOrder(z)` sets the context z. It is applied to **root canvases created afterwards** (and is captured by `SnapshotContext`). It does *not* affect orphan (canvas-less) widgets — those stay at `z = 0`; use `SetZOffset` on them instead.
- Every element added to a canvas gets `z = parent.z - 5` (`UiCanvas::AddElem`). Each nesting level steps 5 further to the front. Rely on this: madman comments call it out ("each canvas child renders 5 z in front of its parent").
- Widgets consume the −0..−4 band internally relative to their own z: button frames sit at relative 0/−1/−2/−3 (idle/pressed/hover/disabled) and the label at −4; load bar contour 0, filling −1, label −2; checkbox sprites −3/−6/−9.
- `elem->SetZOffset(offset)` nudges one element without touching the layout (scroll thumbs, overlays, badges). Games use hand-picked values like −10/−20/−25.
- Typical overlay ladder from the games (context `SetOrder` values): HUD ~0…−30, tooltips/descriptions −50…−85, modals −70…−100. Reserve a front band for tooltips from day one.

---

## 4. Frame timing and lifecycle

Per-frame order (from `Systems::Update` / `src/ECS/Systems.cpp`):

```
SDL events  →  Ui::OnEvent (click swallow)  →  Inputs  →  game event systems
Updt systems (your code builds/mutates UI here)
Ui::Update:  HoverableUpdate → ValuesUpdate → DestroyUpdate → LayoutUpdate → SelectorUpdate
LateUpdt systems  (fresh layout visible here)
render
```

- **Layout is deferred.** `Begin`/`End`, `AddElem`, `SetSize`, `SetSpacing`, text growth… only mark canvases dirty (`MustUpdate`); positions/sizes resolve in `LayoutUpdate`. Until then `GetSize()` is `(0,0)` for auto-sized canvases and `GetPosition()` is unresolved. Three strategies, all used in madman:
    1. Read sizes in a `LateUpdt` hook the same frame.
    2. Re-position the dependent thing every frame ("repositioned each frame since the tab layout resolves a frame after BuildTabs").
    3. Compute the layout analytically from known constants and don't ask the engine.
- **Destroy is deferred.** `Ui::Destroy(handle)` marks the whole subtree destroyed immediately (safe: bound-pointer texts stop being read), nulls your handle, and frees the objects in the next `DestroyUpdate`. Children of a destroyed canvas are cascade-deleted — destroy the root, never the children individually. **Gotcha:** the reaped element fires `unhover` during that deferred reap, *before* the replacement canvas has its first layout — if an `unhover` handler closes a panel, guard it by checking the signal's element against your *current* handles (madman `SettingSys` does exactly this).
- Exceptions that are immediate, not deferred: `Ui::UpdateText` relayouts the parent synchronously when the text size changed, and bound-pointer texts (`ValuesUpdate`) do the same when a value change resizes them.
- Ownership: widgets are heap objects owned by the UI tree. `Ui::Destroy(x)` nulls `x`; after destroying a canvas, null every handle you kept to its descendants — they are dangling. House style in all three games: store handles on the system's Data component, build in an enter function that destroys first (idempotent), tear down with one root `Ui::Destroy` + null all handles.
- `UiElem::destroySignal` fires on actual destruction; `Ui` itself listens on it, so your own listeners run too (`SignalPriority` matters if you touch the tree).

---

## 5. The layout engine, exactly

`Ui::Begin(size, frame)` pushes a canvas on the build stack; every widget created until the matching `Ui::End()` becomes its child, laid out in creation order. Canvases nest (stack), and `End` marks the canvas dirty.

**Size argument** (`Begin(size)` / `SetSize`):

- Component `<= 0` → that axis auto-sizes to content (content + 2×padding).
- Component `> 0` → axis is **fixed** at that value, and it becomes the wrap limit (`sizeLimit`).
- A child canvas's limit is clamped by `parent limit − 2×parent padding`.

**Flow algorithm** (`UiCanvas::UpdateLayout`): a cursor starts at `(padding.x, -padding.y)` (top-left inside padding, Y goes down as negative). Children are placed top-left at the cursor, then the cursor advances by child size + `spacing` — downward for `LayoutDir::TopDown`, rightward for `LeftRight`. When the *fixed* axis limit would be exceeded, the flow **wraps**: TopDown starts a new column to the right, LeftRight starts a new row below. So a fixed-size canvas is also a wrapping grid — `TheReclaim` builds card grids this way.

**Alignment** (`SetCanvasLayoutAlignH/V`, values `Begin`/`Middle`/`End`): offsets whole lines/columns within `size − 2×padding`. `Begin` is the no-op default. Alignment is applied after auto-size stretch, so it mostly matters on fixed-size canvases (in an auto-sized canvas content defines the size and there is no slack, except cross-axis slack between lines of different thickness).

**What the knobs really do:**

| Knob | Scope | Truth |
|---|---|---|
| `SetCanvasPadding` | inner inset of the canvas | works as expected |
| `SetSpacing` | gap between children | works as expected |
| `SetMargin` | per-element outer margin | **effectively dead**: `AddElem` zeroes the child's margin (engine comment "should be margin"). Only affects orphan elements. Don't use; use spacing/padding. |
| `SetRootMargin` | root-canvas inset from its anchor point | works, per the anchor table in §2 |
| `elem->SetAbsolutePos(true)` | opts the element **out of the flow** | it still follows the parent (position is relative to the parent's top-left, Y negative going down) but the layout skips it and the canvas doesn't size around it. This is the overlay/badge/scrollbar mechanism — used ~40× in madman. |

**Context setters persist.** `SetCanvasLayoutDir` etc. mutate the live context and stay set for every later `Begin`, including after `End`. The games re-issue the direction/align/padding/spacing preamble defensively before nearly every `Begin` — do the same, or reset with `Ui::Context("name")`.

**Frames force sizes.** `Begin(size, true)` draws the context's canvas frame — **crashes (null deref) if the context has no canvas frame template**, so `Ui::SetCanvasFrame` must have been called (or a context restored) first. A frame can't be smaller than 2× its corner sprite, and a `fixedStep` template snaps the frame up to multiples of the corner size; the canvas **adopts** the snapped size. `Begin(..., false)` creates an invisible layout container (the most common form in all three games).

**Layout-stability tricks from madman** (fight reflow, not the engine):

- Fixed-size shell canvas holds a grid slot while its inner content is destroyed/swapped — siblings never reflow.
- Fixed-width invisible column sub-canvases (`Begin({W*0.35f, 0}, false)`) make table rows align across rows — the standard "two-column key/value" idiom in every game.
- Toggle highlight by `Ui::ChangeFrame(canvas, "other_frame.png")` between two templates **with identical border geometry** — swapping never reflows.
- Give text an explicit `width` (or put it in a fixed-width canvas) to get word-wrap; otherwise a long line stretches the whole panel.

---

## 6. Hit-testing and input

- Hit-testing runs against a flat **hoverables** list: buttons, anim-buttons and checkboxes register automatically; a `UiTxt`/`UiImg`/`UiCanvas` becomes hoverable only when you attach any `ListenHover/Unhover/ClickPressed/ClickReleased`.
- **No occlusion.** Every hoverable under the mouse hovers, and a click fires on *all* hovered clickables simultaneously — z does not shield. If two interactive things can overlap (tooltip over a button), disable one side (`Disable()`, or interaction groups) while the other is up.
- **Click swallow:** `Ui::OnEvent` runs before `Inputs` and all game event systems. A left press/release is consumed when the mouse is over *any* hovered element **or any root canvas** — including invisible `frame=false` containers. A full-screen invisible canvas therefore eats every world click (`TheReclaim` uses this deliberately as a modal shield). Only the left button is UI-handled; right/middle/wheel always pass through — wheel scrolling must be read from `Inputs`/SDL yourself.
- Hover state updates once per frame in `Ui::Update` from `Ui::MousePos()`; hitboxes are the laid-out rects (top-left + size). A freshly built canvas is not hoverable until its first layout.
- `Disable()` greys a button (disabled frame + disabled text color) and stops click dispatch; `Enable()` reverses. Disabled elements also stop hovering unless `SetHoverableWhenDisabled(true)` — the trick meat/TheReclaim use so a disabled buy button still shows its tooltip. After rebuilding UI under a static mouse no `hover` re-fires until state changes — `TheReclaim` re-fires manually: `if (btn->IsInside(Ui::MousePos())) OnHover(...)`.
- **Interaction groups** (`Ui::SetInteractionGroup(n)` context value stamped on every new element): `EnableGroup` / `DisableGroup` / `EnableOnlyGroup` / `EnableAllGroups` bulk-Disable/Enable all member elements and broadcast `Ui::GetGroupSignal()`. Two caveats: a group must have been *seen* by `SetInteractionGroup` before Enable/Disable works (else `LOG_ERROR` no-op), and enabling a group calls `Enable()` on **every member**, clobbering individually-disabled buttons — re-apply per-button disables from a `GetGroupSignal` listener (`TheReclaim` does). madman and meat skip groups entirely and layer with z + manual `IsHovered` guards instead; both approaches are legitimate.
- Manual queries: `Ui::MousePos()` (mouse in UI units), `elem->IsInside(uiPos)`, `Ui::GetCanvases()` (root map copy).

### Callback anatomy

Listeners are member-function + object pairs, payload is the element pointer:

```cpp
btn->ListenClickReleased(&MySys::OnBuy, this);     // void OnBuy(UiElem* sig)
btn->ListenClickPressed(...);                      // press feedback / hold-to-repeat start
elem->ListenHover(...); elem->ListenUnhover(...);  // tooltips
```

To route N widgets into one handler, attach a payload entity: `Entity e = Entity::Create(); e.Add<int>(i); btn->SetData(e);` then `int i = *sig->GetData().Get<int>();`. The widget **owns** the data entity (destroyed with it; `SetData` destroys any previous). This is the standard pattern in all three games.

Click-released only fires if the press started on the same element (`pressed` flag); moving off and releasing cancels. `ListenClickPressed` + `Released` pairs implement hold-to-repeat (volume steppers) and drag (scrollbar) — track the held element yourself in `Update`.

---

## 7. Widgets, exact behavior

### Text — `Ui::Text(utf8, width = -1)` / `Ui::TextLoc(key, width)`

- `width > 0` enables word-wrap at that many units and is **required for `TextAlign` to do anything** — `SetTextAlign(Center/Right)` with `width <= 0` silently does nothing. Alignment centers lines relative to the *longest laid-out line*, not the requested width. Inside a fixed-width canvas, text auto-wraps at `parent limit − 2×padding` even with `width = -1`.
- `\n` works. `|iconId|` inserts an inline icon glyph (§9).
- **Data binding** — `Ui::Text("{0}/{1}", -1.f, &a, &b)`: pointers (up to several) are dereferenced and compared **every frame**; on change the text re-renders (and immediately relayouts the parent if the size changed). Placeholders are `std::format` style. Rules:
    - Pointers must **outlive the widget**. Games bind to Data-component members or `static` locals — never to stack variables.
    - Works with the localized overload too: `Ui::TextLoc("hud_wave", -1.f, &shown, &total)`.
    - `txt->SetCompact(true)` renders numeric args through `Math::FormatCompact` ("1.5k"). It is number formatting, nothing to do with spacing.
    - Float compare uses a relative 1e-6 epsilon — a per-frame animated float re-renders (and possibly relayouts) every frame; quantize the bound value (round it, or bind an int) for counters you tween.
- `Ui::UpdateText(txt, s, replaceUtf8)` swaps content in place, keeping the original wrap width; `replaceUtf8=false` keeps the stored format template (use with bound text). `txt->SetColor(c)` re-renders glyphs in a new color.
- Localized text re-renders automatically on `Assets::SetLang`. `TextLoc` requires the `Textual` asset to exist for the current language.

### Button — `Ui::Button(utf8)` / `Ui::ButtonLoc(key)`

- 9-slice frame button, size = label + 2×`SetButtonPadding`. Requires at least the idle frame template in the button context (assert otherwise); hover/pressed/disabled fall back to idle. All four frames are built once and toggled by alpha — state flips are cheap.
- `ListenClickReleased` is the "activate" event. Press nudges the label (−1,−1); optional context press/release sounds.
- `btn->SetWidth(w)` widens a button (label re-centered) — the way to give a column of buttons uniform width (measure the widest, apply to all). No-op if `w` ≤ natural width.
- `btn->Disable()/Enable()` swap frame + label color. `SetLabelColor` tints the label until the next enable/disable; `SetColor` tints idle/hover/pressed frames (not the disabled frame — quirk).
- `Ui::UpdateBtn(btn, s)` re-renders the label and resizes the button (relayouts parent).

### AnimButton — `Ui::AnimButton(utf8)` / `AnimButtonLoc`

- Sprite-animation button: context animations idle/hover/pressed/disabled (`Ui::SetAnimBtnIdle/...`; idle mandatory, others fall back to idle). Size = idle frame 0 sprite size, independent of the label; label drawn on top with button padding offset.
- The animator runs **unscaled time** — keeps animating while the game is paused.
- Sprite origin is compensated: the widget is top-left anchored like everything else regardless of the sprite's origin. Used for all icon-style buttons in the games (arrows, reroll, fight...). Often created with an empty label: `Ui::AnimButton("")`.

### Image — `Ui::Image(name)` / `Ui::Image(Sprite*)`

- Single sprite, size = sprite pixel size, origin compensated to top-left. `SetSprite` swaps at runtime (re-anchors), `SetColor` tints.
- Not interactive until you attach a listener. Images with `ListenClickPressed` are the games' lightweight icon-buttons.
- The "scaled square" idiom: a 1×1 (or small) white `square.png` image stretched via `img->root.gtr()->SetScale(...)` makes backings, scrims, glows, scrollbar tracks — the engine has no rectangle-fill primitive. Set `SetAbsolutePos(true)` on it so layout ignores it.

### Checkbox — `Ui::Checkbox(bool* value = nullptr)`

- Needs `Ui::SetCheckboxSprites("checkbox.png")` — a **1×3 horizontal** sheet: unchecked, hovered, checked (sprites `_0_0`, `_0_1`, `_0_2`).
- Truth about the `bool*`: it is **read once at creation** (to set the initial visual) and **written on every click**. It is *not* polled per frame — external changes to the bool do NOT update the visual. Drive external changes through `box->SetChecked(b)`. `checkSignal` fires on every user flip.

### LoadBar — `Ui::LoadBar(size, goal, current)`

- Contour frame + filling frame (both from context: `SetLoadBarFrameContour/Filling`, both mandatory), `fillingMargin` inset, `fillingColor` tint, optional label (`SetLoadBarTextMode`: `None`/`Percent`/`ValueGoal`, plus font/color; `bar->SetCompact`, `bar->SetSuffix`).
- Update with `bar->SetProgress(current, goal)` (visual only) or `Ui::UpdateLoadBar(bar, current, goal)` (also relayouts the parent). Filling is fully hidden below `current < 0.01` (the frame's minimum-size artefact workaround).
- Each `SetProgress` **destroys and rebuilds the filling frame's 9 entities** — don't call it hundreds of times per frame; batch with a dirty flag and update once in `Update` (madman `XpSys` does exactly this).
- **Landmine (unfixed engine bug): a LoadBar with any sibling in the same canvas triggers a use-after-free.** Always wrap each load bar in its own single-child canvas and compose rows by positioning the canvases. Both madman comment sites treat this as law.

### Canvas — extra ops

- `canvas->SetSize({w,h})` fix/release axes at runtime (>0 fixes, ≤0 releases), `SetSpacing`, `SetMargin` (root only, see §2), `MustUpdate()` to force a relayout, `Ui::ChangeFrame(canvas, "tpl.png")` to swap the background frame.
- `canvas->AddElem(elem)` grafts an existing element (re-parents it, resets its z to `parent−5`, zeroes its margin, appends it to the flow order). Used to inject cross-system panels.
- `canvas->GetAnchoredPosition()` = raw position sum without the anchor offset (rarely what you want; prefer `GetPosition`).

---

## 8. Contexts: what they really capture

`Ui::Set*` mutate one live `UiContext`. `Ui::SnapshotContext("name")` deep-copies **everything**: canvas style (frame/padding/spacing/dir/align), text style, the whole button context, anim-button animations, checkbox texture, load-bar style, margins, **layer, material, rootAnchor, z (SetOrder), interactionGroup**. `Ui::Context("name")` restores all of it — including z and interaction group, which surprises people: restoring `"default"` mid-build resets your `SetOrder`. Set order/group *after* `Ui::Context(...)`.

- `SnapshotBtnContext` / `BtnContext` do the same for the button subset only.
- Snapshot names must be unique — a second `SnapshotContext("x")` is a logged no-op (the old snapshot wins). Snapshots are made once in `InitUI()`; you cannot re-bake them later.
- Frame templates must exist (`Ui::MakeFrameTemplate`) before any `SetCanvasFrame`/`SetButtonFrame*`/`SetLoadBarFrame*` referencing them (logged no-op otherwise).
- `Ui::SetMaterial(mat)` swaps the material for subsequently created widgets (it auto-sets `uPpu` on it); `Ui::ResetMaterial(x)` **ignores its argument** and restores the default material. `Ui::SetLayer` moves subsequent widgets to another layer (they keep UI-unit coordinates — only meaningful for layers with a UI-style shader).
- House convention (all games): one `InitUI()` called before `Engine::Launch` builds fonts → icons → frame templates → button snapshots → canvas snapshots. Screens then start with `Ui::Context("default")` (or a purpose snapshot like `"description"` ≡ default + `SetOrder(-50)`), tweak a few setters, and build.

Minimal viable init (order matters):

```cpp
void InitUI()
{
    Ui::MakeFont("Silver.ttf", "p", 19, 1.f, .7f, Assets::GetAvailableLangs());
    Ui::GetWriter()->SetLineAdjustement(0.3f);

    Ui::MakeFrameTemplate("frame_black.png");
    Ui::MakeFrameTemplate("btn_blue_idle.png");
    Ui::MakeFrameTemplate("btn_blue_hover.png");
    Ui::MakeFrameTemplate("btn_blue_pressed.png");
    Ui::MakeFrameTemplate("btn_disabled.png");

    Ui::SetButtonFont("p");
    Ui::SetButtonPadding({16.f, 6.f});
    Ui::SetButtonTextColor(Color::White);
    Ui::SetButtonFrame("btn_blue_idle.png");
    Ui::SetButtonFrameHover("btn_blue_hover.png");
    Ui::SetButtonFramePressed("btn_blue_pressed.png");
    Ui::SetButtonFrameDisabled("btn_disabled.png");
    Ui::SnapshotBtnContext("blue");

    Ui::SetTextFont("p");
    Ui::SetCanvasFrame("frame_black.png");
    Ui::SetCanvasPadding(4);
    Ui::SetSpacing(4);
    Ui::SetCanvasLayoutDir(LayoutDir::TopDown);
    Ui::SetRootAnchor(CanvasAnchor::MiddleCenter);
    Ui::SnapshotContext("default");
}
```

---

## 9. Fonts, text rendering, inline icons

- `Ui::MakeFont(file, fancyName, uiSize, scale, lineHeight, langs, outlineThickness, outlineColor)` — `uiSize` is the glyph height in UI units ≈ pixels (19 for "Silver" body text, 8–10 for small pixel fonts in the games). Empty `langs` = all available languages. An outlined variant is a *separate* font (`"p_outline"`/`"po"` in the games) used for text over busy art.
- Glyphs bake lazily into shared atlases. Fonts resolve per current language: `writer->GetFont(fancyName, lang)`.
- `Ui::GetWriter()->SetLineAdjustement(0.3f)` — every game sets this; it nudges the baseline up by `0.3 × fontSize`.
- **Inline icons:** `writer->AddIcon("gold", Assets::Get<Sprite>("token.png_0_0"), Vec2f(0,-1))` then any rendered string containing `|gold|` embeds the sprite as a glyph. The `Vec2f` offset is a hand-tuned baseline nudge (games use 0 to −6). Unknown icon ids log an error and render the fallback glyph. Icons can be registered at any time (madman registers input-device glyphs at runtime and swaps `"{0}"` for `"|button_a|"` vs `"|mouse_left|"` depending on the active device).
- **Floating world text without a widget:** `Ui::GetWriter()->Write(str, ...)` returns a `Sentence` (root entity + glyph children) you own — position its root, animate it, `sentence.root.Destroy()` when done. All games use this for damage numbers / gain popups. Combine with `Ui::WorldToUi` or spawn it on a world layer with a world font.

---

## 10. 9-slice frames and spritesheet naming

- A frame texture is a **3×3 spritesheet** (with a `.texture` file setting cell `Width`/`Height`): 4 corners, 4 repeating edges, repeating center. `Ui::MakeFrameTemplate("frame.png", fixedStep=false)` registers it once, by texture name.
- **Sprite naming (engine-wide truth): `file.png_<row>_<col>`, row 0 = TOP row, col 0 = LEFT column.** (Assets flips image rows at load; the doc string `<col>_<row>` seen elsewhere is wrong.) So `frame.png_0_0` is the top-left corner cell, `_1_1` the center, `_2_2` bottom-right. Checkbox `_0_0/_0_1/_0_2` = the three cells of a horizontal strip, left to right.
- `fixedStep = true`: the frame only grows in whole multiples of the corner-sprite size (for dashed/patterned borders that must not shear). The canvas adopts the snapped size.
- Frames render from the element's top-left, at minimum 2× the corner size. State-swap trick: author two templates with identical border geometry and toggle with `Ui::ChangeFrame` — zero reflow.

---

## 11. Gamepad navigation (full subsystem, madman exercises all of it)

- **Setup (init):** `Ui::SetGamepadSelector("gamepad_selector.png", {2,2})` — a **2×2** sheet (4 ring corners); `Ui::ClickIsSelect(true)` makes gamepad Select fire the element's existing `ListenClickPressed/Released` handlers, so one handler serves mouse and pad.
- **Wiring inputs:** bind your `ui_left/right/up/down/select/cancel` `ButtonInput`s to `Ui::NavigateLeft/Right/Up/Down/Select/Cancel` (the inputs need signal on press *and* release for Select/Cancel). Analog stick needs a game-side adapter that converts deflection into discrete `Navigate*` calls with repeat (madman `UiStickNavSys`).
- **Per screen:** build a directional graph with `a->AddNav(NavDir::Right, b)` (one-directional — wire both ways yourself), give initial focus with `elem->Navigate()`, read/force focus with `Ui::GetNavigated()/SetNavigated()` (pass nullptr to clear). The selector ring only draws while the last input device was gamepad; it survives device flips.
- Navigation does **not** emit hover events. Screens that restyle on hover must also poll `GetNavigated()` each frame and repaint the focused element (madman `SyncCardSpritesToNavigated` pattern). `ListenNavPressed/Released(dir,…)`, `ListenSelectPressed/Released`, `ListenCancelPressed/Released` exist per element for custom behavior (steppers, back-out).
- Navigating away from a held element cancels the press without firing the release callback (engine `ResetPress`).
- Destroyed elements clear themselves from navigation automatically; after a rebuild, re-`Navigate()` the new element.

---

## 12. Coordinate conversions

| Function | Use for | Notes |
|---|---|---|
| `Ui::MousePos()` | hover/hit tests in UI space | = `ScreenToUi(Mouse::GetPosition())` |
| `Ui::ScreenToUi(px)` | window pixels → UI units | `ScreenToUi({0,0})` / `ScreenToUi(Window::GetSize())` gives the real UI extents of the window (transition wipes use this instead of assuming ±320) |
| `Ui::WorldToUi(world)` | pin a panel/floating text to a world object | goes through `Camera::main` (zoom-aware). Clamp the result to keep panels on-screen |
| `Ui::UiToWorld(ui)` | inverse of the above | unused by all three games; prefer working in one space |
| `Mouse::GetWorldPos()` | world-space picking | not a Ui API — use for clicking world objects, with your own Hoverable component (meat `HoverSys` pattern) |

World-object hovering is **not** the UI system's job: games attach their own `Hoverable{Rect,…}` component + a system that rect-tests `Mouse::GetWorldPos()`, then *bridge* into `Ui` panels from those callbacks.

---

## 13. Proven patterns (steal these)

- **Screen skeleton** (the house style everywhere):

```cpp
void MySys::CreateScreen()
{
    DestroyScreen();                       // idempotent: Ui::Destroy(_d->root) + null all handles
    Ui::Context("default");
    Ui::SetRootAnchor(CanvasAnchor::TopLeft);
    _d->root = Ui::Begin({0,0}, true);
        Ui::TextLoc("title");
        Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);
        Ui::Begin({0,0}, false);           // invisible row
            Ui::Image("token.png_0_0");
            _d->count = Ui::Text("{0}", -1.f, &_d->tokens);   // bound to Data member
        Ui::End();
        auto b = Ui::Button("Buy");
        b->ListenClickReleased(&MySys::OnBuy, this);
    Ui::End();
    _d->root->SetPosition({-320.f, 180.f});
}
```

- **Update strategy ladder** (cheapest first): bound-pointer `Text` for values that change often → `UpdateText`/`UpdateBtn`/`UpdateLoadBar` for sparse changes → destroy + rebuild the panel for structural changes. Rebuild is the *normal* answer for structure — all games rebuild whole panels on hover/state change without perf issues; what they avoid is per-frame rebuilds and per-frame `SetProgress` storms (dirty-flag them).
- **Tooltip / floating description:** separate root canvas on a front `SetOrder` context (e.g. `"description"` = default −50), rebuilt on hover, positioned off the hovered element: `auto tl = elem->GetPosition();` (laid-out top-left) + offsets, or at `Ui::MousePos()`, or `Ui::WorldToUi(worldPos)` clamped. Choose the anchor by screen side so the panel opens inward (meat picks TopLeft/TopRight/BotLeft/BotRight based on which half of the screen the source sits in).
- **Uniform button columns:** create all, find max `GetSize().x` (after layout, or from `SetWidth` on known text), `SetWidth(maxW)` on each.
- **Modal:** `MiddleCenter` root at front order; block the world either with a full-screen invisible canvas (auto click-swallow) or `EnableOnlyGroup(modalGroup)` if you committed to groups.
- **Hide/show:** there is no visibility API. Either destroy/rebuild, or park the canvas off-screen (`SetPosition({-9999, 0})` — used for blink effects) — remember a parked canvas still hit-tests at its hitbox location (off-screen = harmless).
- **Scroll region:** no engine scroll widget. Pattern (madman `ScrollBarUi`, meat `ResearchSys`): fixed-size viewport canvas, content offset by `SetPosition`, scaled-square track+thumb with `SetAbsolutePos(true)` + `SetZOffset`, wheel read from inputs, drag via `ListenClickPressed` + per-frame mouse delta.
- **Compositing game sprites into UI:** any ECS entity renders with the widgets if you stamp `spr->SetLayer(Ui::GetLayer()); spr->SetMaterial(Ui::GetMaterial()->GetID());` — its Transform is then in UI units. This is the sanctioned escape hatch for animated decorations, particles over panels (`ParticleEmitter.OnLayer(Ui::GetLayer()).WithMaterial(...)`), custom bars, tech trees. Position such entities relative to widgets via `elem->root.gtr()->GetPosition()` / `elem->GetPosition()` + `GetSize()`.
- **Element-attached payloads:** `SetData(entity)` for callback routing (§6); `elem->root` is the backing entity when you need to `AddChild` extra visuals that live and die with the widget.

---

## 14. Landmines and API traps (definitive list)

| Trap | Truth |
|---|---|
| Canvas anchor | Chooses which point of the canvas sits at `position`; does NOT dock to the screen. Position `(0,0)` is screen center. See §2. |
| `GetSize`/`GetPosition` after building | Zero/stale until the next `Ui::Update` (LateUpdt or next frame). §4. |
| `UiLoadBar` with siblings | Use-after-free. One load bar per canvas, always. |
| `Ui::Destroy` | Deferred; fires `unhover` on reap a frame later — guard unhover handlers against stale elements. Handles into a destroyed subtree dangle; null them. |
| Overlapping clickables | No z-occlusion; all hovered clickables fire. Disable the covered ones. |
| Full-screen invisible canvas | Swallows every left-click in the game. Sometimes that is the point (modal); never accidental. |
| `Ui::SetMargin` | Dead for canvas children (zeroed on add). Use spacing/padding. |
| `Ui::PPU` | Works (former uniform-name typo `uPPu` vs `uPpu` fixed). Rescales the whole UI — call before building widgets, not mid-frame. |
| `Ui::ResetMaterial(m)` | Ignores its argument; restores the engine default material. |
| `Checkbox(bool*)` | Bool is written on click and read once at creation — not synced per frame. Push external changes with `SetChecked`. |
| `SetTextAlign` | Only applies when the text has `width > 0`; centers against the longest line. |
| `Begin(size, true)` without a canvas frame in context | Null-deref crash. Restore a context or `SetCanvasFrame` first. |
| `Ui::Context(...)` mid-build | Restores z (`SetOrder`), interaction group, layer, material too — reapply your overrides after. |
| `EnableGroup` | Re-enables every member, clobbering individual `Disable()`s; re-apply from `GetGroupSignal`. Groups must be introduced by `SetInteractionGroup` before use. |
| Bound text pointers | Must outlive the widget (Data members / statics). Animated floats re-render every frame — bind quantized values. |
| Per-frame `SetProgress` / rebuilds | Each rebuilds entities; dirty-flag and apply once per frame. |
| Sprite names | `tex.png_<row>_<col>`, row 0 = top. Frames 3×3, checkbox 1×3, gamepad selector 2×2. |
| `UiBtn::SetColor` | Tints idle/hover/pressed frames but not the disabled frame. |
| Snapshot names / frame template names | Must be unique; duplicates are logged no-ops keeping the original. |
| `OnImGui` | Render thread — never touch `Ui` there. |
| Missing `ui.shader` | Boot crash: the game must ship `assets/shaders/ui.vert` + `ui.frag`. |

---

## 15. Checklist for a brand-new SandCastle game

1. Copy `ui.vert`/`ui.frag` into `assets/shaders/`, a `.ttf` into `assets/fonts/`, frame + checkbox spritesheets (with `.texture` grids) into `assets/textures/`.
2. `Camera::main` constraints: `Constraints c; c.SetDefault(); cam->SetConstraints(c);` if you want the fixed 640×360 UI box the standard positions assume.
3. Write `InitUI()` (§8 template): fonts → `SetLineAdjustement` → icons → frame templates → button snapshot(s) → `"default"` context snapshot (+ a `"description"` snapshot at front order). Call it before `Engine::Launch`.
4. Optional gamepad: selector texture (2×2) + `SetGamepadSelector` + `ClickIsSelect(true)` + bind nav inputs to `Ui::Navigate*`/`Select`/`Cancel`.
5. Per screen: handles on the system's Data, idempotent `Create...` (destroy-first), one root `Ui::Destroy` teardown that nulls every handle, `AddNav` graph + `Navigate()` if gamepad matters.
