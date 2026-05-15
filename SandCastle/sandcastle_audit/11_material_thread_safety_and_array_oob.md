# 11 — Material setters race Material::Bind; AddPropertyArray writes past end()

**Severity:** Tier 2
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** garbage uniform values (NaN positions, wrong colors), escalating to access violations inside `glUniform*` calls during render.

## What's broken

Two distinct bugs in `Material`:

### A. Setters race Bind

`Material::SetFloat / SetInt / SetVec*f / Set*Array` are called from main-thread game code; `Material::Bind` is called on the render thread inside the per-batch flush. Both touch `m_properties` / `m_arrayProperties` (`unordered_map<String, MaterialProperty>`) with no synchronization. The array variants reassign whole `std::vector` — render thread reads `&prop.i[0]` against a possibly-reallocated buffer.

### B. AddPropertyArray writes past end()

`AddPropertyArray` calls `property.i.reserve(size)` and then writes `property.i[i]` in a loop. `reserve` does not change `.size()` — the writes are out of bounds.

## Where (agent-reported, verify lines)

### A
- `src/Render/Material.cpp:202-293` — `SetFloat/SetInt/Set*Array` mutate `m_properties` from caller thread.
- `src/Render/Material.cpp:17-54` — `Bind` iterates `m_properties` and `m_arrayProperties` on render thread.
- `src/Render/Material.cpp:278` — array variant: `it_prop->second.f = value;` (whole vector reassignment).
- `src/Render/Material.cpp:45-48` — Bind reads `&prop_kvp.second.i[0]`.

### B
- `src/Render/Material.cpp:148-181` — `AddPropertyArray`:
  ```cpp
  property.i.reserve(size);
  for (int i = 0; i < size; i++) {
      property.i[i] = 0;        // UB: indexing past end()
  }
  ```

## Why it's wrong

### A
Unsynchronized read+write of `std::unordered_map` value bytes is a data race. The array reassignment `it_prop->second.f = value` swaps the whole vector's internal pointer with no atomicity — render thread can see a half-swapped state, deref the old buffer (just freed) or read past the new buffer's size.

### B
`vector::reserve(n)` does not change `size()`. Indexing `operator[]` past `size()` is UB. The writes can corrupt adjacent allocations or the vector's internal bookkeeping. May "work" most of the time because the reserve has the memory laid out, but the vector's internal size pointer is wrong — the next `push_back` writes at the original `end()` (slot 0).

## Why it scales with entity count

A: any system that tweens shader uniforms per-entity (unit tint, damage flash, glow) increases the rate of concurrent setter calls. Per-quad uniform updates with 15k entities is bursty.

B: not load-related; will fire on the first material with array uniforms.

## Suggested fix

### A — thread safety

Two options:

1. **Double-buffer:** main-thread setters write into `m_pending`; `Renderer2D::Process()` snapshots `m_pending → m_live` at frame boundary; render-thread `Bind` reads only from `m_live`. Needs an internal mutex for the snapshot but no contention during the frame.
2. **Queue:** route every setter through `Renderer2D::m_queue.thread.Queue(...)` so it runs on the render thread.

Either works. Option 2 is simpler; option 1 is faster if setters are hot.

### B — fix AddPropertyArray

```cpp
property.i.assign(size, 0);      // sets size AND zeros
property.f.assign(size, 0.f);
```

## Verify before fixing

```
grep -n "m_properties\|m_arrayProperties\|reserve\|assign" SandCastle/src/Render/Material.cpp
grep -n "AddPropertyArray\|SetFloatArray\|SetIntArray" SandCastle/src/ SandCastle/include/
```

Run any code path that calls `AddPropertyArray` under ASAN — the OOB write should fire immediately.

## Context for a fresh session

- Materials are typically configured once at startup, then mutated only by post-processing or shader effects. If your project does no runtime mutation, A is academic. Check by greping for `SetFloat`/`SetVec*` after `Init`.
- B is a clear bug regardless of threading — fix it independently.
