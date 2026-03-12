#include "ehupch.h"
#include "Camera.h"

namespace Ehu {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top,
		float nearZ, float farZ) {
		SetProjection(left, right, bottom, top, nearZ, farZ);
		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float nearZ, float farZ) {
		m_Left = left; m_Right = right; m_Bottom = bottom; m_Top = top; m_NearZ = nearZ; m_FarZ = farZ;
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, nearZ, farZ);
	}

	void OrthographicCamera::RecalculateViewMatrix() {
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
		m_ViewMatrix = glm::inverse(transform);
	}

	PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float nearZ, float farZ) {
		SetProjection(fov, aspect, nearZ, farZ);
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::SetProjection(float fov, float aspect, float nearZ, float farZ) {
		m_Fov = fov; m_Aspect = aspect; m_NearZ = nearZ; m_FarZ = farZ;
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearZ, farZ);
	}

	void PerspectiveCamera::RecalculateViewMatrix() {
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0, 1, 0))
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1, 0, 0))
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
		m_ViewMatrix = glm::inverse(transform);
	}

} // namespace Ehu
