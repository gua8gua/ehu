#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Ehu {

	/// 编辑器用弧球/轨道相机：绕目标点旋转、缩放距离，透视投影
	class EHU_API EditorCamera : public Camera {
	public:
		EditorCamera(float fov, float aspect, float nearZ, float farZ);

		const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		glm::mat4 GetViewProjectionMatrix() const override { return m_ProjectionMatrix * m_ViewMatrix; }

		void SetViewportSize(float width, float height);
		void SetDistance(float distance) { m_Distance = distance; RecalculateView(); }
		void SetFov(float fov) { m_Fov = fov; RecalculateProjection(); }

		/// 本帧旋转增量（弧度）：pitchDelta 俯仰，yawDelta 偏航
		void SetRotationDelta(float pitchDelta, float yawDelta) { m_PitchDelta = pitchDelta; m_YawDelta = yawDelta; }
		/// 本帧缩放增量（改变 m_Distance）
		void SetZoomDelta(float delta) { m_ZoomDelta = delta; }
		/// 应用本帧增量并更新视图（由外部在每帧末调用）
		void OnUpdate();

		float GetDistance() const { return m_Distance; }
		const glm::vec3& GetPosition() const { return m_Position; }

	private:
		void RecalculateProjection();
		void RecalculateView();

		float m_Fov = 45.0f;
		float m_Aspect = 16.0f / 9.0f;
		float m_NearZ = 0.1f;
		float m_FarZ = 1000.0f;

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 5.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		float m_Distance = 5.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;

		float m_PitchDelta = 0.0f;
		float m_YawDelta = 0.0f;
		float m_ZoomDelta = 0.0f;
		float m_RotationSpeed = 0.5f;
		float m_ZoomSpeed = 1.0f;
		float m_MinDistance = 0.5f;
		float m_MaxDistance = 100.0f;
	};

} // namespace Ehu
