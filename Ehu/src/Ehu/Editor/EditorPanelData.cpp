#include "EditorPanelData.h"
#include "EditorContext.h"
#include "CreationRegistry.h"
#include "Core/Application.h"
#include "Project/Project.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include "Scripting/ScriptEngine.h"
#include <glm/gtc/quaternion.hpp>
#include <algorithm>

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

		EditorContext& ctx = EditorContext::Get();
		Scene* scene = ctx.GetActiveScene();
		if (!scene || !Application::Get().IsSceneActivated(scene))
			return snapshot;

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
		const bool sceneActive = Application::Get().IsSceneActivated(scene);
		const bool entityWorldValid = scene && sceneActive && scene->GetWorld().IsValid(entity);
		if (!entityWorldValid) {
			snapshot.State = InspectorState::SelectionInvalid;
			return snapshot;
		}

		snapshot.AllowComponentEdit = (scene == ctx.GetActiveScene() && scene->GetRuntimeState() == SceneRuntimeState::Edit);
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
			snapshot.EntityData.SpriteTexturePath = s->TexturePath;
			snapshot.EntityData.SpriteTilingFactor = s->TilingFactor;
		}
		if (CircleRendererComponent* cr = world.GetComponent<CircleRendererComponent>(entity)) {
			snapshot.EntityData.HasCircleRenderer = true;
			snapshot.EntityData.CircleColor = cr->Color;
			snapshot.EntityData.CircleThickness = cr->Thickness;
			snapshot.EntityData.CircleFade = cr->Fade;
		}
		if (TextComponent* tx = world.GetComponent<TextComponent>(entity)) {
			snapshot.EntityData.HasText = true;
			snapshot.EntityData.TextString = tx->TextString;
			snapshot.EntityData.TextFontPath = tx->FontPath;
			snapshot.EntityData.TextPixelHeight = tx->PixelHeight;
			snapshot.EntityData.TextKerning = tx->Kerning;
			snapshot.EntityData.TextLineSpacing = tx->LineSpacing;
			snapshot.EntityData.TextColor = tx->Color;
		}
		if (CameraComponent* cc = world.GetComponent<CameraComponent>(entity)) {
			snapshot.EntityData.HasCamera = true;
			snapshot.EntityData.CameraPrimary = cc->Primary;
			snapshot.EntityData.CameraFixedAspect = cc->FixedAspectRatio;
		}
		if (MeshComponent* m = world.GetComponent<MeshComponent>(entity)) {
			snapshot.EntityData.HasMesh = true;
			snapshot.EntityData.MeshIndexCount = m->IndexCount;
			snapshot.EntityData.MeshColor = m->Color;
		}
		if (RenderFilterComponent* renderFilter = world.GetComponent<RenderFilterComponent>(entity)) {
			snapshot.EntityData.HasRenderFilter = true;
			snapshot.EntityData.RenderChannelValue = renderFilter->Channel;
		}
		if (PhysicsFilterComponent* physicsFilter = world.GetComponent<PhysicsFilterComponent>(entity)) {
			snapshot.EntityData.HasPhysicsFilter = true;
			snapshot.EntityData.CollisionLayer = physicsFilter->CollisionLayer;
			snapshot.EntityData.CollisionMask = physicsFilter->CollisionMask;
		}
		if (ScriptComponent* sc = world.GetComponent<ScriptComponent>(entity)) {
			if (!sc->Instances.empty()) {
				snapshot.EntityData.HasScript = true;
				if (!Application::Get().IsPlayMode())
					ScriptEngine::InitializeScriptComponentFields(*sc);
				snapshot.EntityData.ScriptInstances.reserve(sc->Instances.size());
				for (const ScriptInstanceData& instance : sc->Instances) {
					if (instance.ClassName.empty())
						continue;
					InspectorScriptInstanceData uiInstance;
					uiInstance.InstanceId = instance.InstanceId;
					uiInstance.ClassName = instance.ClassName;
					uiInstance.Fields.reserve(instance.Fields.size());
					for (const auto& [fieldName, fieldValue] : instance.Fields)
						uiInstance.Fields.emplace_back(fieldName, fieldValue);
					std::sort(uiInstance.Fields.begin(), uiInstance.Fields.end(),
						[](const auto& a, const auto& b) { return a.first < b.first; });
					snapshot.EntityData.ScriptInstances.push_back(std::move(uiInstance));
				}
			}
		}
		return snapshot;
	}

	void EditorPanelDataProvider::SelectEntity(uint64_t handle) {
		auto it = m_HandleToSceneEntity.find(handle);
		if (it == m_HandleToSceneEntity.end())
			return;
		SelectEntity(it->second.first, it->second.second);
	}

	void EditorPanelDataProvider::SelectEntity(Scene* scene, Entity entity) {
		if (!scene)
			return;
		if (EditorContext::Get().GetActiveScene() != scene)
			return;
		if (!Application::Get().IsSceneActivated(scene) || !scene->GetWorld().IsValid(entity))
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

	void EditorPanelDataProvider::SetEntitySpriteTexture(const std::string& texturePath, float tiling) {
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
			s->TexturePath = texturePath;
			s->TilingFactor = tiling;
		}
	}

	void EditorPanelDataProvider::SetEntityCircleRenderer(const glm::vec4& color, float thickness, float fade) {
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
		if (CircleRendererComponent* c = scene->GetWorld().GetComponent<CircleRendererComponent>(e)) {
			c->Color = color;
			c->Thickness = thickness;
			c->Fade = fade;
		}
	}

	void EditorPanelDataProvider::SetEntityText(const std::string& text, const std::string& fontPath, float pixelHeight, float kerning, float lineSpacing, const glm::vec4& color) {
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
		if (TextComponent* t = scene->GetWorld().GetComponent<TextComponent>(e)) {
			t->TextString = text;
			t->FontPath = fontPath;
			t->PixelHeight = pixelHeight;
			t->Kerning = kerning;
			t->LineSpacing = lineSpacing;
			t->Color = color;
		}
	}

	void EditorPanelDataProvider::SetEntityCameraFlags(bool primary, bool fixedAspect) {
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
		if (CameraComponent* c = scene->GetWorld().GetComponent<CameraComponent>(e)) {
			c->Primary = primary;
			c->FixedAspectRatio = fixedAspect;
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

	void EditorPanelDataProvider::SetEntityRenderChannel(RenderChannelId channel) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		RenderFilterComponent* filter = scene->GetWorld().GetComponent<RenderFilterComponent>(e);
		if (!filter) {
			scene->GetWorld().AddComponent(e, RenderFilterComponent{ channel });
			return;
		}
		filter->Channel = channel;
	}

	void EditorPanelDataProvider::SetEntityPhysicsFilter(uint32_t collisionLayer, uint32_t collisionMask) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		PhysicsFilterComponent* filter = scene->GetWorld().GetComponent<PhysicsFilterComponent>(e);
		if (!filter) {
			scene->GetWorld().AddComponent(e, PhysicsFilterComponent{ collisionLayer, collisionMask });
			return;
		}
		filter->CollisionLayer = collisionLayer;
		filter->CollisionMask = collisionMask;
	}

	void EditorPanelDataProvider::AddEntityScriptInstance(const std::string& className) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection())
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		ScriptComponent* script = scene->GetWorld().GetComponent<ScriptComponent>(e);
		if (!script) {
			scene->GetWorld().AddComponent(e, ScriptComponent{});
			script = scene->GetWorld().GetComponent<ScriptComponent>(e);
		}
		if (!script || className.empty())
			return;

		ScriptInstanceData instance;
		instance.InstanceId = UUID();
		instance.ClassName = className;
		script->Instances.push_back(std::move(instance));
	}

	void EditorPanelDataProvider::RemoveEntityScriptInstance(UUID instanceId) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection() || instanceId.Raw() == 0)
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		ScriptComponent* script = scene->GetWorld().GetComponent<ScriptComponent>(e);
		if (!script)
			return;
		auto it = std::remove_if(script->Instances.begin(), script->Instances.end(), [&](const ScriptInstanceData& instance) {
			return instance.InstanceId == instanceId;
		});
		script->Instances.erase(it, script->Instances.end());
	}

	void EditorPanelDataProvider::MoveEntityScriptInstance(UUID instanceId, bool moveUp) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection() || instanceId.Raw() == 0)
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		ScriptComponent* script = scene->GetWorld().GetComponent<ScriptComponent>(e);
		if (!script)
			return;
		auto it = std::find_if(script->Instances.begin(), script->Instances.end(), [&](const ScriptInstanceData& instance) {
			return instance.InstanceId == instanceId;
		});
		if (it == script->Instances.end())
			return;
		const size_t index = static_cast<size_t>(std::distance(script->Instances.begin(), it));
		if (moveUp) {
			if (index == 0)
				return;
			std::swap(script->Instances[index], script->Instances[index - 1]);
		} else {
			if (index + 1 >= script->Instances.size())
				return;
			std::swap(script->Instances[index], script->Instances[index + 1]);
		}
	}

	void EditorPanelDataProvider::SetEntityScriptField(UUID instanceId, const std::string& fieldName, const ScriptFieldValue& value) {
		EditorContext& ctx = EditorContext::Get();
		if (!ctx.HasEntitySelection() || instanceId.Raw() == 0)
			return;
		Scene* scene = ctx.GetSelectedScene();
		if (!scene)
			return;
		bool active = false;
		for (Scene* s : Application::Get().GetActivatedScenes()) {
			if (s == scene) { active = true; break; }
		}
		if (!active)
			return;
		Entity e = ctx.GetSelectedEntity();
		ScriptComponent* script = scene->GetWorld().GetComponent<ScriptComponent>(e);
		if (!script)
			return;
		auto it = std::find_if(script->Instances.begin(), script->Instances.end(), [&](const ScriptInstanceData& instance) {
			return instance.InstanceId == instanceId;
		});
		if (it == script->Instances.end())
			return;
		it->Fields[fieldName] = value;
	}

	bool EditorPanelDataProvider::CreateEntity(const std::string& creatorId, Scene* scene, const std::string& name, RenderChannelId channel, uint32_t collisionLayer, uint32_t collisionMask) {
		if (!scene)
			return false;
		EntityCreateContext context;
		context.TargetScene = scene;
		context.Name = name;
		context.RenderChannel = channel;
		context.CollisionLayer = collisionLayer;
		context.CollisionMask = collisionMask;
		if (!CreationRegistry::CreateEntityById(creatorId, context))
			return false;

		EditorContext& editorContext = EditorContext::Get();
		editorContext.ClearSelectedAsset();
		editorContext.SetSelectedEntity(scene, context.CreatedEntity);
		return true;
	}

} // namespace Ehu
