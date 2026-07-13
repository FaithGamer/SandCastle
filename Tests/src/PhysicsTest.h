#pragma once

#include <SandCastle.h>
#include <box2d/box2d.h>
#include <vector>

// Physics module test suite (Box2D v3 wrapper).
// Runs headless: no Engine::Init, no window. Each section is scoped so its
// bodies are destroyed before the next one starts.

namespace PhysicsTests
{
	static int passed = 0;
	static int failed = 0;

	inline void Check(bool condition, const char* name)
	{
		if (condition)
		{
			passed++;
			LOG_INFO("PASS: {0}", name);
		}
		else
		{
			failed++;
			LOG_ERROR("FAIL: {0}", name);
		}
	}

	inline bool Approx(float value, float expected, float epsilon = 0.05f)
	{
		return value > expected - epsilon && value < expected + epsilon;
	}

	inline bool Approx(SandCastle::Vec2f value, SandCastle::Vec2f expected, float epsilon = 0.05f)
	{
		return Approx(value.x, expected.x, epsilon) && Approx(value.y, expected.y, epsilon);
	}
}

inline void PhysicsTest()
{
	using namespace SandCastle;
	using namespace PhysicsTests;

	Log::Instance()->Init();
	LOG_INFO("--Physics test suite--");

	//World
	{
		Check(b2World_IsValid(Physics::GetB2World()), "world is valid");
		Check(Physics::GetBodyCount() == 0, "world starts empty");
	}

	//Body lifetime
	{
		{
			StaticBody body(Vec2f(10, 5));
			Check(Physics::GetBodyCount() == 1, "body count after create");
		}
		Check(Physics::GetBodyCount() == 0, "body destroyed with scope");
	}

	//Collider AABB
	{
		StaticBody body(Vec2f(10, 5));
		body.AddCollider(makesptr<Box2D>(2.f, 2.f));
		b2AABB aabb = body.GetAABB();
		Check(Approx(aabb.lowerBound.x, 9) && Approx(aabb.lowerBound.y, 4)
			&& Approx(aabb.upperBound.x, 11) && Approx(aabb.upperBound.y, 6), "box collider AABB");

		body.AddCollider(makesptr<Circle2D>(3.f));
		aabb = body.GetAABB();
		Check(Approx(aabb.lowerBound.x, 7) && Approx(aabb.upperBound.y, 8), "multi-collider AABB is the union");
	}

	//Raycasts
	{
		Entity entityNear = Entity::Create();
		Entity entityFar = Entity::Create();

		StaticBody bodyNear(Vec2f(10, 5));
		bodyNear.userData.entityId = entityNear.GetId();
		bodyNear.AddCollider(makesptr<Box2D>(2.f, 2.f));

		StaticBody bodyFar(Vec2f(15, 5));
		bodyFar.userData.entityId = entityFar.GetId();
		bodyFar.AddCollider(makesptr<Box2D>(2.f, 2.f));

		RaycastResult hit;
		Physics::RaycastClosest(hit, Vec2f(0, 5), Vec2f(20, 5));
		Check(hit.hit, "raycast hits");
		Check(hit.entityId == entityNear.GetId(), "raycast reports the closest body");
		Check(Approx(hit.point, Vec2f(9, 5)), "raycast hit point");
		Check(Approx(hit.distance, 9.f), "raycast distance");
		Check(Approx(hit.normal, Vec2f(-1, 0)), "raycast normal");

		RaycastResult miss;
		Physics::RaycastClosest(miss, Vec2f(0, 20), Vec2f(20, 20));
		Check(!miss.hit && !miss, "raycast misses");

		std::vector<RaycastResult> all;
		Physics::RaycastAll(all, Vec2f(0, 5), Vec2f(20, 5));
		Check(all.size() == 2, "raycast all finds both bodies");

		entityNear.Destroy();
		entityFar.Destroy();
	}

	//Layer masks, including changing the layer after colliders exist
	{
		StaticBody body(Vec2f(10, 5)); //default layer 1
		body.AddCollider(makesptr<Box2D>(2.f, 2.f));

		RaycastResult hitLayer1;
		Physics::RaycastClosest(hitLayer1, Vec2f(0, 5), Vec2f(20, 5), Bitmask16(1));
		Check(hitLayer1.hit && hitLayer1.layer.flags == 1, "mask 1 hits layer 1 body");

		RaycastResult missLayer2;
		Physics::RaycastClosest(missLayer2, Vec2f(0, 5), Vec2f(20, 5), Bitmask16(2));
		Check(!missLayer2.hit, "mask 2 ignores layer 1 body");

		body.SetLayer(Bitmask16(2));

		RaycastResult hitLayer2;
		Physics::RaycastClosest(hitLayer2, Vec2f(0, 5), Vec2f(20, 5), Bitmask16(2));
		Check(hitLayer2.hit && hitLayer2.layer.flags == 2, "SetLayer after AddCollider updates live shapes");

		RaycastResult missLayer1;
		Physics::RaycastClosest(missLayer1, Vec2f(0, 5), Vec2f(20, 5), Bitmask16(1));
		Check(!missLayer1.hit, "old layer no longer matches after SetLayer");
	}

	//Named layers
	{
		Physics::AddLayer("terrain");
		Physics::AddLayer("actors");

		StaticBody body(Vec2f(10, 5), Physics::GetLayerMask("actors"));
		body.AddCollider(makesptr<Circle2D>(1.f));

		RaycastResult hit;
		Physics::RaycastClosest(hit, Vec2f(0, 5), Vec2f(20, 5), Physics::GetLayerMask("actors"));
		Check(hit.hit, "named layer mask hits");

		RaycastResult miss;
		Physics::RaycastClosest(miss, Vec2f(0, 5), Vec2f(20, 5), Physics::GetLayerMask("terrain"));
		Check(!miss.hit, "other named layer mask misses");
	}

	//Circle overlap
	{
		Entity entity = Entity::Create();
		StaticBody body(Vec2f(10, 5));
		body.userData.entityId = entity.GetId();
		body.AddCollider(makesptr<Box2D>(2.f, 2.f));

		std::vector<OverlapResult> results;
		Physics::CircleOverlap(results, Vec2f(8, 5), 2.f);
		Check(results.size() == 1 && results[0].entityId == entity.GetId(), "circle overlap finds body");
		Check(Approx(results[0].distance, 2.f), "circle overlap distance to body origin");

		results.clear();
		Physics::CircleOverlap(results, Vec2f(0, 0), 1.f);
		Check(results.empty(), "circle overlap far away finds nothing");

		entity.Destroy();
	}

	//Point inside
	{
		Entity entity = Entity::Create();
		StaticBody body(Vec2f(10, 5));
		body.userData.entityId = entity.GetId();
		body.AddCollider(makesptr<Box2D>(2.f, 2.f));

		std::vector<OverlapResult> results;
		Physics::PointInside(results, Vec2f(10.5f, 5.5f));
		Check(results.size() == 1 && results[0].entityId == entity.GetId(), "point inside box");
		Check(!results.empty() && results[0].layer.flags == 1, "point inside reports the layer");

		results.clear();
		Physics::PointInside(results, Vec2f(12, 5));
		Check(results.empty(), "point outside box");

		entity.Destroy();
	}

	//Body overlap
	{
		Entity entityA = Entity::Create();
		Entity entityB = Entity::Create();
		Entity entityC = Entity::Create();

		StaticBody bodyA(Vec2f(0, 0));
		bodyA.userData.entityId = entityA.GetId();
		bodyA.AddCollider(makesptr<Box2D>(2.f, 2.f));

		StaticBody bodyB(Vec2f(1, 0)); //overlaps A
		bodyB.userData.entityId = entityB.GetId();
		bodyB.AddCollider(makesptr<Box2D>(2.f, 2.f));

		StaticBody bodyC(Vec2f(10, 0)); //far away
		bodyC.userData.entityId = entityC.GetId();
		bodyC.AddCollider(makesptr<Box2D>(2.f, 2.f));

		std::vector<OverlapResult> results;
		bodyA.OverlappingBodies(results);
		Check(results.size() == 1 && results[0].entityId == entityB.GetId(),
			"body overlap finds only the overlapping other body");

		entityA.Destroy();
		entityB.Destroy();
		entityC.Destroy();
	}

	//Collider direct queries
	{
		StaticBody bodyA(Vec2f(0, 0));
		auto boxCollider = makesptr<Box2D>(2.f, 2.f);
		bodyA.AddCollider(boxCollider);

		Check(boxCollider->PointInside(Vec2f(0.5f, 0.5f)), "collider point inside");
		Check(!boxCollider->PointInside(Vec2f(3, 0)), "collider point outside");
		Check(boxCollider->CircleOverlap(Vec2f(2, 0), 1.5f), "collider circle overlap");
		Check(!boxCollider->CircleOverlap(Vec2f(5, 0), 1.f), "collider circle no overlap");
		Check(boxCollider->GetBody() == &bodyA, "collider knows its body");

		StaticBody bodyB(Vec2f(1.5f, 0));
		auto circleCollider = makesptr<Circle2D>(1.f);
		bodyB.AddCollider(circleCollider);
		Check(boxCollider->ColliderOverlap(circleCollider.get()), "collider vs collider overlap");

		StaticBody bodyC(Vec2f(10, 0));
		auto farCollider = makesptr<Circle2D>(1.f);
		bodyC.AddCollider(farCollider);
		Check(!boxCollider->ColliderOverlap(farCollider.get()), "collider vs collider no overlap");
	}

	//Concave polygon collider (earcut triangulation)
	{
		StaticBody body(Vec2f(20, 20));
		auto polygon = makesptr<Polygon2D>();
		//L-shape, counter clockwise: [0,2]x[0,1] + [0,1]x[1,2]
		polygon->AddPoint(Vec2f(0, 0));
		polygon->AddPoint(Vec2f(2, 0));
		polygon->AddPoint(Vec2f(2, 1));
		polygon->AddPoint(Vec2f(1, 1));
		polygon->AddPoint(Vec2f(1, 2));
		polygon->AddPoint(Vec2f(0, 2));
		body.AddCollider(polygon);

		std::vector<OverlapResult> results;
		Physics::PointInside(results, Vec2f(20.5f, 20.5f));
		Check(results.size() == 1, "point in L-shape horizontal arm");

		results.clear();
		Physics::PointInside(results, Vec2f(20.5f, 21.5f));
		Check(results.size() == 1, "point in L-shape vertical arm");

		results.clear();
		Physics::PointInside(results, Vec2f(21.5f, 21.5f));
		Check(results.empty(), "point in L-shape notch is outside");
	}

	//Kinematic transform updates
	{
		KinematicBody body; //at origin
		body.AddCollider(makesptr<Box2D>(4.f, 1.f)); //x in [-2,2], y in [-0.5,0.5]

		std::vector<OverlapResult> results;
		Physics::PointInside(results, Vec2f(0, 1.5f));
		Check(results.empty(), "point above unrotated box is outside");

		body.UpdateTransform(Vec3f(0, 0, 0), 90.f); //box becomes vertical
		Physics::PointInside(results, Vec2f(0, 1.5f));
		Check(results.size() == 1, "rotation rotates the collision shape");

		results.clear();
		body.UpdateTransform(Vec3f(30, 5, 0), 0.f);
		Physics::PointInside(results, Vec2f(30, 5));
		Check(results.size() == 1, "body moves with UpdateTransform");

		results.clear();
		body.UpdateTransform(Vec3f(30, 5, 0), 0.f); //same values: skip path
		Physics::PointInside(results, Vec2f(30, 5));
		Check(results.size() == 1, "redundant UpdateTransform is harmless");

		results.clear();
		body.SetYisZ(true);
		body.UpdateTransform(Vec3f(40, 999.f, 7.f), 0.f);
		Physics::PointInside(results, Vec2f(40, 7));
		Check(results.size() == 1, "YisZ uses z as the vertical axis");
	}

	//Move semantics
	{
		Entity entity = Entity::Create();
		{
			StaticBody bodyA(Vec2f(50, 50));
			bodyA.userData.entityId = entity.GetId();
			bodyA.AddCollider(makesptr<Box2D>(2.f, 2.f));

			StaticBody bodyB(std::move(bodyA));
			Check(Physics::GetBodyCount() == 1, "move keeps a single Box2D body");

			std::vector<OverlapResult> results;
			Physics::PointInside(results, Vec2f(50, 50));
			Check(results.size() == 1 && results[0].entityId == entity.GetId(),
				"moved body still queryable with same entity");

			StaticBody bodyC(Vec2f(60, 60));
			bodyC = std::move(bodyB);
			Check(Physics::GetBodyCount() == 1, "move assignment frees the overwritten body");

			results.clear();
			Physics::PointInside(results, Vec2f(50, 50));
			Check(results.size() == 1 && results[0].entityId == entity.GetId(),
				"move-assigned body still queryable");
		}
		Check(Physics::GetBodyCount() == 0, "no double destroy after moves");
		entity.Destroy();
	}

	//ClearCollider
	{
		StaticBody body(Vec2f(70, 0));
		body.AddCollider(makesptr<Box2D>(2.f, 2.f));

		RaycastResult beforeClear;
		Physics::RaycastClosest(beforeClear, Vec2f(60, 0), Vec2f(80, 0));
		Check(beforeClear.hit, "collider hit before clear");

		body.ClearCollider();
		RaycastResult afterClear;
		Physics::RaycastClosest(afterClear, Vec2f(60, 0), Vec2f(80, 0));
		Check(!afterClear.hit, "no hit after ClearCollider");

		body.AddCollider(makesptr<Circle2D>(1.f));
		RaycastResult reAdded;
		Physics::RaycastClosest(reAdded, Vec2f(60, 0), Vec2f(80, 0));
		Check(reAdded.hit && Approx(reAdded.distance, 9.f), "collider re-added after clear");
	}

	//Bodies as ECS components: stable addresses, destruction
	{
		Entity entity = Entity::Create();
		StaticBody* body = entity.AddGet<StaticBody>(Vec2f(80.f, 0.f));
		body->userData.entityId = entity.GetId();
		body->AddCollider(makesptr<Box2D>(2.f, 2.f));

		Collider::UserData* stableAddress = &body->userData;
		std::vector<Entity> dummies;
		for (int i = 0; i < 200; i++)
		{
			Entity dummy = Entity::Create();
			dummy.Add<StaticBody>(Vec2f(200.f + i, 200.f));
			dummies.emplace_back(dummy);
		}
		Check(&entity.Get<StaticBody>()->userData == stableAddress,
			"component address stable after 200 insertions");

		std::vector<OverlapResult> results;
		Physics::PointInside(results, Vec2f(80, 0));
		Check(results.size() == 1 && results[0].entityId == entity.GetId(), "ECS-stored body queryable");

		for (auto& dummy : dummies)
		{
			dummy.Destroy();
		}
		results.clear();
		Physics::PointInside(results, Vec2f(80, 0));
		Check(results.size() == 1, "body survives sibling component removal");

		entity.Destroy();
		Check(Physics::GetBodyCount() == 0, "ECS destroy frees Box2D bodies");
	}

	//Dynamic simulation (headless: the world is stepped manually)
	{
		Physics::SetGravity(Vec2f(0, -10));
		Check(Approx(Physics::GetGravity(), Vec2f(0, -10)), "gravity set/get");

		StaticBody ground(Vec2f(0, 0));
		ground.AddCollider(makesptr<Box2D>(20.f, 1.f)); //top at y = 0.5

		DynamicBody faller(Vec2f(0, 5));
		faller.AddCollider(makesptr<Box2D>(1.f, 1.f));
		Check(Approx(faller.GetPosition(), Vec2f(0, 5)), "dynamic body starts where created");

		for (int i = 0; i < 30; i++) Physics::Step(1.f / 60.f);
		Check(faller.GetPosition().y < 4.9f, "gravity pulls the body down");

		for (int i = 0; i < 300; i++) Physics::Step(1.f / 60.f);
		Check(Approx(faller.GetPosition().y, 1.f, 0.1f), "body comes to rest on the ground");
		Check(Approx(faller.GetVelocity(), Vec2f(0, 0), 0.1f), "resting body has no velocity");

		//Velocity and impulses, isolated from gravity
		DynamicBody mover(Vec2f(50, 50));
		mover.AddCollider(makesptr<Box2D>(1.f, 1.f)); //density 1: mass 1
		mover.SetGravityScale(0.f);
		Check(Approx(mover.GetMass(), 1.f), "mass computed from collider (density 1)");

		mover.SetVelocity(Vec2f(2, 0));
		Check(Approx(mover.GetVelocity(), Vec2f(2, 0)), "velocity set/get");
		for (int i = 0; i < 60; i++) Physics::Step(1.f / 60.f);
		Check(Approx(mover.GetPosition(), Vec2f(52, 50), 0.15f), "velocity moves the body");
		Check(Approx(mover.GetPosition().y, 50.f, 0.05f), "gravity scale 0 cancels gravity");

		mover.SetVelocity(Vec2f(0, 0));
		mover.ApplyImpulse(Vec2f(0, 3));
		Physics::Step(1.f / 60.f);
		Check(Approx(mover.GetVelocity(), Vec2f(0, 3), 0.1f), "impulse changes velocity by impulse/mass");

		//Rotation
		DynamicBody spinner(Vec2f(80, 80));
		spinner.AddCollider(makesptr<Box2D>(1.f, 1.f));
		spinner.SetGravityScale(0.f);
		spinner.SetAngularVelocity(90.f);
		Check(Approx(spinner.GetAngularVelocity(), 90.f, 0.5f), "angular velocity set/get (degrees)");
		for (int i = 0; i < 30; i++) Physics::Step(1.f / 60.f);
		Check(Approx(spinner.GetRotation(), 45.f, 2.f), "angular velocity rotates the body (CCW positive)");

		DynamicBody locked(Vec2f(90, 90));
		locked.AddCollider(makesptr<Box2D>(1.f, 1.f));
		locked.SetGravityScale(0.f);
		locked.SetFixedRotation(true);
		locked.SetAngularVelocity(90.f);
		for (int i = 0; i < 30; i++) Physics::Step(1.f / 60.f);
		Check(Approx(locked.GetRotation(), 0.f), "fixed rotation never rotates");
	}

	if (failed == 0)
	{
		LOG_INFO("--Physics tests: {0} passed, 0 failed--", passed);
	}
	else
	{
		LOG_ERROR("--Physics tests: {0} passed, {1} FAILED--", passed, failed);
	}
}
