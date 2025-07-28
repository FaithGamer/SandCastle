#include "pch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SandCastle/Render/Transform.h"

#include "SandCastle/ECS/Entity.h"
namespace SandCastle
{
	Transform::Transform(Transform&& transform) noexcept :
		m_localPosition(transform.m_localPosition),
		m_localScale(transform.m_localScale),
		m_localRotation(transform.m_localRotation),
		m_haveParent(transform.m_haveParent),
		m_parent(transform.m_parent),
		m_transformMatrix(transform.m_transformMatrix),
		needCompute(transform.needCompute),
		matrixUpdated(transform.matrixUpdated)
	{

	}
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
		m_transformMatrix(1.f),
		needCompute(true),
		matrixUpdated(true),
		m_haveParent(false),
		m_parent(EntityId(0))

	{
	}

	Transform::Transform(Vec3f translation, Vec3f scale, float angle)
		: m_localPosition(translation),
		m_localScale(scale),
		m_localRotation(0.f, 0.f, angle),
		m_transformMatrix(1.f),
		needCompute(true),
		matrixUpdated(true),
		m_haveParent(false),
		m_parent(EntityId(0))

	{
	}

	Transform::Transform(Vec3f translation, Vec3f scale, Vec3f angles)
		: m_localPosition(translation),
		m_localScale(scale),
		m_localRotation(angles),
		m_transformMatrix(1.f),
		needCompute(true),
		matrixUpdated(true),
		m_haveParent(false),
		m_parent(EntityId(0))

	{
	}

	void Transform::SetParent(EntityId entity)
	{
		m_parent = entity;
		m_haveParent = true;
		needCompute = true;
	}

	void Transform::RemoveParent()
	{
		m_haveParent = false;
		needCompute = true;
	}

	void Transform::SetPosition(Vec3f translation)
	{
		if (translation == m_localPosition)
			return;
		m_localPosition = translation;
		needCompute = true;
	}

	void Transform::SetScale(Vec3f scale)
	{
		if (m_localScale == scale)
			return;
		m_localScale = scale;
		needCompute = true;
	}

	void Transform::SetPosition(float x, float y, float z)
	{
		auto pos = Vec3f(x, y, z);
		if (m_localPosition == pos)
			return;
		m_localPosition = pos;
		needCompute = true;
	}

	void Transform::SetScale(float x, float y, float z)
	{
		auto scale = Vec3f(x, y, z);
		if (m_localScale == scale)
			return;
		m_localScale = Vec3f(x, y, z);
		needCompute = true;
	}

	void Transform::SetRotation(Vec3f angles)
	{
		if (m_localRotation == angles)
			return;
		m_localRotation = angles;
		needCompute = true;
	}

	void Transform::SetRotationZ(float angle)
	{
		if (m_localRotation.z == angle)
			return;
		m_localRotation.z = angle;
		needCompute = true;
	}

	void Transform::Move(Vec3f translation)
	{
		m_localPosition += translation;
		needCompute = true;
	}

	void Transform::Move(float x, float y, float z)
	{
		auto prev = m_localPosition;
		m_localPosition += Vec3f(x, y, z);
		needCompute = true;
	}

	void Transform::Scale(Vec3f scale)
	{
		m_localScale *= scale;
		needCompute = true;
	}

	void Transform::Scale(float x, float y, float z)
	{
		m_localScale *= Vec3f(x, y, z);
		needCompute = true;
	}

	void Transform::Rotate(Vec3f angles)
	{
		//TO DO: Quaternions
		m_localRotation += angles;
		needCompute = true;
	}

	void Transform::RotateZ(float angle)
	{
		//TO DO: Quaternions
		m_localRotation.z += angle;
		needCompute = true;
	}

	Transform Transform::operator+(const Transform& trans)
	{
		Transform t(m_localPosition + trans.m_localPosition,
			m_localScale * trans.m_localScale,
			m_localRotation + trans.m_localRotation);
		//todo see warning
		return t;
	}

	Transform& Transform::operator+=(const Transform& trans)
	{
		m_localPosition += trans.m_localPosition;
		m_localScale *= trans.m_localScale;
		m_localRotation += trans.m_localRotation;

		needCompute = true;

		return *this;
	}

	Transform& Transform::operator=(const Transform& trans)
	{
		m_localPosition = trans.m_localPosition;
		m_localScale = trans.m_localScale;
		m_localRotation = trans.m_localRotation;
		m_haveParent = trans.m_haveParent;
		m_parent = trans.m_parent;
		needCompute = trans.needCompute;
		matrixUpdated = trans.matrixUpdated;
		m_transformMatrix = trans.m_transformMatrix;


		return *this;
	}

	Vec3f Transform::GetPosition() const
	{
		if (m_haveParent)
		{
			if (needCompute)
			{
				ComputeMatrix();
			}
			auto parent = Entity(m_parent).GetComponent<Transform>();
			Mat4 mat = parent->GetTransformMatrix() * m_transformMatrix;
			return Vec3f(mat[3]);
		}
		return m_localPosition;
	}

	Vec3f Transform::GetLocalPosition() const
	{
		return m_localPosition;
	}

	Vec3f Transform::GetScale() const
	{
		if (m_haveParent)
		{
			auto parent = Entity(m_parent).GetComponent<Transform>();
			return parent->GetScale() * m_localScale;
		}
		return m_localScale;
	}

	Vec3f Transform::GetLocalScale() const
	{
		return m_localScale;
	}

	Vec3f Transform::GetRotation() const
	{
		if (m_haveParent)
		{
			auto parent = Entity(m_parent).GetComponent<Transform>();
			return parent->GetRotation() + m_localRotation;
		}
		return m_localRotation;
	}

	Vec3f Transform::GetLocalRotation() const
	{
		return m_localRotation;
	}

	Mat4 Transform::GetTransformMatrix() const
	{
		if (needCompute)
		{
			ComputeMatrix();
		}
		if (m_haveParent)
		{
			auto parent = Entity(m_parent).GetComponent<Transform>();
			return parent->GetTransformMatrix() * m_transformMatrix;
		}
		return m_transformMatrix;
	}

	void Transform::ComputeMatrix() const
	{
		Mat4 matrix(1.f);

		matrix = glm::translate(matrix, (glm::vec3)m_localPosition);
		matrix = glm::rotate(matrix, glm::radians(-m_localRotation.z), glm::vec3(0, 0, 1));
		matrix = glm::scale(matrix, (glm::vec3)m_localScale);
		m_transformMatrix = matrix;
		matrixUpdated = true;
		needCompute = false;
	}
}