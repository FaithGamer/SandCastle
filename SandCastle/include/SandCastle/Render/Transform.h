#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{

	/// @brief World-space state of a Transform, produced by Transform::GetWorld().
	/// sinRot/cosRot are the sine and cosine of `rotation`; they are carried along
	/// so that composing a whole hierarchy needs no trigonometry.
	struct WorldTransform
	{
		Vec3f position;
		Vec2f scale;
		float rotation;
		float sinRot;
		float cosRot;
	};

	/// @brief 2D transform component (3D position, 2D scale, Z rotation).
	/// Composes with a parent Transform when one is set: GetPosition/Scale/Rotation
	/// return the world-space combined value, GetLocal* returns the local one.
	/// Parenting is done through Entity::AddChild / Entity::Unparent, never here.
	/// Add via Entity::Add<Transform>(...).
	class Transform
	{
	public:
		//Transform(Transform&& transform) noexcept;
		//Transform(Transform& transform);
		Transform();
		Transform(Vec3f translation, Vec2f scale, float angle);

		/// @brief Set local position (relative to parent if any).
		void SetPosition(Vec3f translation);
		void SetPosition(float x, float y, float z);
		/// @brief Set local scale (relative to parent if any).
		void SetScale(Vec2f scale);
		void SetScale(float x, float y);
		/// @brief Set local Z rotation in degrees. Wrapped into [0, 360).
		void SetRotation(float angleDegrees);

		/// @brief Set the position in world space, converting back through the
		/// parent chain. Use when an outside authority (physics) hands you a
		/// world value. Identical to SetPosition when there is no parent.
		void SetWorldPosition(Vec3f world);
		void SetWorldPosition(float x, float y, float z);
		/// @brief Set the Z rotation in world space, in degrees.
		/// Identical to SetRotation when there is no parent.
		void SetWorldRotation(float angleDegrees);

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

		/// @brief Full world-space state, resolved through the parent chain.
		/// Costs one registry lookup per hierarchy level and no trigonometry.
		/// Prefer this over calling GetPosition/GetScale/GetRotation separately:
		/// each of those walks the chain on its own.
		/// Note the composition assumes the parent scale is uniform, or the child
		/// unrotated. A non-uniform parent scale under a rotated child is a real
		/// shear, which a position/scale/rotation triple cannot represent.
		inline WorldTransform GetWorld() const
		{
			if (!m_haveParent)
			{
				return WorldTransform{ m_localPosition, m_localScale, m_localRotation, m_localSin, m_localCos };
			}

			const WorldTransform p = Entity::registry.get<Transform>(m_parent).GetWorld();

			//The local offset is stretched along the parent's own axes first, then
			//spun by the parent's world rotation: world = Tp * Rp * Sp * localPos.
			//Reversing that order gets non-square parents wrong.
			const float offsetX = p.scale.x * m_localPosition.x;
			const float offsetY = p.scale.y * m_localPosition.y;

			return WorldTransform{
				Vec3f(
					p.position.x + p.cosRot * offsetX - p.sinRot * offsetY,
					p.position.y + p.sinRot * offsetX + p.cosRot * offsetY,
					//A Z rotation and a 2D scale leave depth alone
					p.position.z + m_localPosition.z),
				Vec2f(p.scale.x * m_localScale.x, p.scale.y * m_localScale.y),
				p.rotation + m_localRotation,
				//Angle addition on the cached local sine/cosine, so no level of the
				//chain ever calls sin/cos. What is needed here is the sine of the
				//parent's *accumulated* rotation, which is exactly what p carries.
				p.sinRot * m_localCos + p.cosRot * m_localSin,
				p.cosRot * m_localCos - p.sinRot * m_localSin
			};
		}

		inline Vec3f GetPosition() const
		{
			if (m_haveParent)
			{
				return GetWorld().position;
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
				return GetWorld().scale;
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
				return GetWorld().rotation;
			}
			return m_localRotation;
		}
		inline float GetLocalRotation() const
		{
			return m_localRotation;
		}
		/// @brief True when this transform composes with a parent's.
		inline bool HasParent() const
		{
			return m_haveParent;
		}

	private:

		//Parenting is Entity's business: it keeps the Parent/Children components
		//in sync so destruction can never leave a transform pointing at a dead
		//entity. Reaching SetParent directly would bypass that bookkeeping.
		friend class Entity;

		/// @brief Make this transform a child of `entity`'s transform (composed at runtime).
		void SetParent(EntityId entity);
		/// @brief Detach from current parent, becoming a world-space root again.
		void RemoveParent();

		Vec3f m_localPosition;
		Vec2f m_localScale;
		float m_localRotation;
		//Sine and cosine of m_localRotation, refreshed on every rotation write so
		//the read path stays free of trigonometry.
		float m_localSin;
		float m_localCos;
		bool m_haveParent;
		EntityId m_parent;

	};
}
