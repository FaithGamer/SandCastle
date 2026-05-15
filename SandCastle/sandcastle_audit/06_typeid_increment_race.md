# 06 — TypeId::GetId increment is non-atomic; two types can collide on the same id

**Severity:** Tier 1 (subtle but real)
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** wrong listener erased by `Signal::StopListen`; surfaces later as a wrong-object dispatch or a use-after-free far from the cause. Or: wrong delegate cast inside a callback (`static_cast<WrongType*>`).

## What's broken

`TypeId::GetId<T>()` returns a process-unique integer id per type by incrementing a shared counter. The counter is a plain `int32_t`, not atomic. Two types initialized concurrently from different threads can both read the same counter value, increment locally, both store the same id, and end up sharing it.

## Where (verified)

`include/SandCastle/Core/TypeId.h:13-19`:
```cpp
template <typename T>
static int32_t GetId()
{
    static const int32_t id = currentTypeId++;   // ++ on non-atomic int32_t
    return id;
}
static int32_t currentTypeId;
```
`src/Core/TypeId.cpp:4` — `int32_t TypeId::currentTypeId = 0;`

## Why it's wrong

C++ magic-statics (block-scope `static`) serialize concurrent initialization of THE SAME T. They do NOT serialize concurrent initialization of two DIFFERENT Ts. The increment expression `currentTypeId++` is a non-atomic read-modify-write — two threads can both read `N`, both compute `N+1`, both store `N+1`, both yield id `N`.

Affected: every consumer of TypeId. The most dangerous is `Signal::StopListen` — at `Signal.tpp:26`, it checks `callback->Type() != TypeId::GetId<Obj>()`. If two types share an id, StopListen<A>(obj_a) can erase a listener registered as B (whose method pointer takes a B*, not an A*). The next Send invokes the method via the wrong type.

## Why it scales with entity count

Type ids are mostly stamped early on the main thread. Crashes from this are rare. But `Renderer2D::PostAssetInitThread` runs on the render thread and registers signals (Window::ResizeSignal.Listen, etc.) which can stamp new TypeIds concurrently with main-thread system pushes also stamping ids. Once the collision happens, every later subscribe/unsubscribe of those two types is wrong.

## Suggested fix

```cpp
// TypeId.h
class TypeId {
public:
    template <typename T>
    static int32_t GetId() {
        static const int32_t id = currentTypeId.fetch_add(1, std::memory_order_relaxed);
        return id;
    }
    static std::atomic<int32_t> currentTypeId;
};
// TypeId.cpp
std::atomic<int32_t> TypeId::currentTypeId{0};
```

The magic-static still serializes per-T initialization. `fetch_add` makes the cross-T increment safe. Memory order relaxed is fine — we only need atomicity, not ordering.

## Verify before fixing

```
grep -rn "TypeId::GetId\|currentTypeId" SandCastle/include SandCastle/src
```

Make sure no client serializes TypeId values (the header comment already says "not stable across builds — do NOT serialize", but check anyway).

## Context for a fresh session

- Symptom is sneaky: a wrong-type listener gets erased in StopListen, then later signal dispatches "miss" or call into a dangling listener. Stack traces will not point back to TypeId.
- This race is what makes the bug look "haunted" — fixing the obvious signal/iterator issues (#3, #4, #5) might leave residual mystery crashes that this addresses.
- One-line fix; very low risk.
