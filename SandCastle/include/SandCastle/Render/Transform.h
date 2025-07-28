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
		Transform(Vec3f translation, Vec3f scale, float angle);
		Transform(Vec3f translation, Vec3f scale, Vec3f angles);

		void SetParent(EntityId entity);
		void RemoveParent();

		void SetPosition(Vec3f translation);
		void SetPosition(float x, float y, float z);
		void SetScale(Vec3f scale);
		void SetScale(float x, float y, float z);

		/// @brief Warning! Currently rotation works only for the Z axis
		/// @param angles Degrees
		void SetRotation(Vec3f angles);
		/// @brief Rotate on the Z axis (mainly for 2D usage)
		/// @param angles Degrees
		void SetRotationZ(float angle);
		
		void Move(Vec3f translation);
		void Move(float x, float y, float z);
		void Scale(Vec3f scale);
		void Scale(float x, float y, float z);
		void Rotate(Vec3f angles);
		void RotateZ(float angle);

		Transform operator+(const Transform& trans);
		Transform& operator+=(const Transform& trans);
		Transform& operator=(const Transform& trans);

		Vec3f GetPosition() const;
		Vec3f GetLocalPosition() const;
		Vec3f GetScale() const;
		Vec3f GetLocalScale() const;
		Vec3f GetRotation() const;
		Vec3f GetLocalRotation() const;

		Mat4 GetTransformMatrix() const;
		mutable bool needCompute;
		mutable bool matrixUpdated;
	private:
		void ComputeMatrix() const;
		
		Vec3f m_localPosition;
		Vec3f m_localScale;
		Vec3f m_localRotation;

		mutable bool m_haveParent;
		EntityId m_parent;
		
		mutable Mat4 m_transformMatrix;
	};
}