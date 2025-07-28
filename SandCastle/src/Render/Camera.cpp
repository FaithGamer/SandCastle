#include "pch.h"

#include <glm/gtc/matrix_transform.hpp>

#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Camera.h"
#include "SandCastle/Core/Log.h"



namespace SandCastle
{
	Camera::Camera()
		:m_position(0, 0, -1), m_target(0, 0, 0), m_worldUp(0, 1, 0), m_localBack(0, 0, 1), m_localRight(1, 0, 0), m_localUp(0, 1, 0),
		m_yaw(0), m_pitch(0), m_roll(0),
		m_fieldOfView(45.f), m_aspectRatio(1), m_nearClippingPlane(-98), m_farClippingPlane(98),
		m_needComputeProjectionMatrix(true), m_needComputeViewMatrix(true), m_projectionMatrix(1.f),
		m_viewMatrix(1.f), m_orthographic(true), m_orthographicZoom(1),
		worldToScreenRatio(0.01f), isMain(false)
	{
	}

	Camera::~Camera()
	{

	}

	void Camera::SetOrthographic(bool orthographic)
	{
		m_orthographic = orthographic;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetOrthographicZoom(float zoom)
	{
		m_orthographicZoom = zoom;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetPosition(Vec3f position)
	{
		m_position = position;
		m_needComputeViewMatrix = true;
	}

	void Camera::SetRotation(Vec3f eulerAngles)
	{
		m_yaw = eulerAngles.x;
		m_pitch = eulerAngles.y;
		m_roll = eulerAngles.z;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::SetTarget(Vec3f target)
	{
		m_target = target;
		m_needComputeViewMatrix = true;
	}

	void Camera::SetPosition(float x, float y, float z)
	{
		m_position = Vec3f(x, y, z);
		m_needComputeViewMatrix = true;
	}

	void Camera::SetRotation(float x, float y, float z)
	{
		m_yaw = x;
		m_pitch = y;
		m_roll = z;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::SetTarget(float x, float y, float z)
	{
		m_target = Vec3f(x, y, z);
		m_needComputeViewMatrix = true;
	}

	void Camera::SetFieldOfView(float fieldOfView)
	{
		m_fieldOfView = fieldOfView;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetAspectRatio(Vec2u xOverY)
	{
		m_aspectRatio = (float)xOverY.x / (float)xOverY.y;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetNearClippingPlane(float nearClippingPlane)
	{
		//Range recommended for compatibility with AMD 6750XT
		//-96 to +96 = 0.2 min distance for Z buffer)
		m_nearClippingPlane = nearClippingPlane;
		m_needComputeProjectionMatrix = true;
	}

	void Camera::SetFarClippingPlane(float farClippingPlane)
	{
		//Range recommended for compatibility with AMD 6750XT
		//-96 to +96 = 0.2 min distance for Z buffer)
		m_farClippingPlane = farClippingPlane;
		m_needComputeProjectionMatrix = true;
	}

	float Camera::GetNearClippingPlane()
	{
		return m_nearClippingPlane;
	}

	float Camera::GetFarClippingPlane()
	{
		return m_farClippingPlane;
	}

	void Camera::MoveWorld(Vec3f offset)
	{
		m_position += offset;
		m_needComputeViewMatrix = true;
	}

	void Camera::MoveWorld(float x, float y, float z)
	{
		m_position += Vec3f(x, y, z);
		m_needComputeProjectionMatrix = true;
	}

	void Camera::MoveLocalX(float offset)
	{
		m_position += Vec3f(m_localRight.x * offset,
			m_localRight.y * offset,
			m_localRight.z * offset);
		m_needComputeViewMatrix = true;
	}

	void Camera::MoveLocalZ(float offset)
	{
		m_position += Vec3f(m_localBack.x * offset,
			m_localBack.y * offset,
			m_localBack.z * offset);
		m_needComputeViewMatrix = true;
	}
	void Camera::Yaw(float yaw)
	{
		m_yaw += yaw;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::Pitch(float pitch)
	{
		m_pitch += pitch;
		ComputeDirection();
		m_needComputeViewMatrix = true;

	}

	void Camera::Roll(float roll)
	{
		m_roll += roll;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::SetYaw(float yaw)
	{
		m_yaw = yaw;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::SetPitch(float pitch)
	{
		m_pitch = pitch;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}

	void Camera::SetRoll(float roll)
	{
		m_roll = roll;
		ComputeDirection();
		m_needComputeViewMatrix = true;
	}
	Vec3f Camera::GetPosition() const
	{
		//
		// turn Vec3f(m_position.x * m_aspectRatio *worldToScreenRatio, m_position.y * worldToScreenRatio, m_position.z * worldToScreenRatio);

		return m_position;
	}
	float Camera::GetAspectRatio() const
	{
		return m_aspectRatio;
	}
	Mat4 Camera::GetViewMatrix() const
	{
		if (m_needComputeViewMatrix)
		{
			ComputeViewMatrix();
			m_needComputeViewMatrix = false;
		}
		return m_viewMatrix;
	}

	Mat4 Camera::GetProjectionMatrix() const
	{
		if (m_needComputeProjectionMatrix)
		{
			ComputeProjectionMatrix();
			m_needComputeProjectionMatrix = false;
		}
		return m_projectionMatrix;
	}

	Mat4 Camera::GetTargetViewMatrix() const
	{
		return glm::lookAt((glm::vec3)m_position, (glm::vec3)m_target, (glm::vec3)m_worldUp);
	}
	Vec2f Camera::WorldToScreen(Vec3f worldPosition, Vec2u screenSize) const
	{
		if (m_orthographic)
		{
			float ratio = worldToScreenRatio;

			ratio *= m_orthographicZoom;
			Vec2f screenNorm =
				Vec2f(-worldPosition.x / m_aspectRatio * ratio, worldPosition.y * ratio)
				- Vec2f(-m_position.x / m_aspectRatio * ratio, m_position.y * ratio);
			return Vec2f((-screenNorm.x + 0.5f) * screenSize.x, (-screenNorm.y + 0.5f) * screenSize.y);
		}
		else
		{
			LOG_WARN("WorldToScreen not implemented for perspective camera");
			return Vec2f(0, 0);
		}

	}
	Vec3f Camera::ScreenToWorld(Vec2f screenPosition, Vec2u screenSize) const
	{
		//todo perspective
		if (m_orthographic)
		{
			Vec2f screenNorm = { screenPosition.x / screenSize.x - 0.5f, screenPosition.y / screenSize.y - 0.5f };
			return{ screenNorm.x * m_aspectRatio / worldToScreenRatio / m_orthographicZoom + m_position.x,
			-screenNorm.y / worldToScreenRatio / m_orthographicZoom + m_position.y, 0 };
		}
		else
		{
			LOG_WARN("ScreenToWorld not implemented for perspective camera");
			return Vec3f(0, 0, 0);
		}
	}

	void Camera::ComputeViewMatrix() const
	{
		m_viewMatrix = glm::lookAt(
			(glm::vec3)m_position * worldToScreenRatio * 2.f,
			(glm::vec3)m_position * worldToScreenRatio * 2.f + (glm::vec3)m_localBack, (glm::vec3)m_localUp);

		//view matrix will be composed like this
		// 
		//		r = localRight
		//		u = localUp
		//		d = localBack
		//		p = position

		/*

		r.x, r.y, r.z, p.x
		u.x, u.y, u.z, p.y
		d.x, d.y, d.z, p.z
		0.0, 0.0, 0.0, 1.0

		*/
	}

	void Camera::ComputeDirection()
	{
		//Calculate view direction (in fact the opposite this is why it's called back here)
		Vec3f back;
		back.x = cos(glm::radians(m_yaw - 90.0f)) * cos(glm::radians(m_pitch));
		back.y = sin(glm::radians(m_pitch));
		back.z = sin(glm::radians(m_yaw - 90.0f)) * cos(glm::radians(m_pitch));

		// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_localBack = back.Normalized();
		m_localRight = (m_localBack.Cross(m_worldUp)).Normalized();
		//m_localRight = glm::normalize(glm::cross((glm::vec3)m_localBack, (glm::vec3)m_worldUp));
		/*Vec3f rightWithRoll = m_localRight;
		rightWithRoll.x += cos(glm::radians(m_roll));
		rightWithRoll.y -= sin(glm::radians(m_roll));*/
		m_localUp = (m_localRight.Cross(m_localBack)).Normalized();
		//m_localUp = glm::normalize(glm::cross((glm::vec3)m_localRight, (glm::vec3)m_localBack));
	}

	void Camera::ComputeProjectionMatrix() const
	{
		if (m_orthographic)
		{
			m_projectionMatrix = glm::ortho(
				m_aspectRatio * m_orthographicZoom,
				-m_aspectRatio * m_orthographicZoom,
				-m_orthographicZoom,
				m_orthographicZoom,
				m_nearClippingPlane,
				m_farClippingPlane);

			/*m_projectionMatrix = glm::ortho(
				 -m_orthographicZoom,
				m_orthographicZoom,
				1.f/ -m_aspectRatio * m_orthographicZoom,
				1.f / m_aspectRatio *m_orthographicZoom,
				-100.f,
				100.f);*/
		}
		else
		{
			m_projectionMatrix = glm::perspective(m_fieldOfView, m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
		}
	}
}