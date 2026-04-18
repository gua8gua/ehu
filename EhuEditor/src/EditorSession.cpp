#include "EditorSession.h"
#include "Core/Application.h"
#include "Core/FileSystem.h"
#include "ECS/Components.h"
#include "Editor/EditorContext.h"
#include "Platform/IO/FileDialog.h"
#include "Project/Project.h"
#include "Project/ProjectConfig.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Scripting/ScriptEngine.h"
#include <filesystem>

namespace Ehu {

	namespace {

		Entity FindMatchingEntityByUUID(Scene* scene, UUID uuid) {
			if (!scene || uuid.Raw() == 0)
				return {};
			return scene->FindEntityByUUID(uuid);
		}

		UUID GetSelectedEntityUUID(Scene* scene, Entity entity) {
			if (!scene || !scene->GetWorld().IsValid(entity))
				return UUID(0);
			if (const IdComponent* id = scene->GetWorld().GetComponent<IdComponent>(entity))
				return id->Id;
			return UUID(0);
		}

	}

	bool EditorSession::TryOpenFirstRecentProject() {
		const int maxAttempts = 10;
		for (int attempt = 0; attempt < maxAttempts; ++attempt) {
			std::vector<std::string> recent = GetRecentProjects();
			if (recent.empty())
				return false;
			if (OpenProject(recent[0]))
				return true;
		}
		return false;
	}

	std::vector<std::string> EditorSession::GetRecentProjects() const {
		return Project::GetRecentProjects();
	}

	bool EditorSession::CreateProject(const std::string& projectDir, const std::string& projectName) {
		ClearError();
		EditorContext::Get().ClearAll();
		Application::Get().DeactivateAllScenes();
		Project::CloseActive();

		Ref<Project> project = Project::New(projectDir, projectName);
		if (!project) {
			SetError("创建项目失败");
			return false;
		}

		Project::AddToRecent(project->GetProjectFilePath());
		Application::Get().ConfigureProject(*project);
		return OpenScene(project->GetConfig().StartupScene);
	}

	bool EditorSession::OpenProject(const std::string& projectFilePath) {
		ClearError();
		StopScene();
		EditorContext::Get().ClearAll();
		Application::Get().DeactivateAllScenes();
		Project::CloseActive();

		Ref<Project> project = Project::Load(projectFilePath);
		if (!project) {
			Project::RemoveFromRecent(projectFilePath);
			SetError("项目文件无效，无法加载");
			return false;
		}

		Application::Get().ConfigureProject(*project);
		Project::AddToRecent(projectFilePath);

		std::string targetScene = project->GetConfig().StartupScene;
		if (targetScene.empty() && !project->GetConfig().Scenes.empty())
			targetScene = project->GetConfig().Scenes.front().RelativePath;
		if (targetScene.empty())
			return NewScene();
		return OpenScene(targetScene);
	}

	void EditorSession::CloseProject() {
		StopScene();
		m_EditorScene.reset();
		m_RuntimeScene.reset();
		m_EditorScenePath.clear();
		m_SceneState = EditorSceneState::Edit;
		EditorContext::Get().ClearAll();
		Application::Get().DeactivateAllScenes();
		Project::CloseActive();
		ClearError();
	}

	bool EditorSession::NewScene() {
		ClearError();
		if (!Project::GetActive()) {
			SetError("未选择项目");
			return false;
		}

		StopScene();
		Ref<Scene> scene = CreateRef<Scene>();
		Entity root = scene->CreateEntity();
		scene->GetWorld().AddComponent(root, TransformComponent{});
		if (TagComponent* tag = scene->GetWorld().GetComponent<TagComponent>(root))
			tag->Name = "Root";

		return SetEditingScene(scene, "");
	}

	bool EditorSession::OpenSceneDialog() {
		Ref<Project> project = Project::GetActive();
		if (!project) {
			SetError("未选择项目");
			return false;
		}

		std::string chosen;
		if (!FileDialog::OpenFile("Open Ehu Scene", "Ehu Scene (*.ehuscene)", "*.ehuscene", chosen))
			return true;

		const std::string relativePath = MakeProjectRelativeScenePath(chosen);
		if (relativePath.empty()) {
			SetError("所选场景不在当前项目 Assets 目录内");
			return false;
		}
		return OpenScene(relativePath);
	}

	bool EditorSession::OpenScene(const std::string& sceneRelativePath) {
		ClearError();
		Ref<Project> project = Project::GetActive();
		if (!project || sceneRelativePath.empty()) {
			SetError("未选择场景或项目");
			return false;
		}

		StopScene();
		Application::Get().DeactivateAllScenes();
		Application::Get().ConfigureProject(*project);
		Ref<Scene> scene = Application::Get().LoadSceneFromProject(*project, sceneRelativePath, true);
		if (!scene) {
			SetError("场景加载失败");
			return false;
		}

		SyncProjectStartupScene(sceneRelativePath);
		Project::SaveActive();
		return SetEditingScene(scene, sceneRelativePath);
	}

