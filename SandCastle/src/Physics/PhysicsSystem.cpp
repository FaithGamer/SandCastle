#include "pch.h"
#include "SandCastle/Physics/PhysicsSystem.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/Systems.h"

namespace SandCastle
{
	PhysicsSystem::PhysicsSystem()
	{
		SetPriority(99999);
	}
	void PhysicsSystem::Start()
	{
	}
	//Shortest-path angle interpolation, in degrees
	static float LerpAngle(float from, float to, float t)
	{
		float delta = to - from;
		while (delta > 180.f) delta -= 360.f;
		while (delta < -180.f) delta += 360.f;
		return from + delta * t;
	}

	void PhysicsSystem::Update()
	{
		//Kinematic bodies follow their entity's transform.
		//UpdateTransform skips bodies that didn't move.
		Entity::View<KinematicBody, Transform>().each([](KinematicBody& body, Transform& transform)
			{
				//One walk up the parent chain instead of one per value
				auto world = transform.GetWorld();
				body.UpdateTransform(world.position, world.rotation);
			});

		//Dynamic bodies drive their entity's transform. The simulation moves
		//on the fixed timestep only; render frames in between interpolate
		//between the last two fixed steps so motion stays smooth at any
		//framerate, at the cost of up to one fixed step of visual latency.
		float alpha = Systems::GetFixedUpdateAlpha();
		Entity::View<DynamicBody, Transform>().each([alpha](DynamicBody& body, Transform& transform)
			{
				//Resting/sleeping bodies have prev == curr: the lerp collapses to
				//the exact rest state, so writing unconditionally is both cheap
				//and guarantees the transform lands on the final position.
				Vec2f position = Vec::Lerp(body.m_prevPosition, body.m_currPosition, alpha);
				float rotation = LerpAngle(body.m_prevRotation, body.m_currRotation, alpha);

				//Box2D works in world space, so the result has to go back in as
				//world space. Writing it straight to the local slot would place a
				//parented body at its parent's offset twice over.
				Vec3f world = transform.GetPosition();
				if (body.m_YisZ)
				{
					transform.SetWorldPosition(position.x, world.y, position.y);
				}
				else
				{
					transform.SetWorldPosition(position.x, position.y, world.z);
				}
				transform.SetWorldRotation(rotation);
			});
	}
	void PhysicsSystem::FixedUpdate()
	{
		Physics::Step(Time::FixedDelta());

		//Snapshot the simulated state; Update() interpolates between the last two snapshots
		Entity::View<DynamicBody>().each([](DynamicBody& body)
			{
				body.m_prevPosition = body.m_currPosition;
				body.m_prevRotation = body.m_currRotation;
				//A sleeping body didn't move
				if (b2Body_IsAwake(body.GetB2Body()))
				{
					body.m_currPosition = body.GetPosition();
					body.m_currRotation = body.GetRotation();
					//Keep the teleport-skip cache honest in case UpdateTransform is ever called
					body.m_rotation = body.m_currRotation;
				}
			});
	}
	int PhysicsSystem::GetUsedMethod()
	{
		return System::Method::Updt | System::Method::FixedUpdt;
	}

}
