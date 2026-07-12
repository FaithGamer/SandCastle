#include "pch.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Physics/ColliderRenderDebugSystem.h"

namespace SandCastle
{
	//The engine does its own layer filtering in the callbacks (Bitmask16::Contains
	//semantics: every layer bit of the shape must be in the query mask), so the
	//Box2D-side query filter is wide open.
	static b2QueryFilter QueryAll()
	{
		return b2QueryFilter{ UINT64_MAX, UINT64_MAX };
	}

	//Shapes created through Body/Collider carry the Body's UserData on their Box2D body.
	static Collider::UserData* GetShapeUserData(b2ShapeId shapeId)
	{
		return static_cast<Collider::UserData*>(b2Body_GetUserData(b2Shape_GetBody(shapeId)));
	}

	static uint16_t GetShapeLayer(b2ShapeId shapeId)
	{
		return (uint16_t)b2Shape_GetFilter(shapeId).categoryBits;
	}

	Physics::Physics()
	{
		b2WorldDef def = b2DefaultWorldDef();
		def.gravity = b2Vec2{ 0, -1 };
		m_world = b2CreateWorld(&def);
	}
	Physics::~Physics()
	{
		b2DestroyWorld(m_world);
	}

	//
	//
	// Raycasts
	//
	//

	struct RaycastContext
	{
		RaycastResult* result;
		std::vector<RaycastResult>* results;
		Vec2f start;
		Bitmask16 mask;
	};

	static RaycastResult MakeRaycastResult(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, Vec2f start, uint16_t layer)
	{
		RaycastResult result;
		Collider::UserData* data = GetShapeUserData(shapeId);
		if (data != nullptr)
		{
			result.entityId = data->entityId;
		}
		result.point = point;
		result.normal = normal;
		result.distance = Vec::Distance((Vec2f)point, start);
		result.hit = true;
		result.layer = layer;
		return result;
	}

	static float RaycastClosestCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
	{
		RaycastContext* ctx = static_cast<RaycastContext*>(context);

		//Shape is not contained in the collision mask: skip it and keep the ray at full length
		uint16_t layer = GetShapeLayer(shapeId);
		if (!ctx->mask.Contains(layer))
			return -1.f;

		//Store the hit and clip the ray, so only closer shapes can override the result
		*ctx->result = MakeRaycastResult(shapeId, point, normal, ctx->start, layer);
		return fraction;
	}

	static float RaycastAllCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
	{
		RaycastContext* ctx = static_cast<RaycastContext*>(context);

		uint16_t layer = GetShapeLayer(shapeId);
		if (!ctx->mask.Contains(layer))
			return -1.f;

		ctx->results->emplace_back(MakeRaycastResult(shapeId, point, normal, ctx->start, layer));
		return 1.f;
	}

	void Physics::RaycastClosest(RaycastResult& result, Vec2f start, Vec2f end, Bitmask16 mask)
	{
		RaycastContext ctx{ &result, nullptr, start, mask };
		b2Vec2 translation{ end.x - start.x, end.y - start.y };
		b2World_CastRay(Instance()->m_world, start, translation, QueryAll(), RaycastClosestCallback, &ctx);
	}

	void Physics::RaycastAll(std::vector<RaycastResult>& results, Vec2f start, Vec2f end, Bitmask16 mask)
	{
		RaycastContext ctx{ nullptr, &results, start, mask };
		b2Vec2 translation{ end.x - start.x, end.y - start.y };
		b2World_CastRay(Instance()->m_world, start, translation, QueryAll(), RaycastAllCallback, &ctx);
	}

	//
	//
	// Circle overlap
	//
	//

	struct CircleOverlapContext
	{
		std::vector<OverlapResult>* results;
		Vec2f pos;
		Bitmask16 mask;
	};

	static bool CircleOverlapCallback(b2ShapeId shapeId, void* context)
	{
		CircleOverlapContext* ctx = static_cast<CircleOverlapContext*>(context);

		uint16_t layer = GetShapeLayer(shapeId);
		if (!ctx->mask.Contains(layer))
			return true;

		Collider::UserData* data = GetShapeUserData(shapeId);
		if (data == nullptr)
			return true;

		//Distance between the circle center and the shape's body origin
		Vec2f bodyPos = b2Body_GetPosition(b2Shape_GetBody(shapeId));
		float distance = Vec::Distance(bodyPos, ctx->pos);

		ctx->results->emplace_back(OverlapResult(data->entityId, distance, Bitmask16(layer)));

		// Continue the query.
		return true;
	}

