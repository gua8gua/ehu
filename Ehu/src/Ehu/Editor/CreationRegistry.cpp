#include "ehupch.h"
#include "CreationRegistry.h"
#include "Core/FileSystem.h"
#include "Project/Project.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Renderer/Camera/Camera.h"
#include "ECS/World.h"
#include <unordered_map>
#include <algorithm>

namespace Ehu {

	namespace {
		struct CreatorState {
			std::unordered_map<std::string, AssetCreatorEntry> AssetCreators;
			std::unordered_map<std::string, EntityCreatorEntry> EntityCreators;
			bool DefaultsRegistered = false;
		};

		CreatorState& GetState() {
			static CreatorState s_State;
			return s_State;
		}

		std::string JoinAssetPath(Project& project, const std::string& relativeDirectory, const std::string& name) {
			const std::string relativePath = relativeDirectory.empty() ? name : FileSystem::Join(relativeDirectory, name);
			return project.GetAssetFileSystemPath(relativePath);
		}

		bool CreateDirectoryAsset(AssetCreateContext& context) {
			if (context.Name.empty()) {
				context.ErrorMessage = "目录名称不能为空";
				return false;
			}
			return FileSystem::CreateDirectory(JoinAssetPath(context.TargetProject, context.RelativeDirectory, context.Name));
		}

		bool CreateEmptyFileAsset(AssetCreateContext& context) {
			if (context.Name.empty()) {
				context.ErrorMessage = "文件名不能为空";
				return false;
			}
			return FileSystem::WriteTextFile(JoinAssetPath(context.TargetProject, context.RelativeDirectory, context.Name), "");
		}

		bool CreateSceneAsset(AssetCreateContext& context) {
			if (context.Name.empty()) {
				context.ErrorMessage = "场景文件名不能为空";
				return false;
			}

			std::string fileName = context.Name;
			if (FileSystem::GetExtension(fileName).empty())
				fileName += ".ehuscene";

			Scene scene;
			Entity root = scene.CreateEntity();
			scene.GetWorld().AddComponent(root, TransformComponent{});
			if (TagComponent* tag = scene.GetWorld().GetComponent<TagComponent>(root))
				tag->Name = "Root";
			SceneSerializer serializer;
			return serializer.Serialize(&scene, JoinAssetPath(context.TargetProject, context.RelativeDirectory, fileName));
		}

		bool CreateScriptTemplate(AssetCreateContext& context) {
			if (context.Name.empty()) {
				context.ErrorMessage = "脚本名不能为空";
				return false;
			}

			std::string className = context.Name;
			if (FileSystem::GetExtension(className).empty())
				className += ".cs";
			std::string pureClassName = FileSystem::GetFileName(className);
			size_t dotPos = pureClassName.rfind('.');
			if (dotPos != std::string::npos)
				pureClassName = pureClassName.substr(0, dotPos);

			const std::string content =
				"using Ehu;\n\n"
				"namespace Game {\n"
				"    /// <summary>ScriptComponent.Instances[n].ClassName = \"Game." + pureClassName + "\"</summary>\n"
				"    public class " + pureClassName + " {\n"
				"        public void OnCreate() {\n"
				"        }\n\n"
				"        public void OnUpdate(float dt) {\n"
				"        }\n\n"
				"        public void OnPhysicsUpdate(float fixedDt) {\n"
				"        }\n"
				"    }\n"
				"}\n";
			return FileSystem::WriteTextFile(JoinAssetPath(context.TargetProject, context.RelativeDirectory, className), content);
		}

		bool CreateMaterialTemplate(AssetCreateContext& context) {
			if (context.Name.empty()) {
				context.ErrorMessage = "材质名不能为空";
				return false;
			}
			std::string fileName = context.Name;
			if (FileSystem::GetExtension(fileName).empty())
				fileName += ".ehumat";
			const std::string content =
				"{\n"
				"  \"name\": \"" + context.Name + "\",\n"
				"  \"shader\": \"Default\",\n"
				"  \"color\": [1.0, 1.0, 1.0, 1.0]\n"
				"}\n";
			return FileSystem::WriteTextFile(JoinAssetPath(context.TargetProject, context.RelativeDirectory, fileName), content);
		}

