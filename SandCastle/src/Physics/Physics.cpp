#include "pch.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Physics/AABBQueries.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Physics/ColliderRenderDebugSystem.h"

namespace SandCastle
{
	Physics::Physics()
	{
		m_world = new b2World({ 0, -1 });

	}
	Physics::~Physics()
	{
		delete m_world;
	}
	void Physics::RaycastClosest(RaycastResult& result, Vec2f start, Vec2f end, Bitmask16 mask)
	{
		auto ins = Instance();
		QueryRaycastCallbackClosest query(start, mask, &result);
		ins->m_world->RayCast(&query, start, end);
	}
	void Physics::RaycastAll(std::vector<RaycastResult>& results, Vec2f start, Vec2f end, Bitmask16 mask)
	{
		auto ins = Instance();
		QueryRaycastCallbackAll query(start, mask, &results);
		ins->m_world->RayCast(&query, start, end);
	}
	void Physics::BodyOverlap(std::vector<OverlapResult>& results, Body* body, Bitmask16 mask)
	{
		//to do TEST
		auto ins = Instance();

		b2AABB aabb = body->GetAABB();

		QueryBodyOverlapAll query(body, mask, &results);

		ins->m_world->QueryAABB(&query, aabb);
	}
	int Physics::GetBodyCount()
	{
		auto ins = Instance();
		return ins->m_world->GetBodyCount();

	}
	void Physics::CircleOverlap(std::vector<OverlapResult>& results, Vec2f pos, float radius, Bitmask16 mask)
	{
		auto ins = Instance();
			
		// Make a box englobing the circle
		b2AABB aabb;
		aabb.lowerBound = pos - radius;
		aabb.upperBound = pos + radius;

		//Create the query
		b2CircleShape shape;
		shape.m_p = pos;
		shape.m_radius = radius;
		QueryB2ShapeOverlapAll query(&shape, mask, &results);

		//Query the world
		ins->m_world->QueryAABB(&query, aabb);

		//Results are now stored 
	}

	void Physics::PointInside(std::vector<OverlapResult>& results, Vec2f pos, Bitmask16 mask)
	{
		auto ins = Instance();

		b2AABB aabb;
		aabb.lowerBound = pos - 0.01f;
		aabb.upperBound = pos + 0.01f;

		QueryPointInsideAll query(pos, mask, &results);

		ins->m_world->QueryAABB(&query, aabb);

	}
	void Physics::DrawColliders(bool draw)
	{
		if (draw)
		{
			Systems::Push<ColliderRenderDebugSystem>();
		}
		else
		{
			Systems::Remove<ColliderRenderDebugSystem>();
		}
	}

	b2World* Physics::GetB2World()
	{
		return Instance()->m_world;
	}
}	