	bool EditorSession::SaveScene() {
		ClearError();
		if (!m_EditorScene) {
			SetError("没有可保存的场景");
			return false;
		}
		if (m_EditorScenePath.empty())
			return SaveSceneAs();
		return SaveSceneToPath(ResolveSceneAbsolutePath(m_EditorScenePath));
	}

	bool EditorSession::SaveSceneAs() {
		ClearError();
		Ref<Project> project = Project::GetActive();
		if (!project || !m_EditorScene) {
			SetError("没有可保存的项目或场景");
			return false;
		}

		std::string chosen;
		if (!FileDialog::SaveFile("Save Ehu Scene", "Ehu Scene (*.ehuscene)", "*.ehuscene", chosen))
			return true;
		if (FileSystem::GetExtension(chosen).empty())
			chosen += ".ehuscene";
		return SaveSceneToPath(chosen);
	}

	bool EditorSession::SaveSceneToPath(const std::string& absoluteScenePath) {
		ClearError();
		Ref<Project> project = Project::GetActive();
		if (!project || !m_EditorScene || absoluteScenePath.empty()) {
			SetError("保存场景参数无效");
			return false;
		}

		const std::string relativePath = MakeProjectRelativeScenePath(absoluteScenePath);
		if (relativePath.empty()) {
			SetError("保存路径必须位于项目 Assets 目录内");
			return false;
		}

		SceneSerializer serializer;
		if (!serializer.Serialize(m_EditorScene.get(), absoluteScenePath)) {
			SetError("场景序列化失败");
			return false;
		}

		SyncProjectStartupScene(relativePath);
		EditorContext::Get().SetActiveScene(m_EditorScene.get(), relativePath);
		m_EditorScenePath = relativePath;
		if (!Project::SaveActive()) {
			SetError("项目配置保存失败");
			return false;
		}
		return true;
	}

	bool EditorSession::BeginPlayMode() {
		ClearError();
		if (IsPlaying())
			return true;
		return ActivateRuntimeScene(false);
	}

	bool EditorSession::BeginSimulationMode() {
		ClearError();
		if (IsSimulating())
			return true;
		return ActivateRuntimeScene(true);
	}

	void EditorSession::StopScene() {
		if (m_SceneState == EditorSceneState::Edit)
			return;

		const UUID selectedId = CaptureSelectedEntityUUID();
		Application::Get().SetPlayMode(false);
		Application::Get().DeactivateAllScenes();
		if (Project::GetActive())
			Application::Get().ConfigureProject(*Project::GetActive());
		if (m_EditorScene)
			Application::Get().RegisterScene(m_EditorScene, true);
		m_RuntimeScene.reset();
		m_SceneState = EditorSceneState::Edit;
		EditorContext::Get().SetActiveScene(m_EditorScene.get(), m_EditorScenePath);
		RestoreSelectedEntity(m_EditorScene.get(), selectedId);
	}

	bool EditorSession::ReloadScriptAssemblies() {
		ClearError();
		Ref<Project> project = Project::GetActive();
		if (!project) {
			SetError("未选择项目");
			return false;
		}

		if (m_SceneState != EditorSceneState::Edit)
			StopScene();

		const ProjectConfig& config = project->GetConfig();
		const std::string coreAssembly = ResolveProjectRelativePath(*project, config.ScriptCoreAssemblyPath);
		const std::string appAssembly = ResolveProjectRelativePath(*project, config.ScriptAppAssemblyPath);
		if (!ScriptEngine::ReloadAssemblies(coreAssembly, appAssembly)) {
			SetError("脚本程序集重载失败");
			return false;
		}
		return true;
	}

	bool EditorSession::HasProject() const {
		return Project::GetActive() != nullptr;
	}

	bool EditorSession::HasScene() const {
		return m_EditorScene != nullptr || m_RuntimeScene != nullptr;
	}

	Ref<Project> EditorSession::GetProject() const {
		return Project::GetActive();
	}

	Scene* EditorSession::GetActiveScene() const {
		switch (m_SceneState) {
		case EditorSceneState::Play:
		case EditorSceneState::Simulate:
			return m_RuntimeScene.get();
		case EditorSceneState::Edit:
		default:
			return m_EditorScene.get();
		}
	}

	Scene* EditorSession::GetEntityCreationTargetScene() const {
		EditorContext& context = EditorContext::Get();
		if (context.HasActiveScene())
			return context.GetActiveScene();
		if (context.HasEntitySelection())
			return context.GetSelectedScene();
		return GetActiveScene();
	}

	void EditorSession::SetError(const std::string& errorMessage) {
		m_LastError = errorMessage;
	}

