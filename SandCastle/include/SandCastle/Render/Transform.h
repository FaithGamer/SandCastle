#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{


	/// @brief 2D transform component (3D position, 2D scale, Z rotation).
	/// Composes with a parent Transform when one is set: GetPosition/Scale/Rotation
	/// return the world-space combined value, GetLocal* returns the local one.
	/// Add via Entity::Add<Transform>(...).
	class Transform
	{
	public:
		//Transform(Transform&& transform) noexcept;
		//Transform(Transform& transform);
		Transform();
		Transform(Vec3f translation, Vec2f scale, float angle);

		/// @brief Make this transform a child of `entity`'s transform (composed at runtime).
		void SetParent(EntityId entity);
		/// @brief Detach from current parent, becoming a world-space root again.
		void RemoveParent();

		/// @brief Set local position (relative to parent if any).
		void SetPosition(Vec3f translation);
		void SetPosition(float x, float y, float z);
		/// @brief Set local scale (relative to parent if any).
		void SetScale(Vec2f scale);
		void SetScale(float x, float y);
		/// @brief Set local Z rotation in degrees.
		void SetRotation(float angleDegrees);

		/// @brief Translate the local position by `translation`.
		void Move(Vec3f translation);
		void Move(float x, float y, float z);
		/// @brief Multiply the local scale component-wise by `scale`.
		void Scale(Vec2f scale);
		void Scale(float x, float y);
		/// @brief Add `angleDegrees` to the local rotation.
		void Rotate(float angleDegrees);

		//Transform operator+(const Transform& trans);
		//Transform& operator+=(const Transform& trans);
	//	Transform& operator=(const Transform& trans);

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