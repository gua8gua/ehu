#include "EditorPanelData.h"
#include "EditorContext.h"
#include "Core/Application.h"
#include "Project/Project.h"
#include "Renderer/Camera/Camera.h"  // Scene 析构需要完整 Camera 类型
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include <glm/gtc/quaternion.hpp>

namespace Ehu {

	EditorPanelDataProvider& EditorPanelDataProvider::Get() {
		static EditorPanelDataProvider s_Instance;
		return s_Instance;
	}

	HierarchySnapshot EditorPanelDataProvider::GetHierarchySnapshot() {
		HierarchySnapshot snapshot;
		snapshot.HasProject = (Project::GetActive() != nullptr);
		m_HandleToSceneEntity.clear();
		if (!snapshot.HasProject)
			return snapshot;

		const auto& scenes = Application::Get().GetActivatedScenes();
		EditorContext& ctx = EditorContext::Get();

		for (Scene* scene : scenes) {
			if (!scene) continue;
			for (Entity e : scene->GetEntities()) {
				World& w = scene->GetWorld();
				const char* label = "Entity";
				if (TagComponent* tag = w.GetComponent<TagComponent>(e))
					label = tag->Name.empty() ? "Entity" : tag->Name.c_str();

				uint64_t handle = m_NextHandle++;
				m_HandleToSceneEntity[handle] = { scene, e };

				HierarchyEntry entry;
				entry.Label = label;
				entry.Handle = handle;
				entry.IsSelected = ctx.HasEntitySelection() && ctx.GetSelectedScene() == scene && ctx.GetSelectedEntity().id == e.id;
				snapshot.Entries.push_back(entry);
			}
		}
		return snapshot;
	}

	InspectorSnapshot EditorPanelDataProvider::GetInspectorSnapshot() {
		InspectorSnapshot snapshot;
		if (!Project::GetActive()) {
			snapshot.State = InspectorState::NoSelection;
			return snapshot;
		}

		EditorContext& ctx = EditorContext::Get();
		if (ctx.HasAssetSelection()) {
			snapshot.State = InspectorState::AssetSelection;
			snapshot.AssetPath = ctx.GetSelectedAsset();
			return snapshot;
		}

		if (!ctx.HasEntitySelection()) {
			snapshot.State = InspectorState::NoSelection;
			return snapshot;
		}

		Scene* scene = ctx.GetSelectedScene();
		Entity entity = ctx.GetSelectedEntity();
		bool sceneActive = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { sceneActive = true; break; }
		}
		bool entityValid = (scene && sceneActive) ? scene->GetWorld().IsValid(entity) : false;
		if (!scene || !sceneActive || !entityValid) {
			snapshot.State = InspectorState::SelectionInvalid;
			return snapshot;
		}

		snapshot.State = InspectorState::EntitySelection;
		World& world = scene->GetWorld();

		if (TagComponent* tag = world.GetComponent<TagComponent>(entity)) {
			snapshot.EntityData.HasTag = true;
			snapshot.EntityData.TagName = tag->Name;
		}
		if (TransformComponent* t = world.GetComponent<TransformComponent>(entity)) {
			snapshot.EntityData.HasTransform = true;
			snapshot.EntityData.Position = t->Position;
			snapshot.EntityData.Scale = t->Scale;
			snapshot.EntityData.RotationEulerDeg = glm::degrees(glm::eulerAngles(t->Rotation));
		}
		if (SpriteComponent* s = world.GetComponent<SpriteComponent>(entity)) {
			snapshot.EntityData.HasSprite = true;
			snapshot.EntityData.SpriteSize = s->Size;
			snapshot.EntityData.SpriteColor = s->Color;
		}
		if (MeshComponent* m = world.GetComponent<MeshComponent>(entity)) {
			snapshot.EntityData.HasMesh = true;
			snapshot.EntityData.MeshIndexCount = m->IndexCount;
			snapshot.EntityData.MeshColor = m->Color;
		}
		return snapshot;
	}

	void EditorPanelDataProvider::SelectEntity(uint64_t handle) {
		auto it = m_HandleToSceneEntity.find(handle);
		if (it == m_HandleToSceneEntity.end())
			return;
		Scene* scene = it->second.first;
		Entity entity = it->second.second;

		bool sceneActive = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { sceneActive = true; break; }
		}
		if (!sceneActive)
			return;
		EditorContext& ctx = EditorContext::Get();
		ctx.ClearSelectedAsset();
		ctx.SetSelectedEntity(scene, entity);
	}

	void EditorPanelDataProvider::SetEntityTag(const std::string& name) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene) return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active) return;
		Entity e = ctx.GetSelectedEntity();
		if (TagComponent* tag = scene->GetWorld().GetComponent<TagComponent>(e))
			tag->Name = name;
	}

	void EditorPanelDataProvider::SetEntityTransform(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rotationEulerDeg) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene) return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active) return;
		Entity e = ctx.GetSelectedEntity();
		if (TransformComponent* t = scene->GetWorld().GetComponent<TransformComponent>(e)) {
			t->SetPosition(pos);
			t->SetScale(scale);
			t->SetRotation(glm::radians(rotationEulerDeg));
		}
	}

	void EditorPanelDataProvider::SetEntitySprite(const glm::vec2& size, const glm::vec4& color) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene) return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active) return;
		Entity e = ctx.GetSelectedEntity();
		if (SpriteComponent* s = scene->GetWorld().GetComponent<SpriteComponent>(e)) {
			s->Size = size;
			s->Color = color;
		}
	}

	void EditorPanelDataProvider::SetEntityMeshColor(const glm::vec4& color) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene) return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active) return;
		Entity e = ctx.GetSelectedEntity();
		if (MeshComponent* m = scene->GetWorld().GetComponent<MeshComponent>(e))
			m->Color = color;
	}

} // namespace Ehu