	void EditorSession::ClearError() {
		m_LastError.clear();
	}

	bool EditorSession::SetEditingScene(Ref<Scene> scene, const std::string& sceneRelativePath) {
		if (!scene) {
			SetError("场景无效");
			return false;
		}

		m_EditorScene = scene;
		m_EditorScenePath = sceneRelativePath;
		m_RuntimeScene.reset();
		m_SceneState = EditorSceneState::Edit;

		EditorContext::Get().ClearSelectedAsset();
		EditorContext::Get().SetActiveScene(scene.get(), sceneRelativePath);

		return ActivateEditingScene();
	}

	void EditorSession::SyncProjectStartupScene(const std::string& sceneRelativePath) {
		Ref<Project> project = Project::GetActive();
		if (!project || sceneRelativePath.empty())
			return;

		ProjectConfig& config = project->GetConfig();
		auto it = std::find_if(config.Scenes.begin(), config.Scenes.end(), [&](const ProjectSceneEntry& entry) {
			return entry.RelativePath == sceneRelativePath;
		});
		if (it == config.Scenes.end())
			config.Scenes.push_back({ sceneRelativePath, true });
		else
			it->Active = true;
		config.StartupScene = sceneRelativePath;
	}

	std::string EditorSession::ResolveSceneAbsolutePath(const std::string& sceneRelativePath) const {
		Ref<Project> project = Project::GetActive();
		if (!project || sceneRelativePath.empty())
			return {};
		return project->GetAssetFileSystemPath(sceneRelativePath);
	}

	std::string EditorSession::MakeProjectRelativeScenePath(const std::string& absoluteScenePath) const {
		Ref<Project> project = Project::GetActive();
		if (!project || absoluteScenePath.empty())
			return {};

		std::error_code ec;
		std::filesystem::path assetDir(project->GetAssetDirectory());
		std::filesystem::path absolutePath(absoluteScenePath);
		absolutePath = std::filesystem::absolute(absolutePath, ec);
		if (ec)
			return {};
		std::filesystem::path relative = std::filesystem::relative(absolutePath, assetDir, ec);
		if (ec)
			return {};
		const std::string relativeString = relative.lexically_normal().generic_string();
		if (relativeString.empty() || relativeString.rfind("..", 0) == 0)
			return {};
		return relativeString;
	}

	std::string EditorSession::ResolveProjectRelativePath(const Project& project, const std::string& path) const {
		if (path.empty())
			return {};
		std::filesystem::path projectPath(project.GetProjectDirectory());
		std::filesystem::path candidate(path);
		if (candidate.is_absolute())
			return candidate.lexically_normal().string();
		return (projectPath / candidate).lexically_normal().string();
	}

	UUID EditorSession::CaptureSelectedEntityUUID() const {
		const EditorContext& context = EditorContext::Get();
		return GetSelectedEntityUUID(context.GetSelectedScene(), context.GetSelectedEntity());
	}

	void EditorSession::RestoreSelectedEntity(Scene* scene, UUID entityId) const {
		if (!scene || entityId.Raw() == 0) {
			EditorContext::Get().ClearSelectedEntity();
			return;
		}

		Entity entity = FindMatchingEntityByUUID(scene, entityId);
		if (scene->GetWorld().IsValid(entity))
			EditorContext::Get().SetSelectedEntity(scene, entity);
		else
			EditorContext::Get().ClearSelectedEntity();
	}

	bool EditorSession::ActivateEditingScene() {
		if (!m_EditorScene)
			return false;

		Application::Get().DeactivateAllScenes();
		if (Project::GetActive())
			Application::Get().ConfigureProject(*Project::GetActive());
		Application::Get().RegisterScene(m_EditorScene, true);
		EditorContext::Get().SetActiveScene(m_EditorScene.get(), m_EditorScenePath);
		return true;
	}

	bool EditorSession::ActivateRuntimeScene(bool simulationMode) {
		if (!Project::GetActive() || !m_EditorScene) {
			SetError("缺少项目或编辑场景");
			return false;
		}

		const UUID selectedId = CaptureSelectedEntityUUID();
		m_RuntimeScene = Scene::Copy(*m_EditorScene);
		Application::Get().DeactivateAllScenes();
		Application::Get().ConfigureProject(*Project::GetActive());
		Application::Get().RegisterScene(m_RuntimeScene, true);
		Application::Get().SetPlayMode(true);
		if (simulationMode)
			m_RuntimeScene->OnSimulationStart();
		m_SceneState = simulationMode ? EditorSceneState::Simulate : EditorSceneState::Play;
		EditorContext::Get().SetActiveScene(m_RuntimeScene.get(), m_EditorScenePath);
		RestoreSelectedEntity(m_RuntimeScene.get(), selectedId);
		return true;
	}

} // namespace Ehu
