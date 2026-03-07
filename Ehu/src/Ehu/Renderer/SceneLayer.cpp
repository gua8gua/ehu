#include "ehupch.h"
#include "SceneLayer.h"
#include "Scene/Scene.h"
#include "Scene/SceneCameraEntity.h"
#include "Scene/SceneEntity.h"
#include "RenderQueue.h"
#include "Core/Application.h"
#include <glm/glm.hpp>

namespace Ehu {

	/// Extract 阶段：将归属本层的实体从场景中提取并打包提交到 queue（含视图裁剪占位，后续可加视锥裁剪）
	static void SubmitEntitiesOfLayerToQueue(const Scene& scene, const Layer* thisLayer, RenderQueue& queue, Camera* viewCamera) {
		for (SceneEntity* e : scene.GetEntities()) {
			if (e->GetRenderLayer() != thisLayer)
				continue;
			const glm::mat4& wt = e->GetTransform();
			if (e->HasMeshComponent()) {
				const MeshComponent& m = e->GetMeshComponent();
				if (m.VAO && m.IndexCount > 0)
					queue.SubmitMesh(m.VAO, m.IndexCount, wt, m.Color, m.SortKey, m.Transparent, viewCamera, 0);
			}
			if (e->HasSpriteComponent()) {
				const SpriteComponent& s = e->GetSpriteComponent();
				glm::vec3 pos(wt[3][0], wt[3][1], wt[3][2]);
				queue.SubmitQuad(pos, s.Size, s.Color, s.SortKey, s.Transparent, viewCamera, 0);
			}
		}
	}

	SceneLayer::SceneLayer(const std::string& name)
		: Layer(name)
	{}

	SceneLayer::~SceneLayer() = default;

	void SceneLayer::OnAttach() {
		Layer::OnAttach();
	}

	void SceneLayer::OnUpdate(const TimeStep& timestep) {
		// Phase1 逻辑 Tick：驱动所有已激活场景的 OnUpdate（ECS/Systems），再执行本层额外逻辑
		for (Scene* scene : Application::Get().GetActivatedScenes())
			scene->OnUpdate(timestep);
		OnUpdateScene(timestep);
	}

	void SceneLayer::SubmitTo(RenderQueue& queue) const {
		// Phase2 Extract：从所有已激活场景中提取归属本层的实体；先取得该场景主相机用于后续位置变换/排序，无主相机直接跳过
		for (Scene* scene : Application::Get().GetActivatedScenes()) {
			if (!scene) continue;
			SceneCameraEntity* mainCamEnt = scene->GetMainCamera();
			if (!mainCamEnt) continue;
			Camera* cam = mainCamEnt->GetCamera();
			if (!cam) continue;
			mainCamEnt->SyncCameraFromTransform();
			SubmitEntitiesOfLayerToQueue(*scene, this, queue, cam);
		}
	}

} // namespace Ehu
