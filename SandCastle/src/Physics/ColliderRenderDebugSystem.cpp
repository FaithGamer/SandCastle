#include "pch.h"
#include "SandCastle/Physics/ColliderRenderDebugSystem.h"
#include "SandCastle/Physics/Collider.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Physics/Body.h"

namespace SandCastle
{

	ColliderRenderDebugSystem::ColliderRenderDebugSystem() : m_debugLayer(0), m_updateQueueAuto(true)
	{
		SetPriority(-9999);
	}

	void AddColliderRender(Entity bodyEntt, Body* body)
	{
		auto colliders = body->GetColliders();
		for (size_t i = 0; i < colliders->size(); i++)
		{
			Entity wireEntt = Entity::Create();
			wireEntt.AddGet<ColliderRender>(&*(*colliders)[i]);
			wireEntt.Add<Transform>();
			bodyEntt.AddChild(wireEntt);
		}
	}

	void ColliderRenderDebugSystem::Start()
	{
		m_debugLayer = Renderer2D::GetLayerId("DebugLayer");
	}

	void ColliderRenderDebugSystem::Update()
	{
		if (!m_updateQueueAuto)
			return;
		UpdateQueue();
	}

	void ColliderRenderDebugSystem::UpdateQueue()
	{
		//Collider render entity creation is currently disabled, pending the
		//wire rendering path being reactivated (WireRenderSystem).
	}

	void ColliderRenderDebugSystem::UpdateQueueAuto(bool updateQueueAuto)
	{
		m_updateQueueAuto = updateQueueAuto;
	}
}
