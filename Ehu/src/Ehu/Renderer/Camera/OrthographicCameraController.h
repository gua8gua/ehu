#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "TimeStep.h"
#include <glm/glm.hpp>

namespace Ehu {

	/// 2D 正交相机控制器：管理 aspect、zoom，平移/缩放/旋转由外部每帧设置后供 2D 层使用
	class EHU_API OrthographicCameraController {
	public:
		OrthographicCameraController(float aspectRatio, float zoomLevel = 1.0f);

		void OnUpdate(const TimeStep& dt);
		void OnEvent(Event& e);

		void SetPosition(const glm::vec3& pos) { m_Position = pos; m_Camera.SetPosition(pos); }
		void SetRotation(float rotation) { m_Rotation = rotation; m_Camera.SetRotation(rotation); }
		void SetZoom(float zoom) { m_ZoomLevel = zoom; RecalculateProjection(); }
		void SetAspectRatio(float aspect) { m_AspectRatio = aspect; RecalculateProjection(); }

		const glm::vec3& GetPosition() const { return m_Position; }
		float GetRotation() const { return m_Rotation; }
		float GetZoom() const { return m_ZoomLevel; }
		float GetAspectRatio() const { return m_AspectRatio; }

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		/// 由外部在 OnUpdate 前设置本帧的移动/缩放增量（不依赖 Input 模块）
		void SetMoveDelta(const glm::vec2& delta) { m_MoveDelta = delta; }
		void SetZoomDelta(float delta) { m_ZoomDelta = delta; }

	private:
		void RecalculateProjection();
		bool OnWindowResize(WindowResizeEvent& e);

		float m_AspectRatio = 1.0f;
		float m_ZoomLevel = 1.0f;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
		glm::vec2 m_MoveDelta = { 0.0f, 0.0f };
		float m_ZoomDelta = 0.0f;
		float m_MoveSpeed = 1.0f;
		float m_ZoomSpeed = 0.5f;

		OrthographicCamera m_Camera;
	};

} // namespace Ehu
