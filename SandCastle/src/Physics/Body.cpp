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

	Vec2f Body::GetPosition() const
	{
		return b2Body_GetPosition(m_bodyId);
	}

	float Body::GetRotation() const
	{
		return Math::Degrees(b2Rot_GetAngle(b2Body_GetRotation(m_bodyId)));
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

	DynamicBody::DynamicBody(Vec2f position, Bitmask16 layer) : Body(layer),
		m_prevPosition(position),
		m_currPosition(position)
	{
		CreateB2Body(b2_dynamicBody, position, layer);
	}
	DynamicBody::DynamicBody(DynamicBody&& body) noexcept :
		m_prevPosition(body.m_prevPosition),
		m_currPosition(body.m_currPosition),
		m_prevRotation(body.m_prevRotation),
		m_currRotation(body.m_currRotation)
	{
		MoveFrom(std::move(body));
	}

	void DynamicBody::UpdateTransform(Vec3f position, float rotation)
	{
		Body::UpdateTransform(position, rotation);
		//Teleport: snap the interpolation to the new state instead of gliding there
		m_currPosition = GetPosition();
		m_currRotation = GetRotation();
		m_prevPosition = m_currPosition;
		m_prevRotation = m_currRotation;
	}

	void DynamicBody::SetVelocity(Vec2f velocity)
	{
		//Setting the velocity of a sleeping body has no effect until it wakes
		b2Body_SetAwake(m_bodyId, true);
		b2Body_SetLinearVelocity(m_bodyId, velocity);
	}

	Vec2f DynamicBody::GetVelocity() const
	{
		return b2Body_GetLinearVelocity(m_bodyId);
	}

	void DynamicBody::SetAngularVelocity(float degreesPerSecond)
	{
		b2Body_SetAwake(m_bodyId, true);
		b2Body_SetAngularVelocity(m_bodyId, Math::Radians(degreesPerSecond));
	}

	float DynamicBody::GetAngularVelocity() const
	{
		return Math::Degrees(b2Body_GetAngularVelocity(m_bodyId));
	}

	void DynamicBody::ApplyForce(Vec2f force)
	{
		b2Body_ApplyForceToCenter(m_bodyId, force, true);
	}

	void DynamicBody::ApplyImpulse(Vec2f impulse)
	{
		b2Body_ApplyLinearImpulseToCenter(m_bodyId, impulse, true);
	}

	void DynamicBody::SetGravityScale(float scale)
	{
		b2Body_SetAwake(m_bodyId, true);
		b2Body_SetGravityScale(m_bodyId, scale);
	}

	float DynamicBody::GetGravityScale() const
	{
		return b2Body_GetGravityScale(m_bodyId);
	}

	void DynamicBody::SetFixedRotation(bool fixed)
	{
		b2Body_SetFixedRotation(m_bodyId, fixed);
	}

	void DynamicBody::SetLinearDamping(float damping)
	{
		b2Body_SetLinearDamping(m_bodyId, damping);
	}

	void DynamicBody::SetAngularDamping(float damping)
	{
		b2Body_SetAngularDamping(m_bodyId, damping);
	}

	float DynamicBody::GetMass() const
	{
		return b2Body_GetMass(m_bodyId);
	}
}
