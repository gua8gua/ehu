#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	/// 相机抽象基类（平台无关）
	class EHU_API Camera {
	public:
		virtual ~Camera() = default;

		/// 获取投影矩阵
		virtual const glm::mat4& GetProjectionMatrix() const = 0;
		/// 获取视图矩阵
		virtual const glm::mat4& GetViewMatrix() const = 0;
		/// 获取视图投影矩阵
		virtual glm::mat4 GetViewProjectionMatrix() const = 0;
	};

	/// 2D 正射相机
	class EHU_API OrthographicCamera : public Camera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top,
			float nearZ = -1.0f, float farZ = 1.0f);

		const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		glm::mat4 GetViewProjectionMatrix() const override { return m_ProjectionMatrix * m_ViewMatrix; }

		void SetProjection(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		const glm::vec3& GetPosition() const { return m_Position; }
		float GetRotation() const { return m_Rotation; }

		float GetLeft() const { return m_Left; }
		float GetRight() const { return m_Right; }
		float GetBottom() const { return m_Bottom; }
		float GetTop() const { return m_Top; }
		float GetNearZ() const { return m_NearZ; }
		float GetFarZ() const { return m_FarZ; }

	private:
		void RecalculateViewMatrix();

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
		float m_Left = 0.0f, m_Right = 0.0f, m_Bottom = 0.0f, m_Top = 0.0f, m_NearZ = -1.0f, m_FarZ = 1.0f;
	};

	/// 3D 透视相机（占位，后续实现）
	class EHU_API PerspectiveCamera : public Camera {
	public:
		PerspectiveCamera(float fov, float aspect, float nearZ, float farZ);

		const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		glm::mat4 GetViewProjectionMatrix() const override { return m_ProjectionMatrix * m_ViewMatrix; }

		void SetProjection(float fov, float aspect, float nearZ, float farZ);
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetRotation() const { return m_Rotation; }

		float GetFov() const { return m_Fov; }
		float GetAspect() const { return m_Aspect; }
		float GetNearZ() const { return m_NearZ; }
		float GetFarZ() const { return m_FarZ; }

	private:
		void RecalculateViewMatrix();

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		float m_Fov = 60.0f, m_Aspect = 16.0f / 9.0f, m_NearZ = 0.1f, m_FarZ = 1000.0f;
	};

} // namespace Ehu
