# 01 — WorkerThread data races (m_currentQueue, empty-check, TaskCount)

**Severity:** Tier 1
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** vector corruption / segfault inside `WorkerThread::Thread` or inside a queued render task. Intermittent, more frequent under load.

## What's broken

`WorkerThread` is the engine's single-threaded queue used by `RenderQueue` and by every game-side worker. Three concrete data races exist on its cross-buffer state — all verified by direct source read.

## Where (verified)

1. `include/SandCastle/Core/Worker.h:85` — `bool m_currentQueue;` is a plain non-atomic bool.
2. `src/Core/Worker.cpp:38-39` (`Queue()`, caller thread):
   ```cpp
   std::unique_lock queueLock(m_queueMutex[!m_currentQueue]);  // reads m_currentQueue OUTSIDE any lock
   std::unique_lock waiterLock(m_waiterMutex);
   ```
3. `src/Core/Worker.cpp:76` (`Thread()`, worker thread):
   ```cpp
   m_currentQueue = !m_currentQueue;   // writes m_currentQueue under m_waiterMutex only
   ```
4. `src/Core/Worker.cpp:91`:
   ```cpp
   m_haveTask = !(m_queue[0].empty() && m_queue[1].empty());
   ```
   Reads `m_queue[!m_currentQueue].empty()` while holding only `m_queueMutex[m_currentQueue]`. Concurrent with another thread's `emplace_back` at line 41.
5. `src/Core/Worker.cpp:57-60` (`TaskCount()`) — reads both `m_queue[i].size()` with **no lock at all**.

## Why it's wrong

Concurrent unsynchronized read+write of a plain `bool` is a C++ data race. On x86 a torn read is unlikely in practice, but the compiler may fold or reorder the read. The concrete failure mode: if `Queue()` reads a stale `m_currentQueue`, it locks the wrong queue mutex and `emplace_back`s into the same vector the worker is currently iterating. The vector's reallocation during emplace_back invalidates the worker's iterator — direct memory corruption.

The empty-check at line 91 and `TaskCount` read `std::vector::empty()`/`size()` while another thread is mid-`emplace_back`. The standard does not allow concurrent read of a mutating vector; vector internals (end pointer) can be torn.

## Why it scales with entity count

Heavy frames = many quads pushed → many tasks queued onto the renderer's `WorkerThread`. Game-side workers (`HordeGridState::worker`, `ProjectileEffectState::worker`, `LaserEffectState::worker`) also use `WorkerThread` and queue per-frame tasks proportional to unit count.

## Suggested fix

1. Change `Worker.h:85` to `std::atomic<bool> m_currentQueue;`. Read with `load(std::memory_order_acquire)` in `Queue()`, write with `store` (or simpler: `fetch_xor(1)` in `Thread()`).
2. For the empty-check at `Worker.cpp:91`, acquire both queue mutexes at once via `std::scoped_lock lk(m_queueMutex[0], m_queueMutex[1]);` (deadlock-free, scoped_lock uses std::lock).
3. For `TaskCount()`: either also `std::scoped_lock` both mutexes, or maintain an `std::atomic<size_t> m_pendingTaskCount` incremented in Queue / decremented in Thread.
4. After the fix, audit every call site to make sure no client relies on the lock-free `TaskCount()` for tight loops (grep for `TaskCount`, `HaveTask`).

## Verify before fixing

```
grep -n "m_currentQueue\|m_queue\[\|m_haveTask" SandCastle/src/Core/Worker.cpp
grep -rn "WorkerThread\|gs\.worker\|es\.worker" madman/src
```

Re-read `Worker.cpp:36-105` end-to-end. Make sure your patch preserves the lazy-wake semantics of `m_doneCondition`/`m_doneMutex`.

## Context for a fresh session

- `docs/PATTERNS.md` describes the threading model. The render thread is the only thread other than main; this WorkerThread is the bridge.
- `Worker.h:11-13` doc says "adding tasks to the queue is not blocking" — under the current race, that promise can be violated.
- The engine has one `WorkerThread` per `RenderQueue` (Renderer2D-owned) plus three per-game-system instances in madman. The pattern is widely used.
