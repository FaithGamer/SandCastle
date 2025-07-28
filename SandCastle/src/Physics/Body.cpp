#include "pch.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	Body::Body(Body&& body) noexcept
		:
		m_layer(body.m_layer),
		m_mask(body.m_mask),
		m_YisZ(body.m_YisZ),
		m_b2Body(body.m_b2Body),
		m_colliders(body.m_colliders)
	{

	}
	Body::Body(Bitmask16 layer) : m_layer(layer), m_mask(65535), m_YisZ(false), m_b2Body(nullptr)
	{

	}

	Body::~Body()
	{
		//Free the b2Body in the b2World
		m_b2Body->GetWorld()->DestroyBody(m_b2Body);
	}

	void Body::SetLayer(Bitmask16 layer)
	{
		m_layer = layer;
	}

	void Body::SetLayerMask(Bitmask16 mask)
	{
		m_mask = mask;
	}

	void Body::AddCollider(sptr<Collider> collider)
	{
		//Attach collider and body
		collider->SetBody(this, GetB2Filter(), userData);
		m_colliders.emplace_back(collider);
	}

	void Body::ClearCollider()
	{
		//to do
	}

	void Body::OverlappingBodies(std::vector<OverlapResult>& results)
	{
		Physics::BodyOverlap(results, this, m_mask);
	}

	bool Body::BodyOverlap(Body* body)
	{
		//to do
		return false;
	}

	bool Body::CircleOverlap(Vec2f point, float radius)
	{
		//to do
		return false;
	}

	bool Body::PointInside(Vec2f point)
	{
		//to do
		return false;
	}

	void Body::UpdateTransform(Vec3f position, float rotation)
	{
		//todo scale, needs to recreate the fixture

		b2Vec2 pos(position.x, position.y);
		if (m_YisZ)
		{
			pos.y = position.z;
		}
		//box2D rotation is counter-clockwise
		m_b2Body->SetTransform(pos, Math::Radians(-rotation));
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

	b2Body* Body::GetB2Body()
	{
		return m_b2Body;
	}

	b2Filter Body::GetB2Filter()
	{
		b2Filter filter;
		filter.categoryBits = m_layer.flags;
		filter.groupIndex = 0;
		filter.maskBits = m_mask.flags;
		return filter;
	}

	const std::vector<sptr<Collider>>* Body::GetColliders()
	{
		return &m_colliders;
	}

	b2AABB Body::GetAABB()
	{
		b2AABB aabb = m_colliders[0]->GetAABB();
		for (int i = 1; i < m_colliders.size(); i++)
		{
			auto caabb = m_colliders[i]->GetAABB();
			if (caabb.lowerBound.x < aabb.lowerBound.x)
				aabb.lowerBound.x = caabb.lowerBound.x;
			if (caabb.lowerBound.y < aabb.lowerBound.y)
				aabb.lowerBound.y = caabb.lowerBound.y;
			if (caabb.upperBound.x > aabb.upperBound.x)
				aabb.upperBound.x = caabb.upperBound.x;
			if (caabb.upperBound.y > aabb.upperBound.y)
				aabb.upperBound.y = caabb.upperBound.y;
		}
		return aabb;
	}

	StaticBody::StaticBody(Vec2f position, Bitmask16 layer) : Body(layer)
	{
		b2BodyDef def;


		def.type = b2BodyType::b2_staticBody;
		m_b2Body = Physics::GetB2World()->CreateBody(&def);
		m_b2Body->SetTransform(position, 0);
	}
	StaticBody::StaticBody(StaticBody&& body) noexcept
	{
		m_layer = body.m_layer;
		m_mask = body.m_mask;
		m_YisZ = body.m_YisZ;
		m_b2Body = body.m_b2Body;
		m_colliders = body.m_colliders;
	}
	KinematicBody::KinematicBody(Bitmask16 layer) : Body(layer)
	{
		b2BodyDef def;
		def.type = b2BodyType::b2_kinematicBody;
		m_b2Body = Physics::GetB2World()->CreateBody(&def);
		m_b2Body->SetTransform(Vec2f(0, 0), 0);
	}
	KinematicBody::KinematicBody(KinematicBody&& body) noexcept

	{
		m_layer = body.m_layer;
		m_mask = body.m_mask;
		m_YisZ = body.m_YisZ;
		m_b2Body = body.m_b2Body;
		m_colliders = body.m_colliders;
	}

}