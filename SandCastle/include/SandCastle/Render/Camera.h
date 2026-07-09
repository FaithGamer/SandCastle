#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Components.h"

namespace SandCastle
{
	/// @brief View/projection generator used by the renderer.
	/// Defaults to an orthographic 2D camera. Provides yaw/pitch/roll, target
	/// look-at, and a "Constraints" struct for pixel-perfect rendering. The
	/// active camera is referenced by Renderer2D via Camera::main.
	class Camera
	{
	public:
		/// @brief Optional render-area constraints. Use SetDefault() for a 16:9 360p pixel-perfect setup.
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
		/// @brief Switch between orthographic (default, 2D) and perspective (3D) projection.
		void SetOrthographic(bool orthographic);
		/// @brief Set world-space position of the camera.
		void SetPosition(Vec3f position);
		/// @brief Set the camera rotation as Euler angles in degrees.
		void SetRotation(Vec3f eulerAngles);
		/// @brief Aim the camera at the given world-space target point.
		void SetTarget(Vec3f target);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetTarget(float x, float y, float z);
		/// @brief Vertical field of view in degrees (perspective only).
		void SetFieldOfView(float fieldOfView);
		/// @brief Override the projection aspect ratio (width/height).
		void SetAspectRatio(float aspectRatio);
		/// @brief Override the projection aspect ratio from a Vec2u dimension pair.
		void SetAspectRatio(Vec2u xOverY);
		void SetNearClippingPlane(float nearClippingPlane);
		void SetFarClippingPlane(float farClippingPlane);
		/// @brief Pixel perfect zoom according to constraints.
		/// The zoom snaps to values keeping a whole number of screen pixels per
		/// texel (integers when zooming in, multiples of 1/(targetHeight/pxStep)
		/// when unzooming). Unzooming is limited by the window resolution.
		/// your textures should be 1 PPU.
		void SetPxZoom(float scale);

		/// @brief Can be used for pixel perfect effect.
		/// Or just to ensure the render area is always of a specific aspect ratio
		/// @param constraints 
		void SetConstraints(Constraints constraints);

		/// @brief Translate the camera in world space by `offset`.
		void MoveWorld(Vec3f offset);
		void MoveWorld(float x, float y, float z);
		/// @brief Translate along the camera's local X axis.
		void MoveLocalX(float offset);
		/// @brief Translate along the camera's local Z axis (forward/back).
		void MoveLocalZ(float offset);
		/// @brief Rotate around the local Y axis (degrees, additive).
		void Yaw(float yaw);
		/// @brief Rotate around the local X axis (degrees, additive).
		void Pitch(float pitch);
		/// @brief Rotate around the local Z axis (degrees, additive).
		void Roll(float roll);
		void SetYaw(float yaw);
		void SetPitch(float pitch);
		void SetRoll(float roll);

		Vec3f GetPosition() const;
		float GetAspectRatio() const;
		/// @brief View matrix (world -> camera space).
		Mat4 GetViewMatrix() const;
		/// @brief Projection matrix (camera -> clip space).
		Mat4 GetProjectionMatrix() const;
		/// @brief View matrix derived from SetTarget instead of yaw/pitch/roll.
		Mat4 GetTargetViewMatrix() const;
		float GetNearClippingPlane();
		float GetFarClippingPlane();
		/// @brief Pixel-perfect target render size derived from constraints + window size.
		Vec2u GetTargetSize() const;
		/// @brief Reduction factor applied to zoom by pixel-perfect constraints.
		float GetReduction() const;
		Constraints GetConstraints() const;

		/// @brief Project a world-space position to screen-space pixel coordinates.
		Vec2f WorldToScreen(Vec3f worldPosition, Vec2u screenSize) const;
		/// @brief Unproject a screen-space pixel position back to world space.
		Vec3f ScreenToWorld(Vec2f screenPosition, Vec2u screenSize) const;

		/// @brief Zoom multiplier (1 = no zoom).
		float zoom;
		/// @brief Global pointer to the camera the renderer should sample from.
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