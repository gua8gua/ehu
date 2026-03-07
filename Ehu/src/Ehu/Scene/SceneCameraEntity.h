#pragma once

#include "Core/Core.h"
#include "SceneEntity.h"
#include "Renderer/Camera/Camera.h"

namespace Ehu {

	/// 相机对象：作为“物体”的子类存在于场景中，由场景选择主相机并在 Extract/Dispatch 阶段使用
	/// - SceneEntity 负责 Transform（位置/旋转/缩放）
	/// - SceneCameraEntity 持有 Camera，并在 SyncCameraFromTransform() 时将 Transform 同步到 Camera
	class EHU_API SceneCameraEntity : public SceneEntity {
	public:
		explicit SceneCameraEntity(Camera* camera) : m_Camera(camera) {}
		~SceneCameraEntity() {
			delete m_Camera;
			m_Camera = nullptr;
		}

		Camera* GetCamera() const { return m_Camera; }

		/// 将当前实体 Transform 同步到 Camera（用于主相机在每帧提取/渲染前更新 ViewMatrix）
		void SyncCameraFromTransform() const {
			if (!m_Camera) return;

			const glm::vec3 pos = GetPosition();
			const glm::vec3 eulerRad = GetRotationEuler();
			const glm::vec3 eulerDeg = glm::degrees(eulerRad);

			if (auto* pc = dynamic_cast<PerspectiveCamera*>(m_Camera)) {
				pc->SetPosition(pos);
				pc->SetRotation(eulerDeg);
			} else if (auto* oc = dynamic_cast<OrthographicCamera*>(m_Camera)) {
				oc->SetPosition(pos);
				oc->SetRotation(eulerDeg.z);
			}
		}

	private:
		Camera* m_Camera = nullptr;
	};

} // namespace Ehu

