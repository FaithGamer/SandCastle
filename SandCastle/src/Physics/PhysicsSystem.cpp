#include "pch.h"
#include "SandCastle/Physics/PhysicsSystem.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	PhysicsSystem::PhysicsSystem()
	{
		SetPriority(99999);
		//ListenAddGet<KinematicBody>(&PhysicsSystem::OnAddKinematicBody);
		//ListenAddGet<StaticBody>(&PhysicsSystem::OnAddStaticBody);
	}
	void PhysicsSystem::Start()
	{
	}
	/*void PhysicsSystem::OnAddKinematicBody(ComponentSignal signal)
	{
		//Bind the body with it's entity id
		Entity(signal.entity).Get<KinematicBody>()->userData = Collider::UserData(signal.entity);
	}
	void PhysicsSystem::OnAddStaticBody(ComponentSignal signal)
	{
		//Bind the body with it's entity id
		Entity entity(signal.entity);
		auto body = entity.Get<StaticBody>();
		body->userData = Collider::UserData(signal.entity);

	}*/
	void PhysicsSystem::Update()
	{
		/*ForeachComponents<KinematicBody, Transform>([](KinematicBody& body, Transform& transform)
			{
				body.UpdateTransform(transform.GetPosition(), transform.GetRotation().z);
			});*/
	}
	int PhysicsSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

}