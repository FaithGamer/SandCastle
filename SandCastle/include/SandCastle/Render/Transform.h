#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{


	class Transform
	{
	public:
		Transform(Transform&& transform) noexcept;
		//Transform(Transform& transform);
		Transform();
		Transform(Vec3f translation, Vec2f scale, float angle);

		void SetParent(EntityId entity);
		void RemoveParent();

		void SetPosition(Vec3f translation);
		void SetPosition(float x, float y, float z);
		void SetScale(Vec2f scale);
		void SetScale(float x, float y);
		void SetRotation(float angleDegrees);

		void Move(Vec3f translation);
		void Move(float x, float y, float z);
		void Scale(Vec2f scale);
		void Scale(float x, float y);
		void Rotate(float angleDegrees);

		Transform operator+(const Transform& trans);
		Transform& operator+=(const Transform& trans);
		Transform& operator=(const Transform& trans);

		inline Vec3f GetPosition() const
		{
			if (m_haveParent)
			{
				return Entity::registry.get<Transform>(m_parent).GetPosition() + m_localPosition;
			}
			return m_localPosition;
		}
		inline Vec3f GetLocalPosition() const
		{
			return m_localPosition;
		}
		inline Vec2f GetScale() const
		{
			if (m_haveParent)
			{
				return Entity::registry.get<Transform>(m_parent).GetScale() * m_localScale;
			}
			return m_localScale;
		}
		inline Vec2f GetLocalScale() const
		{
			return m_localScale;
		}
		inline float GetRotation() const
		{
			if (m_haveParent)
			{
				return Entity::registry.get<Transform>(m_parent).GetRotation() + m_localRotation;
			}
			return m_localRotation;
		}
		inline float GetLocalRotation() const
		{
			return m_localRotation;
		}

	private:
		
		Vec3f m_localPosition;
		Vec2f m_localScale;
		float m_localRotation;
		mutable bool m_haveParent;
		EntityId m_parent;
		
	};
}