#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Components.h"

namespace SandCastle
{
	class Camera
	{
	public:
		struct Constraints
		{
			/// @brief pxStep 
			/// by what increment (in pixels) the render area is allowed to step
			/// example pxStep = 360. Render area can be: 360, 720, 1080, 1440...
			/// use this for pixel perfect render.
			/// let to 0 for default behaviour.
			unsigned int pxStep = 0; 
			/// @brief targetRatio
			/// what should be the render area W/H ratio.
			/// If the window width is too small, the render area will adjust to fit the target ratio.
			float targetRatio = 0; 
			/// @brief Stop rendering outside of the render area? (black bars if clear color is black)
			bool cropW = false;
			bool cropH = false;
			float pxZoom = 1.f;
			/// @brief Standard values for a 16/9 pixel art game on modern displays
			void SetDefault()
			{
				pxStep = 360;
				targetRatio = 16.f / 9.f;
				cropW = true;
				cropH = true;
			}
		};
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
		/// @brief Pixel perfect zoom according to constraints.
		/// unzooming will be limited depending on window resolution
		/// your textures should be 1 PPU.
		void SetPxZoom(float scale);

		/// @brief Can be used for pixel perfect effect.
		/// Or just to ensure the render area is always of a specific aspect ratio
		/// @param constraints 
		void SetConstraints(Constraints constraints);

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
		float GetNearClippingPlane();
		float GetFarClippingPlane();
		Vec2u GetTargetSize() const;
		float GetReduction() const;
		Constraints GetConstraints() const;

		Vec2f WorldToScreen(Vec3f worldPosition, Vec2u screenSize) const;
		Vec3f ScreenToWorld(Vec2f screenPosition, Vec2u screenSize) const;

		float zoom;
		static Camera* main;

	private:
		void ComputeViewMatrix() const;
		void ComputeProjectionMatrix() const;
		void ComputeDirection();
		void ComputePixelPerfect();
		void ComputeReduction();

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
		
		Constraints m_px;
		float m_reduction = 1.f;
		Vec2u m_targetSize = 0;

		mutable Mat4 m_projectionMatrix;
		mutable Mat4 m_viewMatrix;
		mutable bool m_needComputeViewMatrix;
		mutable bool m_needComputeProjectionMatrix;

		bool m_orthographic;
	};

}