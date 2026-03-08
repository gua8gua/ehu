#include "ehupch.h"
#include "EditorCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Ehu {

	EditorCamera::EditorCamera(float fov, float aspect, float nearZ, float farZ)
		: m_Fov(fov), m_Aspect(aspect), m_NearZ(nearZ), m_FarZ(farZ) {
		RecalculateProjection();
		RecalculateView();
	}

	void EditorCamera::SetViewportSize(float width, float height) {
		m_Aspect = width / height;
		RecalculateProjection();
	}

	void EditorCamera::OnUpdate() {
		if (m_PitchDelta != 0.0f || m_YawDelta != 0.0f) {
			m_Yaw += m_YawDelta * m_RotationSpeed;
			m_Pitch += m_PitchDelta * m_RotationSpeed;
			m_PitchDelta = m_YawDelta = 0.0f;
			RecalculateView();
		}
		if (m_ZoomDelta != 0.0f) {
			m_Distance -= m_ZoomDelta * m_ZoomSpeed;
			if (m_Distance < m_MinDistance) m_Distance = m_MinDistance;
			if (m_Distance > m_MaxDistance) m_Distance = m_MaxDistance;
			m_ZoomDelta = 0.0f;
			RecalculateView();
		}
	}

	void EditorCamera::RecalculateProjection() {
		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_Aspect, m_NearZ, m_FarZ);
	}

	void EditorCamera::RecalculateView() {
		m_Position = m_FocalPoint - m_Distance * (
			glm::vec3(
				cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw)),
				sin(glm::radians(m_Pitch)),
				cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw))
			)
		);
		m_ViewMatrix = glm::lookAt(m_Position, m_FocalPoint, glm::vec3(0.0f, 1.0f, 0.0f));
	}

} // namespace Ehu
