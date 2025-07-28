#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Components.h"

namespace SandCastle
{
	class Camera
	{
	public:
		PointableComponent;

		Camera();
		~Camera();
		void SetOrthographic(bool orthographic);
		void SetPosition(Vec3f position);
		void SetRotation(Vec3f eulerAngles);
		void SetTarget(Vec3f target);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetTarget(float x, float y, float z);
		void SetFieldOfView(float fieldOfView);
		void SetAspectRatio(float aspectRatio);
		void SetAspectRatio(Vec2u xOverY);
		void SetNearClippingPlane(float nearClippingPlane);
		void SetFarClippingPlane(float farClippingPlane);
		float GetNearClippingPlane();
		float GetFarClippingPlane();

		void MoveWorld(Vec3f offset);
		void MoveWorld(float x, float y, float z);
		void MoveLocalX(float offset);
		void MoveLocalZ(float offset);
		void Yaw(float yaw);
		void Pitch(float pitch);
		void Roll(float roll);
		void SetYaw(float yaw);
		void SetPitch(float pitch);
		void SetRoll(float roll);
		
		Vec3f GetPosition() const;
		float GetAspectRatio() const;
		Mat4 GetViewMatrix() const;
		Mat4 GetProjectionMatrix() const;
		Mat4 GetTargetViewMatrix() const;

		Vec2f WorldToScreen(Vec3f worldPosition, Vec2u screenSize) const;
		Vec3f ScreenToWorld(Vec2f screenPosition, Vec2u screenSize) const;

		float worldToScreenRatio;
		bool isMain;

	private:
		void ComputeViewMatrix() const;
		void ComputeProjectionMatrix() const;
		void ComputeDirection();

		Vec3f m_target;

		Vec3f m_position;
		Vec3f m_worldUp;
		Vec3f m_localBack;
		Vec3f m_localRight;
		Vec3f m_localUp;

		float m_yaw;
		float m_pitch;
		float m_roll;

		float m_fieldOfView;
		float m_aspectRatio;
		float m_nearClippingPlane;
		float m_farClippingPlane;

		mutable Mat4 m_projectionMatrix;
		mutable Mat4 m_viewMatrix;
		mutable bool m_needComputeViewMatrix;
		mutable bool m_needComputeProjectionMatrix;

		bool m_orthographic;
	};

}