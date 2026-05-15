# 17 — Small correctness bugs (Polygon2D AABB init, Inputs::DestroyInputMap typo, UiAnimBtn StopListen)

**Severity:** Tier 3 (none are crash sources today; all are clear bugs)
**Engine:** `C:/dev/SandCastle/SandCastle/`

Three independent small correctness bugs. Fix while you're nearby.

---

## 17A — Polygon2D::GetAABB initialization sentinels prevent any shrinkage

**Effect:** every Polygon2D body reports an AABB covering `[-1e8, +1e8]` → physics queries become O(N) instead of O(log N) → stalls under heavy collision counts.

`src/Physics/Collider.cpp:228-249` (approximate, verify):
```cpp
aabb.lowerBound.x = -99999999.f;
aabb.lowerBound.y = -99999999.f;
aabb.upperBound.x =  99999999.f;
aabb.upperBound.y =  99999999.f;
for (int i = 1; i < m_shapes.size(); i++) {        // starts at 1, skips m_shapes[0]
    b2AABB saabb;
    m_shapes[i].ComputeAABB(&saabb, ...);
    if (saabb.lowerBound.x < aabb.lowerBound.x) ... // never true: -1e8 is already very small
    ...
}
```

The sentinels are inverted (lower=-inf, upper=+inf), so the comparisons (`saabb.lower < aabb.lower`) are never true. The AABB stays at the sentinel forever. Also the loop starts at `i = 1` so `m_shapes[0]` is never included.

**Fix:**
```cpp
if (m_shapes.empty()) return aabb;  // or whatever the contract is
m_shapes[0].ComputeAABB(&aabb, ...);
for (int i = 1; i < m_shapes.size(); i++) {
    b2AABB saabb;
    m_shapes[i].ComputeAABB(&saabb, ...);
    aabb.Combine(saabb);             // box2d helper
}
return aabb;
```

---

## 17B — Inputs::DestroyInputMap adds instead of removing (typo)

**Effect:** DestroyInputMap leaks the input map (and possibly the call site assumes destruction happened, leading to stale active maps consuming SDL events).

`src/Input/Inputs.cpp:348-352`:
```cpp
void Inputs::DestroyInputMap(const String& name) {
    instance->m_inputMaps.Add(name);   // ← should be Remove(name)
}
```

**Fix:** `instance->m_inputMaps.Remove(name);`

Single-character fix. Verify the function's intent matches the name (check the `.h` doc comment).

---

## 17C — UiAnimBtn::~UiAnimBtn StopListen is silently a no-op

**Effect:** doesn't actually fire today (base `~UiBtn` cleans up afterwards), but is dead code that masks intent and would become a UAF if `~UiBtn` ever changes.

`src/UI/UiAnimBtn.cpp:10-14` (approximate, verify):
```cpp
UiAnimBtn::~UiAnimBtn() {
    if (keyLoc != "")
        Assets::Instance()->langSignal.StopListen(this);   // type-deduced as UiAnimBtn*
}
```

`src/UI/Ui.cpp:611-612` (where the listener was registered):
```cpp
Assets::Instance()->langSignal.langSignal.Listen(&UiBtn::OnLang, (UiBtn*)btn);
```

`Signal::StopListen` matches by `TypeId::GetId<Obj>()`. The Listen call used `UiBtn`, the StopListen uses `UiAnimBtn` — TypeId mismatch, no-op. The actual cleanup happens later via `~UiBtn` (UiBtn.cpp:8-12). Today this works because the addresses are identical (single-inheritance), but it's confusing dead code.

**Fix:** either
- remove the StopListen line from `~UiAnimBtn` (the base handles it), or
- match the registration: `Assets::Instance()->langSignal.StopListen((UiBtn*)this);`

Pair with file 06 (TypeId race): if you fix TypeId, audit all `StopListen(this)` calls in subclasses to confirm none rely on wrong-type matching.

---

## Verify before fixing

```
grep -n "Polygon2D\|GetAABB" SandCastle/src/Physics/Collider.cpp
grep -n "DestroyInputMap\|m_inputMaps" SandCastle/src/Input/Inputs.cpp
grep -n "StopListen\|langSignal" SandCastle/src/UI/UiBtn.cpp SandCastle/src/UI/UiAnimBtn.cpp SandCastle/src/UI/Ui.cpp
```

## Context for a fresh session

- 17A is a performance bug only — but at high entity counts performance bugs become timing-window crash multipliers.
- 17B was probably introduced by a copy-paste from `CreateInputMap`. Quick fix.
- 17C is a latent footgun — fixing 04 (signal listener UAF) will likely surface other similar mismatched StopListen calls; this is the first one.
