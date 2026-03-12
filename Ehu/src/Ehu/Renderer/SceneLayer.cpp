#include "ehupch.h"
#include "SceneLayer.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include "RenderQueue.h"
#include "Renderer/Camera/Camera.h"
#include "Core/Application.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ehu {

	namespace {

		void RunCameraSync(World& w) {
			w.Each<TransformComponent, CameraComponent>([&](Entity, TransformComponent& t, CameraComponent& c) {
				if (!c.Camera) return;
				Camera* cam = c.Camera;
				const glm::vec3 pos = t.Position;
				const glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(t.Rotation));
				if (auto* pc = dynamic_cast<PerspectiveCamera*>(cam)) {
					pc->SetPosition(pos);
					pc->SetRotation(eulerDeg);
				} else if (auto* oc = dynamic_cast<OrthographicCamera*>(cam)) {
					oc->SetPosition(pos);
					oc->SetRotation(eulerDeg.z);
				}
			});
		}

		void SubmitEntitiesOfLayerToQueue(World& world, const Layer* thisLayer, RenderQueue& queue, Camera* viewCamera) {
			world.Each<TransformComponent, SpriteComponent, TagComponent>([&](Entity, TransformComponent& t, SpriteComponent& s, TagComponent& tag) {
				if (tag.RenderLayer != thisLayer) return;
				const glm::mat4& wt = t.GetWorldMatrix();
				glm::vec3 pos(wt[3][0], wt[3][1], wt[3][2]);
				queue.SubmitQuad(pos, s.Size, s.Color, s.SortKey, s.Transparent, viewCamera, 0);
			});
			world.Each<TransformComponent, MeshComponent, TagComponent>([&](Entity, TransformComponent& t, MeshComponent& m, TagComponent& tag) {
				if (tag.RenderLayer != thisLayer) return;
				if (!m.VAO || m.IndexCount == 0) return;
				queue.SubmitMesh(m.VAO, m.IndexCount, t.GetWorldMatrix(), m.Color, m.SortKey, m.Transparent, viewCamera, 0);
			});
		}

	} // namespace

	SceneLayer::SceneLayer(const std::string& name)
		: Layer(name)
	{}

	SceneLayer::~SceneLayer() = default;

	void SceneLayer::OnAttach() {
		Layer::OnAttach();
	}

	void SceneLayer::OnUpdate(const TimeStep& timestep) {
		for (Scene* scene : Application::Get().GetActivatedScenes())
			scene->OnUpdate(timestep);
		OnUpdateScene(timestep);
	}

	void SceneLayer::SubmitTo(RenderQueue& queue) const {
		SubmitTo(queue, nullptr);
	}

	void SceneLayer::SubmitTo(RenderQueue& queue, Camera* viewCameraOverride) const {
		Camera* viewCam = viewCameraOverride;
		for (Scene* scene : Application::Get().GetActivatedScenes()) {
			if (!scene) continue;
			World& world = scene->GetWorld();
			RunCameraSync(world);
			if (!viewCam)
				viewCam = scene->GetMainCamera();
			if (!viewCam) continue;
			SubmitEntitiesOfLayerToQueue(world, this, queue, viewCam);
			if (viewCameraOverride) viewCam = viewCameraOverride;
		}
	}

} // namespace Ehu
