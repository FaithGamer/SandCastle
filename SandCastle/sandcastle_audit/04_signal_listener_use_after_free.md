# 04 — Signal listener has no auto-removal on listener death (UAF time bomb)

**Severity:** Tier 1
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside a member function pointer call (`(static_cast<Obj*>(m_obj)->*m_method)(...)`). The `Obj*` points at freed memory. Often manifests as a corrupt vtable read.

## What's broken

`Signal<T>::Listen(method, obj)` stores `obj` as a raw `void*` in the listener's `Delegate`. There is no back-pointer from `obj` to the `Signal`s it joined and no destruction hook. If `obj` is destroyed without calling `Signal::StopListen(obj)` for every signal it joined, the next `Send` dereferences freed memory.

This is the classic C++ pub/sub UAF footgun. It is the most likely root cause of crashes that scale with object count — every uncleaned listener is an independent time bomb.

## Where (verified by direct source read)

- `include/SandCastle/Core/Signal.h:92` — `Delegate<void, Obj, T> delegate;` inside `MethodCallback<Obj>` stores `Obj*` raw.
- `include/SandCastle/Core/Delegate.h:124` — `(static_cast<Obj*>(m_obj)->*m_method)(...)` dereferences `m_obj` with no validity check.
- `include/SandCastle/Core/Signal.tpp:7-11` — `Listen` constructs a `MethodCallback<Obj>` and inserts it into `m_listeners`. No back-registration anywhere.

## Why it's wrong

The contract today is "every listener must manually call StopListen for every signal it joined, in its destructor". This contract is invisible at the listener site — adding `signal->Listen(...)` does not require adding `signal->StopListen(this)` anywhere. The compiler doesn't enforce it. There's no static analysis catch. With many signals and many entities subscribing, mistakes are inevitable.

## Why it scales with entity count

Per-entity listeners: every Animator's keyframe signal, every Ui element on `Assets::langSignal`, every system on input signals, etc. With 15k units alive, the listener count is large; any single stale entry is one `Send` away from a crash.

## Suggested fixes (pick one — they're mutually exclusive choices, not steps)

### Option A: introduce a base class with auto-cleanup (recommended)

```cpp
class SignalListener {
public:
    virtual ~SignalListener() {
        for (auto* s : m_joined) s->StopListenOpaque(this);
    }
    void RegisterSignal(SignalBase* s) { m_joined.insert(s); }
    void UnregisterSignal(SignalBase* s) { m_joined.erase(s); }
private:
    std::unordered_set<SignalBase*> m_joined;
};
```
`Signal<T>::Listen(method, obj)` calls `obj->RegisterSignal(this)`; `~SignalListener` auto-StopListens from every joined signal. Requires every listener to inherit `SignalListener` — invasive but bulletproof.

### Option B: weak observer (intrusive ref count)

Listeners hold an `sptr<Self>` and Signal stores `wptr<Obj>`. On Send, `wptr.lock()`; if null, skip. Tweak: `MethodCallback<Obj>` stores `wptr<Obj>` instead of `Obj*`.

Less invasive — only listener-bearing objects need to be sptr-managed. But changes the ownership model and Signal::Listen needs a different signature.

### Option C: leave as-is, add diagnostic instrumentation

In `~MethodCallback<Obj>` (or via a #ifdef DEBUG build), assert that `m_obj` is still in a "registered listeners" set, somehow. Most useful as a stopgap to find existing leaks before deciding A or B.

## Verify before fixing

```
grep -rn "Listen\|StopListen" SandCastle/include/SandCastle/Core/Signal.h SandCastle/include/SandCastle/Core/Signal.tpp SandCastle/include/SandCastle/Core/Delegate.h
grep -rn "Listen(" SandCastle/src/ madman/src/ | wc -l
grep -rn "StopListen(" SandCastle/src/ madman/src/ | wc -l
```

A mismatch between Listen and StopListen call counts is suggestive (not conclusive — one StopListen can match many Listens).

## Context for a fresh session

- Pair with `03_signal_send_iterator_invalidation.md` (fixing that one snapshot doesn't fix this UAF; both are needed).
- Once Option A or B is in place, audit existing listeners and replace their manual StopListen with the auto-cleanup base.
- This is the classical C++ observer-pattern lifetime trap. Search "boost::signals2 disconnect_all_slots" and "Qt QObject::deleteLater" for prior-art designs.
