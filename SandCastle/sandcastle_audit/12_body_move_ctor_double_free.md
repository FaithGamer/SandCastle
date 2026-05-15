# 12 — Body move ctor copies m_b2Body without nulling source (latent double-free)

**Severity:** Tier 2 (latent — currently no `Body` users in madman)
**Engine:** `C:/dev/SandCastle/SandCastle/`
**Crash signature:** access violation inside Box2D's broad-phase (`b2DynamicTree::DestroyProxy` or similar) on entity destroy / scene unload.

## What's broken

`Body`, `StaticBody`, `KinematicBody` move constructors copy `m_b2Body` into the destination but do NOT null the source's `m_b2Body`. The destructor unconditionally calls `m_b2Body->GetWorld()->DestroyBody(m_b2Body)` with no null check. Result: every entt swap-and-pop relocation of a Body causes a double-free of the underlying b2Body when both halves are eventually destroyed.

Also: `userData` is a struct member; `&userData` is stuffed into b2Fixture's `userData.pointer` (Collider.cpp). The move ctor doesn't rewire the fixtures' userData to the new Body's address — they keep pointing at the old (freed) Body's userData.

## Where (verified by direct source read)

`src/Physics/Body.cpp:8-17`:
```cpp
Body::Body(Body&& body) noexcept :
    m_layer(body.m_layer),
    m_mask(body.m_mask),
    m_YisZ(body.m_YisZ),
    m_b2Body(body.m_b2Body),         // copied
    m_colliders(body.m_colliders)    // copied
{
    // body.m_b2Body NOT nulled
}
```

`src/Physics/Body.cpp:23-27`:
```cpp
Body::~Body() {
    m_b2Body->GetWorld()->DestroyBody(m_b2Body);   // no null check
}
```

Same flaw in `StaticBody` move (`Body.cpp:148-155`) and `KinematicBody` move (`Body.cpp:163-171`).

## Why it's wrong

EnTT relocates components on destroy via swap-and-pop. Every time an entity with a Body is removed, every Body after it in storage is move-constructed into the freed slot. With N bodies, M removals → roughly N×M moves. Each move duplicates `m_b2Body` into both source and destination. When the storage shrinks at process end (or on `DestroyAll`), each duplicate fires `b2World::DestroyBody` again. Box2D's broad-phase is a tree of raw pointers — double-free corrupts the tree, next raycast deref's freed memory.

Userdata pointer is even worse: a moved Body's fixtures point at freed `&old_body.userData`. The next raycast that touches that fixture reads garbage, returning a bogus `EntityId` that the caller dereferences against the entt registry — UAF.

## Why it scales with entity count

Direct: more Bodies = more relocation moves per destroy. Box2D tree corruption probability rises with N.

## Current status in madman

`grep -r Body madman/src` returns no usage. So this is a **latent foot-gun**, not the cause of current crashes. Fix before anyone adds collisions.

## Suggested fix

```cpp
// Body.cpp
Body::Body(Body&& body) noexcept :
    m_layer(body.m_layer),
    m_mask(body.m_mask),
    m_YisZ(body.m_YisZ),
    m_b2Body(body.m_b2Body),
    m_colliders(std::move(body.m_colliders))     // move not copy
{
    body.m_b2Body = nullptr;                     // critical
    // rewire userData pointers in fixtures to point at THIS body's userData
    if (m_b2Body) {
        for (b2Fixture* f = m_b2Body->GetFixtureList(); f; f = f->GetNext()) {
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(&userData);
        }
    }
}

Body::~Body() {
    if (m_b2Body) {
        m_b2Body->GetWorld()->DestroyBody(m_b2Body);
    }
}
```

Apply the same shape to `StaticBody`, `KinematicBody`. Then audit `Collider::SetBody` (the place that stuffs `&body.userData` into the fixture) to confirm the address actually needs rewiring — if there's a stable indirection, no rewire needed.

Best long-term: make `Body` non-movable (`Body(Body&&) = delete;`) and store via `PointableComponent` so entt never moves it. Box2D bodies are conceptually pinned.

## Verify before fixing

```
grep -rn "Body\|StaticBody\|KinematicBody" SandCastle/src/Physics/ SandCastle/include/SandCastle/Physics/
grep -rn "Body\|StaticBody\|KinematicBody" madman/src/
```

If madman starts using Body before this fix, it will crash at high entity counts.

## Context for a fresh session

- `PointableComponent` is in `include/SandCastle/ECS/Components.h`. Tag a component with it to pin its storage (sets entt `in_place_delete = true`).
- Related but separate issue: `Collider::Polygon2D::GetAABB` has an initialization bug (see file 17).
- `PhysicsSystem::Update` is currently empty (no `b2World::Step` in the engine). Collisions don't tick yet; only AABB queries work against the static tree. So a Body added today is a fixture in the tree but never simulates — and the double-free still applies on destroy.
