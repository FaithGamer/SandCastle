# 13 — Ui::Destroy gap + UiCanvas cascade can double-delete or fire clicks on dead elements

**Severity:** Tier 3 (real but narrower window)
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside `UiBtn::ClickReleased` / hover handlers, or inside `Ui::DestroyUpdate`'s post-delete iteration after a cascade.

## What's broken

Two related UI lifetime bugs:

### A — "queued for destroy but still hot" window

`Ui::Destroy(elem)` only flags `destroyed = true` and queues the element into `m_destroy`. Actual delete happens at the next `Ui::Update()`. Between those points, the element still appears in `m_hovered` / `m_hoverables` / `m_pressed`. `OnClick` / `HoverableUpdate` (which may run during the same SDL event burst) dispatches signals on the queued-for-destroy element. If the click handler dereferences a Data field whose owning system already deleted its Data (state transition), UAF.

### B — Parent + child both queued in m_destroy → double-delete

`Ui::DestroyUpdate` computes `condemnedIds` but only uses it to filter `m_layoutUpdate` — not the `m_destroy` vector itself. `~UiCanvas` (line 8-15) deletes its children unconditionally. If a child is also in `m_destroy`, the destroy loop later tries to use `m_destroy[i]->GetID()` on freed memory.

## Where (agent-reported, verify line numbers)

### A
- `src/UI/Ui.cpp:1195-1203` — `Ui::Destroy` queues `m_destroy.emplace_back(elem)` and sets `elem->destroyed = true`.
- `src/UI/Ui.cpp:214-245` — `HoverableUpdate` iterates `m_hoverables` with no `IsDestroyed()` check.
- `src/UI/Ui.cpp:258-295` — `OnClick` iterates `m_hovered` with no `IsDestroyed()` check.
- `src/UI/UiElem.cpp:144-164` — `ClickPressed/Released` only check `disabled`, not `destroyed`.

### B
- `src/UI/Ui.cpp:94-148` — `DestroyUpdate`: builds `condemnedIds`, applies it only to `m_layoutUpdate` (lines 104-130). Then `m_destroy` loop at 132-145 deletes every queued entry; `~UiCanvas` cascade can free children that are still queued.
- `src/UI/UiCanvas.cpp:8-15` — `~UiCanvas` deletes children unconditionally.

## Why it's wrong

A: queuing destroy but leaving the element active in hover/click state breaks the invariant "events can only fire on live elements". State transitions in particular dispatch a click → DestroyMenu → DeleteData → next event in the same poll loop → click on a queued element whose Data is now nulled.

B: the `condemnedIds` filter logic was added for `m_layoutUpdate` but not extended to `m_destroy`. The cascade-delete from `~UiCanvas` is the missing piece.

## Why it scales with entity count

Indirect: the more UI elements and the higher the rebuild frequency (HUD churn, banners, upgrade picks), the more often a destroy queue contains overlapping subtrees. Not as load-correlated as the worker-thread or signal issues.

## Suggested fix

### A
Add destroyed-check at the top of click/hover paths:
```cpp
// UiElem::ClickPressed
if (destroyed) return;

// Ui::HoverableUpdate iteration
for (auto* h : m_hoverables) {
    if (h->IsDestroyed()) continue;
    ...
}
```
And drain `m_destroy` synchronously at the top of `Ui::OnEvent` (or call DestroyUpdate before dispatch) so a single SDL_PollEvent burst can't observe a stale element.

### B
Before the delete loop in `DestroyUpdate`, drop entries whose ancestor is also in m_destroy:

```cpp
auto end = std::remove_if(m_destroy.begin(), m_destroy.end(),
    [&](UiElem* e) {
        // Walk up the parent chain; if any ancestor is in condemnedIds (and is
        // not this same element), the cascade from that ancestor will free us.
        for (auto* p = e->parent; p; p = p->parent) {
            if (condemnedIds.count(p->GetID())) return true;
        }
        return false;
    });
m_destroy.erase(end, m_destroy.end());
```

Then proceed with the existing loop. Now the cascade-delete from `~UiCanvas` only frees elements that aren't separately queued.

## Verify before fixing

```
grep -n "m_destroy\|DestroyUpdate\|condemnedIds" SandCastle/src/UI/Ui.cpp
grep -n "destroyed\|IsDestroyed" SandCastle/src/UI/UiElem.cpp SandCastle/src/UI/UiBtn.cpp SandCastle/include/SandCastle/UI/UiElem.h
```

Trace a state-transition path (e.g. `MainSys::OnPlay → States::Set(Run) → OnExitMain → DestroyMenu`) and check whether any subsequent SDL_PollEvent could fire a click handler.

## Context for a fresh session

- `UiCanvas::~UiCanvas` already has `hasDestroyed = true` guard that prevents recursive destroys via `OnDestroy`. The double-delete in m_destroy is a different problem (the LIST hasn't been pruned).
- The `condemnedIds` set is computed at `Ui.cpp:104` — it's already collecting the right info, just needs to be applied to `m_destroy` too.
