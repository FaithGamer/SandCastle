# 03 — Signal::Send iterator invalidation on re-entrant Listen/StopListen

**Severity:** Tier 1
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation deep inside `Signal<T>::Send`'s range-for, often inside `std::set` internals. Intermittent, depends on listener behavior.

## What's broken

`Signal<T>::Send` iterates `m_listeners` with a range-based for and offers no protection against listeners that, during dispatch, call `Listen` or `StopListen` on the same signal (directly or indirectly via destroying an object that holds a listener). `std::set::erase` on the current iterator invalidates the range-for's cached "current" iterator; the next `++it` is undefined behavior.

## Where (verified)

`include/SandCastle/Core/Signal.tpp:73-88`:
```cpp
template<typename T>
void Signal<T>::Send(T& signal)
{
    for (const sptr<OpaqueCallback>& listener : m_listeners)
    {
        listener->Call(signal);   // user code free to Listen/StopListen on this signal here
    }
}
```

`StopListen` mutates `m_listeners` mid-iteration:
- `Signal.tpp:35` — `m_listeners.erase(callback_it); break;`
- `Signal.tpp:61` — `m_listeners.erase(callback_it++); break;`

## Why it's wrong

`std::set` erase invalidates the erased iterator only. The range-for loop has cached the iterator that's now invalid; the implicit `++it` is UB. Listeners that `StopListen` themselves (a common "fire-once" pattern), or destroy peers that `StopListen` from the same signal, will hit this.

Insert (`Listen` during Send) is safer for `std::set` than `std::vector` but still risky if the loop expectations are mutated.

## Why it scales with entity count

More entities → more listener-rich signals (langSignal, hover/click signals, gamepadModeSignal, every Animator's keyframe signals). More dispatches per frame, more listeners per dispatch, more likely a listener triggers a re-entrant Listen/StopListen on its own signal.

## Suggested fix

Snapshot the listener set before iterating. `sptr` copies are cheap:

```cpp
template<typename T>
void Signal<T>::Send(T& signal)
{
    auto snapshot = m_listeners;            // shared_ptr copy, ~one ref-bump per listener
    for (const sptr<OpaqueCallback>& listener : snapshot)
    {
        listener->Call(signal);
    }
}
template<typename T>
void Signal<T>::Send(T&& signal)
{
    auto snapshot = m_listeners;
    for (const sptr<OpaqueCallback>& listener : snapshot)
    {
        listener->Call(std::forward<T>(signal));
    }
}
```

Trade-off: a listener added during Send won't fire until the NEXT Send (acceptable; current behavior is undefined). A listener removed during Send may still fire once before its actual erase (also acceptable — combined with fix #04 you can null-check or use a "dead listener" flag).

Alternative: maintain a re-entrancy flag and defer Listen/StopListen mutations into a pending queue applied at end-of-Send. More complex, no obvious benefit over the snapshot approach.

## Verify before fixing

```
grep -n "Signal\|m_listeners" SandCastle/include/SandCastle/Core/Signal.h SandCastle/include/SandCastle/Core/Signal.tpp
```

Audit listener call sites: any that destroy an object inside their callback, or call `StopListen` on a signal they're currently receiving.

## Context for a fresh session

- Pair this with `04_signal_listener_use_after_free.md` (no auto-removal of dead listeners) and `05_signal_no_thread_safety_listen_send.md` (no mutex on the set).
- `std::set<sptr<OpaqueCallback>>` ordering is by `sptr` value (pointer compare), not by registration order — already chaotic; snapshot is the only safe fix.
