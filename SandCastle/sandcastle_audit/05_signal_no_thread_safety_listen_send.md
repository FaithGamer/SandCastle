# 05 — Signal has no thread safety — Listen/Send/StopListen are racy

**Severity:** Tier 1
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** corruption inside `std::set` internals (red-black tree imbalance), often surfacing on an unrelated later `Send`. Very hard to reproduce reliably.

## What's broken

`Signal<T>::m_listeners` is a plain `std::set<sptr<OpaqueCallback>>` with no synchronization. `Listen`, `Send`, and `StopListen` can all be called from different threads. The engine explicitly fires signals from both main and render threads — there's already a documented cross-thread Send (Window resize: rendered from main, listened by render thread).

## Where (verified)

- `include/SandCastle/Core/Signal.h:134` — `std::set<sptr<OpaqueCallback>> m_listeners;` (no mutex).
- `src/Render/Renderer2D.cpp:159` (executed on render thread inside `PostAssetInitThread`):
  ```cpp
  Window::GetResizeSignal()->Listen(&Renderer2D::OnWindowResize, this);
  ```
- `src/Render/Window.cpp:305-310` (executed on main thread):
  ```cpp
  void Window::OnSDLPixelSizeChanged(SDL_Event& event) {
      ...
      ResizeSignal.Send(m_pixelSize);
  }
  ```
- All Listen/StopListen/Send methods in `Signal.tpp` mutate or iterate `m_listeners` without any mutex.

## Why it's wrong

`std::set` insert and iterate from different threads is UB. The set's tree-balance writes during insert can race the iterator advancing during Send. Less obviously: even read-only Send + read-only Listen-count from different threads is UB unless the implementation guarantees thread-safe `const` operations (libstdc++/MSVC do not, in general, for mutated containers).

## Why it scales with entity count

More entities → more listeners → larger sets → longer insert rebalances → wider race windows. Cross-thread signals (resize, language change, save events) become more contended as their listener counts grow.

## Suggested fix

Add an internal mutex around all `m_listeners` access.

```cpp
class Signal {
    mutable std::mutex m_mutex;
    std::set<sptr<OpaqueCallback>> m_listeners;

    void Listen(...) { std::lock_guard lk(m_mutex); m_listeners.insert(...); }
    void StopListen(...) { std::lock_guard lk(m_mutex); ... erase ... }
    void Send(T& s) {
        decltype(m_listeners) snapshot;
        { std::lock_guard lk(m_mutex); snapshot = m_listeners; }
        for (auto& l : snapshot) l->Call(s);  // dispatch unlocked, listeners can re-enter Listen/StopListen
    }
};
```

The snapshot-then-unlock pattern also fixes `03_signal_send_iterator_invalidation.md` (re-entrant Listen/StopListen during Call is now safe — they touch the live set under the mutex, while the iteration walks the snapshot copy).

Performance is fine: signals are not hot-path for most engine code; the snapshot is `sptr` ref-bumps, not deep copies. If you hit a real bottleneck later, replace with `std::shared_mutex` for reader-mostly signals.

## Verify before fixing

```
grep -rn "m_listeners\|Signal<" SandCastle/include/SandCastle/Core/Signal.h SandCastle/include/SandCastle/Core/Signal.tpp
grep -rn "\.Send(\|->Send(" SandCastle/src/ madman/src/
grep -rn "\.Listen(\|->Listen(" SandCastle/src/ madman/src/ | head -50
```

Identify which signals are demonstrably cross-thread (Window::ResizeSignal is one; audit Animator keyframe signals, Assets::langSignal, gamepadModeSignal).

## Context for a fresh session

- Combine the fix with `03_signal_send_iterator_invalidation.md` (snapshot inside Send) — both bugs are fixed by the same `Send` rewrite.
- Combine with `04_signal_listener_use_after_free.md` for the full Signal hardening; without #04, the snapshot may still call `Call` on a freed object.
- Listening from `OnImGui` (which runs on the render thread) is a known pattern in some engines but is currently de facto racy here.
