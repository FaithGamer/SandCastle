#include "pch.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	Body::Body(Bitmask16 layer) :
		m_layer(layer),
		m_mask(65535),
		m_bodyId(b2_nullBodyId),
		m_rotation(0),
		m_YisZ(false)
	{

	}
	Body::Body(Body&& body) noexcept
	{
		MoveFrom(std::move(body));
	}

	Body& Body::operator=(Body&& body) noexcept
	{
		if (this != &body)
		{
			DetachColliders();
			if (B2_IS_NON_NULL(m_bodyId) && b2Body_IsValid(m_bodyId))
			{
				b2DestroyBody(m_bodyId);
			}
			MoveFrom(std::move(body));
		}
		return *this;
	}

	Body::~Body()
	{
		DetachColliders();
		//Free the body in the Box2D world. The id is null if this instance
		//has been moved from, and stale if the world has already been destroyed.
		if (B2_IS_NON_NULL(m_bodyId) && b2Body_IsValid(m_bodyId))
		{
			b2DestroyBody(m_bodyId);
		}
	}

	void Body::CreateB2Body(b2BodyType type, Vec2f position, Bitmask16 layer)
	{
		m_layer = layer;
		b2BodyDef def = b2DefaultBodyDef();
		def.type = type;
		def.position = position;
		def.userData = &userData;
		m_bodyId = b2CreateBody(Physics::GetB2World(), &def);
	}

	void Body::DetachColliders()
	{
		for (auto& collider : m_colliders)
		{
			collider->m_shapes.clear();
			collider->m_body = nullptr;
		}
		m_colliders.clear();
	}

	void Body::MoveFrom(Body&& body) noexcept
	{
		m_layer = body.m_layer;
		m_mask = body.m_mask;
		m_YisZ = body.m_YisZ;
		m_rotation = body.m_rotation;
		m_bodyId = body.m_bodyId;
		m_colliders = std::move(body.m_colliders);
		userData = body.userData;
		body.m_bodyId = b2_nullBodyId;

		//The Box2D body and the colliders still point to the old instance
		if (B2_IS_NON_NULL(m_bodyId))
		{
			b2Body_SetUserData(m_bodyId, &userData);
		}
		for (auto& collider : m_colliders)
		{
			collider->m_body = this;
		}
	}

	void Body::SetLayer(Bitmask16 layer)
	{
		m_layer = layer;
		for (auto& collider : m_colliders)
		{
			collider->SetFilter(GetB2Filter());
		}
	}

	void Body::SetLayerMask(Bitmask16 mask)
	{
		m_mask = mask;
		for (auto& collider : m_colliders)
		{
			collider->SetFilter(GetB2Filter());
		}
	}

	void Body::AddCollider(sptr<Collider> collider)
	{
		if (B2_IS_NULL(m_bodyId))
		{
			LOG_ERROR("Cannot add a collider to a Body with no underlying Box2D body. Use StaticBody or KinematicBody.");
			return;
		}
		//Attach collider and body
		collider->SetBody(this, GetB2Filter());
		m_colliders.emplace_back(collider);
	}

	void Body::ClearCollider()
	{
		for (auto& collider : m_colliders)
		{
			collider->DestroyShapes();
		}
		m_colliders.clear();
		//Shape destruction defers the mass update, recompute once at the end
		if (B2_IS_NON_NULL(m_bodyId) && b2Body_IsValid(m_bodyId))
		{
			b2Body_ApplyMassFromShapes(m_bodyId);
		}
	}

	void Body::OverlappingBodies(std::vector<OverlapResult>& results)
	{
		Physics::BodyOverlap(results, this, m_mask);
	}

	void Body::UpdateTransform(Vec3f position, float rotation)
	{
		b2Vec2 pos{ position.x, position.y };
		if (m_YisZ)
		{
			pos.y = position.z;
		}
		//Teleporting is expensive, skip when the body didn't move
		b2Vec2 current = b2Body_GetPosition(m_bodyId);
		if (current.x == pos.x && current.y == pos.y && m_rotation == rotation)
			return;

		m_rotation = rotation;
		//Engine rotations are counter-clockwise positive (renderer and
		//Math::AngleToVec agree on this), exactly like Box2D
		b2Body_SetTransform(m_bodyId, pos, b2MakeRot(Math::Radians(rotation)));
	}

	void Body::SetYisZ(bool yIsZ)
	{
		m_YisZ = yIsZ;
	}

	Bitmask16 Body::GetLayer() const
	{
		return m_layer;
	}

	Bitmask16 Body::GetLayerMask() const
	{
		return m_mask;
	}

	b2BodyId Body::GetB2Body() const
	{
		return m_bodyId;
	}

	b2Filter Body::GetB2Filter() const
	{
		b2Filter filter = b2DefaultFilter();
		filter.categoryBits = m_layer.flags;
		filter.maskBits = m_mask.flags;
		filter.groupIndex = 0;
		return filter;
	}

	const std::vector<sptr<Collider>>* Body::GetColliders()
	{
		return &m_colliders;
	}

	b2AABB Body::GetAABB()
	{
		return b2Body_ComputeAABB(m_bodyId);
	}

	StaticBody::StaticBody(Vec2f position, Bitmask16 layer) : Body(layer)
	{
		CreateB2Body(b2_staticBody, position, layer);
	}
	StaticBody::StaticBody(StaticBody&& body) noexcept
	{
		MoveFrom(std::move(body));
	}
	KinematicBody::KinematicBody(Bitmask16 layer) : Body(layer)
	{
		CreateB2Body(b2_kinematicBody, Vec2f(0, 0), layer);
	}
	KinematicBody::KinematicBody(KinematicBody&& body) noexcept
	{
		MoveFrom(std::move(body));
	}

}
