#pragma once

#include "Core/Ref.h"
#include "Core/UUID.h"
#include <string>
#include <vector>

namespace Ehu {

	class Project;
	class Scene;

	enum class EditorSceneState {
		Edit = 0,
		Play,
		Simulate
	};

	class EditorSession {
	public:
		bool TryOpenFirstRecentProject();
		std::vector<std::string> GetRecentProjects() const;

		bool CreateProject(const std::string& projectDir, const std::string& projectName);
		bool OpenProject(const std::string& projectFilePath);
		void CloseProject();

		bool NewScene();
		bool OpenSceneDialog();
		bool OpenScene(const std::string& sceneRelativePath);
		bool SaveScene();
		bool SaveSceneAs();
		bool SaveSceneToPath(const std::string& absoluteScenePath);

		bool BeginPlayMode();
		bool BeginSimulationMode();
		void StopScene();
		bool ReloadScriptAssemblies();

		bool HasProject() const;
		bool HasScene() const;
		bool HasActiveScene() const { return GetActiveScene() != nullptr; }
		Ref<Project> GetProject() const;
		Scene* GetEditorScene() const { return m_EditorScene.get(); }
		Scene* GetRuntimeScene() const { return m_RuntimeScene.get(); }
		Scene* GetActiveScene() const;
		Scene* GetEntityCreationTargetScene() const;
		const std::string& GetScenePath() const { return m_EditorScenePath; }
		EditorSceneState GetSceneState() const { return m_SceneState; }
		bool IsEditing() const { return m_SceneState == EditorSceneState::Edit; }
		bool IsPlaying() const { return m_SceneState == EditorSceneState::Play; }
		bool IsSimulating() const { return m_SceneState == EditorSceneState::Simulate; }
		const std::string& GetLastError() const { return m_LastError; }

	private:
		void SetError(const std::string& errorMessage);
		void ClearError();
		bool SetEditingScene(Ref<Scene> scene, const std::string& sceneRelativePath);
		void SyncProjectStartupScene(const std::string& sceneRelativePath);
		std::string ResolveSceneAbsolutePath(const std::string& sceneRelativePath) const;
		std::string MakeProjectRelativeScenePath(const std::string& absoluteScenePath) const;
		std::string ResolveProjectRelativePath(const Project& project, const std::string& path) const;
		UUID CaptureSelectedEntityUUID() const;
		void RestoreSelectedEntity(Scene* scene, UUID entityId) const;
		bool ActivateEditingScene();
		bool ActivateRuntimeScene(bool simulationMode);

	private:
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		std::string m_EditorScenePath;
		EditorSceneState m_SceneState = EditorSceneState::Edit;
		std::string m_LastError;
	};

} // namespace Ehu