		void ApplyCommonFilters(EntityCreateContext& context) {
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, RenderFilterComponent{ context.RenderChannel });
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, PhysicsFilterComponent{ context.CollisionLayer, context.CollisionMask });
		}

		bool CreateEmptyEntity(EntityCreateContext& context) {
			if (!context.TargetScene) {
				context.ErrorMessage = "无目标场景";
				return false;
			}
			context.CreatedEntity = context.TargetScene->CreateEntity();
			if (TagComponent* tag = context.TargetScene->GetWorld().GetComponent<TagComponent>(context.CreatedEntity))
				tag->Name = context.Name.empty() ? "Entity" : context.Name;
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, TransformComponent{});
			ApplyCommonFilters(context);
			return true;
		}

		bool CreateCameraEntity(EntityCreateContext& context) {
			if (!CreateEmptyEntity(context))
				return false;
			Camera* camera = context.TargetScene->AddOwnedCamera(CreateScope<PerspectiveCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f));
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, CameraComponent{ camera });
			return true;
		}

		bool CreateSpriteEntity(EntityCreateContext& context) {
			if (!CreateEmptyEntity(context))
				return false;
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, SpriteComponent{});
			return true;
		}

		bool CreateMeshEntity(EntityCreateContext& context) {
			if (!CreateEmptyEntity(context))
				return false;
			context.TargetScene->GetWorld().AddComponent(context.CreatedEntity, MeshComponent{});
			return true;
		}
	}

	void CreationRegistry::EnsureDefaultCreatorsRegistered() {
		CreatorState& state = GetState();
		if (state.DefaultsRegistered)
			return;
		state.DefaultsRegistered = true;

		RegisterAssetCreator({ "folder", "Folder", "Basic", &CreateDirectoryAsset });
		RegisterAssetCreator({ "file", "Empty File", "Basic", &CreateEmptyFileAsset });
		RegisterAssetCreator({ "scene", "Scene", "Basic", &CreateSceneAsset });
		RegisterAssetCreator({ "script_cs", "C# Script", "Templates", &CreateScriptTemplate });
		RegisterAssetCreator({ "material", "Material", "Templates", &CreateMaterialTemplate });

		RegisterEntityCreator({ "entity_empty", "Empty Entity", "Basic", &CreateEmptyEntity });
		RegisterEntityCreator({ "entity_camera", "Camera", "Basic", &CreateCameraEntity });
		RegisterEntityCreator({ "entity_sprite", "Sprite", "Basic", &CreateSpriteEntity });
		RegisterEntityCreator({ "entity_mesh", "Mesh", "Basic", &CreateMeshEntity });
	}

	void CreationRegistry::RegisterAssetCreator(const AssetCreatorEntry& entry) {
		if (entry.Id.empty() || !entry.CreateFn)
			return;
		GetState().AssetCreators[entry.Id] = entry;
	}

	void CreationRegistry::RegisterEntityCreator(const EntityCreatorEntry& entry) {
		if (entry.Id.empty() || !entry.CreateFn)
			return;
		GetState().EntityCreators[entry.Id] = entry;
	}

	std::vector<AssetCreatorEntry> CreationRegistry::GetAssetCreators() {
		EnsureDefaultCreatorsRegistered();
		std::vector<AssetCreatorEntry> result;
		for (const auto& [id, entry] : GetState().AssetCreators)
			result.push_back(entry);
		std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.DisplayName < b.DisplayName; });
		return result;
	}

	std::vector<EntityCreatorEntry> CreationRegistry::GetEntityCreators() {
		EnsureDefaultCreatorsRegistered();
		std::vector<EntityCreatorEntry> result;
		for (const auto& [id, entry] : GetState().EntityCreators)
			result.push_back(entry);
		std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.DisplayName < b.DisplayName; });
		return result;
	}

	bool CreationRegistry::CreateAssetById(const std::string& id, AssetCreateContext& context) {
		EnsureDefaultCreatorsRegistered();
		auto it = GetState().AssetCreators.find(id);
		if (it == GetState().AssetCreators.end()) {
			context.ErrorMessage = "未找到创建器: " + id;
			return false;
		}
		return it->second.CreateFn(context);
	}

	bool CreationRegistry::CreateEntityById(const std::string& id, EntityCreateContext& context) {
		EnsureDefaultCreatorsRegistered();
		auto it = GetState().EntityCreators.find(id);
		if (it == GetState().EntityCreators.end()) {
			context.ErrorMessage = "未找到实体创建器: " + id;
			return false;
		}
		return it->second.CreateFn(context);
	}

} // namespace Ehu
