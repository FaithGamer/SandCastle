#include "pch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SandCastle/Render/Transform.h"

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
		m_haveParent(false)

	{
	}

	Transform::Transform(Vec3f translation, Vec2f scale, float angle)
		: m_localPosition(translation),
		m_localScale(scale),
		m_localRotation(angle),
		m_haveParent(false)

	{
	}

	void Transform::SetParent(EntityId entity)
	{
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
		m_localRotation = angle;
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
		m_localRotation += anglesDegrees;
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