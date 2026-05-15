# 14 — AnimationSystem cached references can dangle across keyframe signal callbacks

**Severity:** Tier 3
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside the post-loop signal-send block, accessing a moved `Animator&`. Manifests during death-animation cascades.

## What's broken

`AnimationSystem::Update` iterates `Entity::View<Animator, SpriteRender, Transform>` and saves keyframe callback info into a local `calls` vector. After the view loop ends, it dispatches the signals via `animator.animations[c.name].signals[c.frame].Send(...)`.

The `animator` reference comes from the view's `.each` lambda — but it's used AFTER the `.each` returns. If any signal listener destroys an entity (extremely common — death animations trigger "kill self" signals), the Animator pool can compact and the cached `animator` reference dangles.

The dev comment at line ~61 already acknowledges "it is NOT safe to destroy the entity within the callback" — but doesn't go far enough; destroying other entities is also unsafe because Animator pool relocation moves the iterated reference.

## Where (agent-reported, verify lines)

`src/Render/AnimationSystem.cpp:15-67` (whole `Update` function):
```cpp
calls.emplace_back(*frame, animator.currentState, animator.currentStateName);
// loop continues over view.each
...
// AFTER view scope ends:
for (auto& c : calls)
    animator.animations[c.name].signals[c.frame].Send(...);   // animator may dangle
```

Comment at line 61 acknowledges the destroy-self hazard.

## Why it's wrong

`Entity::View::each` cleans up its iteration state when the lambda returns. After that, holding a reference to a component obtained inside `.each` is only safe if the pool hasn't been mutated. Signal callbacks (sent after the loop) can destroy other entities; entt's swap-and-pop relocates Animator components for entities after the destroyed one; the saved `animator` reference is now invalid.

The user's MEMORY note says entt views tolerate mid-iteration mutation — but only for the entity currently being visited. Post-iteration use of cached refs is a different case and is NOT covered.

## Why it scales with entity count

Death-animation cascades scale with on-screen entity count. With 15k units and combat firing many deaths per frame, the `calls` vector accumulates many entries; each Send's listener might trigger entity destruction; subsequent Send calls reference moved Animators.

## Suggested fix

Don't cache the `animator` reference across signal Send. Save the data you need directly into `calls`:

```cpp
struct Call {
    Signal<KeyframeSignal>* signal;
    KeyframeSignal payload;
    EntityId entity;
};
std::vector<Call> calls;

Entity::View<Animator, SpriteRender, Transform>().each(
    [&](EntityId e, Animator& animator, ...) {
        ...
        if (frame_advanced) {
            auto& state = animator.animations[animator.currentStateName];
            calls.push_back({ &state.signals[*frame], { animator.currentStateName, *frame }, e });
        }
    });

// Safe — no Animator& held across this loop
for (auto& c : calls) {
    if (!Entity(c.entity).Valid()) continue;   // skip if a previous Send destroyed it
    c.signal->Send(c.payload);
}
```

Caveat: the Signal pointer itself can dangle if the Animator was destroyed. The simplest fix is to copy the signal handlers out (Signal stores `sptr` listeners — copying the listener list out is cheap):

```cpp
struct Call {
    std::vector<sptr<OpaqueCallback>> listeners;  // snapshot
    KeyframeSignal payload;
};
```

Or — simplest — fire signals INSIDE the each lambda, with the documented contract "listeners must not destroy entities". Combine with file `03_signal_send_iterator_invalidation.md`'s snapshot fix in Signal::Send.

## Verify before fixing

```
grep -n "calls\|animator\.\|signals\[" SandCastle/src/Render/AnimationSystem.cpp
grep -rn "Keyframe.*Send\|animation.*signals" SandCastle/src/ madman/src/
```

Find any madman listener that destroys entities on keyframe signal — these are your first-victim test cases.

## Context for a fresh session

- Related to `03_signal_send_iterator_invalidation.md` — both are about safe dispatch.
- User's MEMORY note: "entt views tolerate mid-iteration mutation". That note is specifically about destroying the CURRENT entity inside `view.each`. Post-iteration deref of cached refs is NOT covered.
