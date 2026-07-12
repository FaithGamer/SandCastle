#include "pch.h"
#include "SandCastle/Physics/PhysicsSystem.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	PhysicsSystem::PhysicsSystem()
	{
		SetPriority(99999);
	}
	void PhysicsSystem::Start()
	{
	}
	void PhysicsSystem::Update()
	{
		//Kinematic bodies follow their entity's transform.
		//UpdateTransform skips bodies that didn't move.
		Entity::View<KinematicBody, Transform>().each([](KinematicBody& body, Transform& transform)
			{
				body.UpdateTransform(transform.GetPosition(), transform.GetRotation());
			});
	}
	int PhysicsSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

}
