#include "pch.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SandCastle/Render/Transform.h"

#include "SandCastle/Core/Math.h"
#include "SandCastle/ECS/Entity.h"
namespace SandCastle
{
/*	Transform::Transform(Transform&& transform) noexcept :
		m_localPosition(transform.m_localPosition),
		m_localScale(transform.m_localScale),
		m_localRotation(transform.m_localRotation),
		m_haveParent(transform.m_haveParent),
		m_parent(transform.m_parent)
	{

	}*/
	/*Transform::Transform(Transform& transform):
		m_localPosition(transform.m_localPosition),
		m_localScale(transform.m_localScale),
		m_localRotation(transform.m_localRotation),
		m_haveParent(transform.m_haveParent),
		m_parent(transform.m_parent),
		m_transformMatrix(transform.m_transformMatrix),
		needCompute(false),
		matrixUpdated(false)
	{

	}*/
	Transform::Transform() :
		m_localPosition(0.f),
		m_localScale(1.f),
		m_localRotation(0.f),
		m_localSin(0.f),
		m_localCos(1.f),
		m_haveParent(false)

	{
	}

	Transform::Transform(Vec3f translation, Vec2f scale, float angle)
		: m_localPosition(translation),
		m_localScale(scale),
		m_localRotation(0.f),
		m_localSin(0.f),
		m_localCos(1.f),
		m_haveParent(false)

	{
		SetRotation(angle);
	}

	void Transform::SetParent(EntityId entity)
	{
		if (!Entity::registry.valid(entity))
		{
			LOG_ERROR("Cannot parent a transform to an invalid entity.");
			return;
		}
		if (Entity::registry.try_get<Transform>(entity) == nullptr)
		{
			LOG_ERROR("Cannot parent a transform to an entity that has no Transform.");
			return;
		}
		//A cycle would make GetWorld() recurse until the stack dies, so refuse to
		//build one. The walk is only as long as the chain and only runs on reparent.
		for (EntityId ancestor = entity;;)
		{
			const Transform& transform = Entity::registry.get<Transform>(ancestor);
			if (&transform == this)
			{
				LOG_ERROR("Cannot parent a transform to one of its own descendants.");
				return;
			}
			if (!transform.m_haveParent)
				break;
			ancestor = transform.m_parent;
		}

		m_parent = entity;
		m_haveParent = true;
	}

	void Transform::RemoveParent()
	{
		m_haveParent = false;
	}

	void Transform::SetPosition(Vec3f translation)
	{
		if (translation == m_localPosition)
			return;
		m_localPosition = translation;
	}

	void Transform::SetScale(Vec2f scale)
	{
		m_localScale = scale;
	}

	void Transform::SetPosition(float x, float y, float z)
	{
		m_localPosition = Vec3f(x, y, z);
	}

	void Transform::SetScale(float x, float y)
	{
		m_localScale = Vec2f(x, y);
	}

	void Transform::SetRotation(float angle)
	{
		//Wrapped into [0, 360) so that accumulating rotations for hours never
		//drifts into the float range where sin/cos lose precision.
		m_localRotation = Math::Repeat(angle, 360.f);
		const float radians = Math::Radians(m_localRotation);
		m_localSin = std::sin(radians);
		m_localCos = std::cos(radians);
	}

	void Transform::SetWorldPosition(Vec3f world)
	{
		if (!m_haveParent)
		{
			SetPosition(world);
			return;
		}

		const WorldTransform parent = Entity::registry.get<Transform>(m_parent).GetWorld();

		//Exact inverse of the composition in GetWorld: undo the parent's rotation
		//first, then its scale.
		const float deltaX = world.x - parent.position.x;
		const float deltaY = world.y - parent.position.y;
		const float unrotatedX = parent.cosRot * deltaX + parent.sinRot * deltaY;
		const float unrotatedY = parent.cosRot * deltaY - parent.sinRot * deltaX;

		//A zero scale axis collapses every local value onto the same world point,
		//so no local value maps back: keep the current one instead of dividing.
		SetPosition(Vec3f(
			parent.scale.x != 0.f ? unrotatedX / parent.scale.x : m_localPosition.x,
			parent.scale.y != 0.f ? unrotatedY / parent.scale.y : m_localPosition.y,
			world.z - parent.position.z));
	}

	void Transform::SetWorldPosition(float x, float y, float z)
	{
		SetWorldPosition(Vec3f(x, y, z));
	}

	void Transform::SetWorldRotation(float angleDegrees)
	{
		if (!m_haveParent)
		{
			SetRotation(angleDegrees);
			return;
		}
		SetRotation(angleDegrees - Entity::registry.get<Transform>(m_parent).GetWorld().rotation);
	}

	void Transform::Move(Vec3f translation)
	{
		m_localPosition += translation;
	}

	void Transform::Move(float x, float y, float z)
	{
		m_localPosition += Vec3f(x, y, z);
	}

	void Transform::Scale(Vec2f scale)
	{
		m_localScale *= scale;
	}

	void Transform::Scale(float x, float y)
	{
		m_localScale *= Vec2f(x, y);
	}

	void Transform::Rotate(float anglesDegrees)
	{
		SetRotation(m_localRotation + anglesDegrees);
	}

/*	Transform Transform::operator+(const Transform& trans)
	{
		Transform t(m_localPosition + trans.m_localPosition,
			m_localScale * trans.m_localScale,
			m_localRotation + trans.m_localRotation);
		return t;
	}

	Transform& Transform::operator+=(const Transform& trans)
	{
		m_localPosition += trans.m_localPosition;
		m_localScale *= trans.m_localScale;
		m_localRotation += trans.m_localRotation;

		return *this;
	}

	Transform& Transform::operator=(const Transform& trans)
	{
		m_localPosition = trans.m_localPosition;
		m_localScale = trans.m_localScale;
		m_localRotation = trans.m_localRotation;
		m_haveParent = trans.m_haveParent;
		m_parent = trans.m_parent;

		return *this;
	}*/




}
