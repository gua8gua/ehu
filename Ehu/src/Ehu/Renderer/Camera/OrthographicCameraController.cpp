#include "ehupch.h"
#include "OrthographicCameraController.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, float zoomLevel)
		: m_AspectRatio(aspectRatio), m_ZoomLevel(zoomLevel),
		m_Camera(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel) {}

	void OrthographicCameraController::RecalculateProjection() {
		float halfSize = m_ZoomLevel;
		m_Camera.SetProjection(
			-m_AspectRatio * halfSize, m_AspectRatio * halfSize,
			-halfSize, halfSize);
	}

	void OrthographicCameraController::OnUpdate(const TimeStep& dt) {
		if (m_MoveDelta.x != 0.0f || m_MoveDelta.y != 0.0f) {
			m_Position.x += m_MoveDelta.x * m_MoveSpeed * static_cast<float>(dt.GetSeconds());
			m_Position.y += m_MoveDelta.y * m_MoveSpeed * static_cast<float>(dt.GetSeconds());
			m_Camera.SetPosition(m_Position);
			m_MoveDelta = { 0.0f, 0.0f };
		}
		if (m_ZoomDelta != 0.0f) {
			m_ZoomLevel -= m_ZoomDelta * m_ZoomSpeed;
			if (m_ZoomLevel < 0.1f) m_ZoomLevel = 0.1f;
			RecalculateProjection();
			m_ZoomDelta = 0.0f;
		}
	}

	void OrthographicCameraController::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(EHU_BIND_EVENT_FN(OrthographicCameraController::OnWindowResize));
	}

	bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& e) {
		SetAspectRatio(static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight()));
		return false;
	}

} // namespace Ehu
