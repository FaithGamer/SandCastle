#include "pch.h"
#include "SandCastle/Physics/AABBQueries.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	//
	//
	// Raycast Closest
	//
	//

	QueryRaycastCallbackClosest::QueryRaycastCallbackClosest(Vec2f start, Bitmask16 mask, RaycastResult* Result) 
		: m_start(start), m_mask(mask), result(Result)
	{

	}
	float QueryRaycastCallbackClosest::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		//Fixture's body is not contained in the collision mask
		uint16 layer = fixture->GetFilterData().categoryBits;
		if (!m_mask.Contains(layer))
			return 1.0f;

		//UserData contains entity id of fixture's body's entity
		auto data = static_cast<Collider::UserData*>((void*)(fixture->GetUserData().pointer));

		//Store raycast result
		result->entityId = data->entityId;
		result->point = point;
		result->normal = normal;
		result->distance = Math::Abs(Vec2f((Vec2f)point - m_start).Magnitude()) * fraction;
		result->hit = true;
		result->layer = layer;

		return fraction;
	}

	//
	//
	// Raycast all
	//
	//

	QueryRaycastCallbackAll::QueryRaycastCallbackAll(Vec2f start, Bitmask16 mask, std::vector<RaycastResult>* Results) 
		: m_start(start), m_mask(mask), results(Results)
	{

	}
	float QueryRaycastCallbackAll::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		//Fixture's body is not contained in the collision mask
		uint16 layer = fixture->GetFilterData().categoryBits;
		if (!m_mask.Contains(layer))
			return 1.0f;

		//UserData contains entity id of fixture's body's entity
		auto data = static_cast<Collider::UserData*>((void*)(fixture->GetUserData().pointer));

		//Store raycast results
		RaycastResult result(
			data->entityId,
			point,
			normal,
			Math::Abs(Vec2f((Vec2f)point - m_start).Magnitude()) * fraction,
			true,
			Bitmask16(layer)
		);

		results->emplace_back(result);

		return 1.0;
	}

	//
	//
	// B2Shape overlap
	//
	//

	QueryB2ShapeOverlapAll::QueryB2ShapeOverlapAll(b2Shape* shape, Bitmask16 mask, std::vector<OverlapResult>* Results)
		: m_mask(mask), m_shape(shape), results(Results)
	{
	}
	bool QueryB2ShapeOverlapAll::ReportFixture(b2Fixture* fixture)
	{
		//Layer mask
		uint16 layer = fixture->GetFilterData().categoryBits;
		if (!m_mask.Contains(layer))
			return true;

		b2Transform identity(b2Vec2(0, 0), b2Rot(0));
		auto transform = fixture->GetBody()->GetTransform();
		if (!b2TestOverlap(m_shape, 0, fixture->GetShape(), 0, identity, transform))
			return true; //No overlap

		//Compute distance between circle center and shape position
		float distance = Vec::Distance((Vec2f)transform.p, (Vec2f)static_cast<b2CircleShape*>(m_shape)->m_p);

		//User data in the fixture's body
		auto data = static_cast<Collider::UserData*>((void*)(fixture->GetUserData().pointer));

		results->emplace_back(OverlapResult(data->entityId, distance, Bitmask16(layer)));

		// Continue the query.
		return true;
	}

	//
	//
	// Body Overlap
	//
	//

	QueryBodyOverlapAll::QueryBodyOverlapAll(Body* body, Bitmask16 mask, std::vector<OverlapResult>* Results)
		: m_body(body), m_mask(mask), results(Results)
	{

	}

	bool QueryBodyOverlapAll::ReportFixture(b2Fixture* fixture)
	{
		//Layer mask
		uint16 layer = fixture->GetFilterData().categoryBits;
		if (!m_mask.Contains(layer))
			return true;

		auto colliders = m_body->GetColliders();
		bool collide = false;
		auto transform = fixture->GetBody()->GetTransform();
		for (int i = 0; i < colliders->size(); i++)
		{
			if ((*colliders)[i]->B2ShapeOverlap(fixture->GetShape(), transform))
			{
				collide = true;
			}
		}
		if (!collide)
			return true;
		//User data in the fixture's body
		auto data = static_cast<Collider::UserData*>((void*)(fixture->GetUserData().pointer));
		float distance = Vec::Distance((Vec2f)m_body->GetB2Body()->GetTransform().p, (Vec2f)transform.p);

		//to do add distance
		results->emplace_back(OverlapResult(data->entityId, distance, layer));

		// Continue the query.
		return true;
	}

	//
	//
	// Point Inside
	//
	//

	QueryPointInsideAll::QueryPointInsideAll(Vec2f point, Bitmask16 mask, std::vector<OverlapResult>* Results)
		: m_point(point), m_mask(mask), results(Results)
	{

	}
	bool QueryPointInsideAll::ReportFixture(b2Fixture* fixture)
	{
		//Layer mask
		uint16 layer = fixture->GetFilterData().categoryBits;
		if (!m_mask.Contains(layer))
			return true;
		//Is point inside fixture
		if (!fixture->TestPoint(m_point))
			return true;
		//User data in the fixture's body
		auto data = static_cast<Collider::UserData*>((void*)(fixture->GetUserData().pointer));

		//to do add distance
		results->emplace_back(OverlapResult(data->entityId));

		// Continue the query.
		return true;
	}

	//
	//
	// Body raycast
	//
	//
	QueryBodyRaycastAll::QueryBodyRaycastAll(Body* body, Bitmask16 mask, std::vector<OverlapResult>* Results)
	{
	}
	float QueryBodyRaycastAll::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		return 0.0f;
	}
}