	void Physics::CircleOverlap(std::vector<OverlapResult>& results, Vec2f pos, float radius, Bitmask16 mask)
	{
		CircleOverlapContext ctx{ &results, pos, mask };
		b2Vec2 center = pos;
		b2ShapeProxy proxy = b2MakeProxy(&center, 1, radius);
		b2World_OverlapShape(Instance()->m_world, &proxy, QueryAll(), CircleOverlapCallback, &ctx);
	}

	//
	//
	// Point inside
	//
	//

	struct PointInsideContext
	{
		std::vector<OverlapResult>* results;
		Vec2f point;
		Bitmask16 mask;
	};

	static bool PointInsideCallback(b2ShapeId shapeId, void* context)
	{
		PointInsideContext* ctx = static_cast<PointInsideContext*>(context);

		uint16_t layer = GetShapeLayer(shapeId);
		if (!ctx->mask.Contains(layer))
			return true;

		//The broad phase only matched AABBs, test the actual shape
		if (!b2Shape_TestPoint(shapeId, ctx->point))
			return true;

		Collider::UserData* data = GetShapeUserData(shapeId);
		if (data == nullptr)
			return true;

		ctx->results->emplace_back(OverlapResult(data->entityId, 0, Bitmask16(layer)));

		// Continue the query.
		return true;
	}

	void Physics::PointInside(std::vector<OverlapResult>& results, Vec2f pos, Bitmask16 mask)
	{
		PointInsideContext ctx{ &results, pos, mask };
		b2AABB aabb;
		aabb.lowerBound = pos - 0.01f;
		aabb.upperBound = pos + 0.01f;
		b2World_OverlapAABB(Instance()->m_world, aabb, QueryAll(), PointInsideCallback, &ctx);
	}

	//
	//
	// Body overlap
	//
	//

	struct BodyOverlapContext
	{
		std::vector<OverlapResult>* results;
		Body* body;
		Bitmask16 mask;
	};

	static bool BodyOverlapCallback(b2ShapeId shapeId, void* context)
	{
		BodyOverlapContext* ctx = static_cast<BodyOverlapContext*>(context);

		uint16_t layer = GetShapeLayer(shapeId);
		if (!ctx->mask.Contains(layer))
			return true;

		//Don't report the queried body itself
		b2BodyId candidate = b2Shape_GetBody(shapeId);
		b2BodyId self = ctx->body->GetB2Body();
		if (B2_ID_EQUALS(candidate, self))
			return true;

		//The broad phase only matched AABBs, test the candidate shape against the body's colliders
		b2ShapeProxy proxy = GetShapeProxy(shapeId);
		b2Transform transform = b2Body_GetTransform(candidate);
		bool collide = false;
		const std::vector<sptr<Collider>>* colliders = ctx->body->GetColliders();
		for (auto& collider : *colliders)
		{
			if (collider->ProxyOverlap(proxy, transform))
			{
				collide = true;
				break;
			}
		}
		if (!collide)
			return true;

		Collider::UserData* data = GetShapeUserData(shapeId);
		if (data == nullptr)
			return true;

		float distance = Vec::Distance((Vec2f)b2Body_GetPosition(self), (Vec2f)transform.p);
		ctx->results->emplace_back(OverlapResult(data->entityId, distance, Bitmask16(layer)));

		// Continue the query.
		return true;
	}

	void Physics::BodyOverlap(std::vector<OverlapResult>& results, Body* body, Bitmask16 mask)
	{
		BodyOverlapContext ctx{ &results, body, mask };
		b2World_OverlapAABB(Instance()->m_world, body->GetAABB(), QueryAll(), BodyOverlapCallback, &ctx);
	}

	//
	//
	// Misc
	//
	//

	int Physics::GetBodyCount()
	{
		return b2World_GetCounters(Instance()->m_world).bodyCount;
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

	b2WorldId Physics::GetB2World()
	{
		return Instance()->m_world;
	}
}
