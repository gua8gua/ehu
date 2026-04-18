#pragma once

#include "ECS/Components.h"
#include <functional>
#include <string>

namespace Ehu {

	class Scene;

	struct NewProjectRequest {
		std::string Directory;
		std::string Name;
	};

	struct CreateEntityRequest {
		std::string CreatorId;
		Scene* TargetScene = nullptr;
		std::string Name;
		RenderChannelId RenderChannel = BuiltinRenderChannels::Default;
		uint32_t CollisionLayer = 1u;
		uint32_t CollisionMask = 0xFFFFFFFFu;
	};

	class EditorModalHost {
	public:
		using CreateProjectHandler = std::function<bool(const NewProjectRequest&)>;
		using OpenProjectHandler = std::function<bool(const std::string&)>;
		using CreateEntityHandler = std::function<bool(const CreateEntityRequest&)>;

		void RequestNewProject() { m_ShowNewProjectModal = true; }
		void RequestOpenProject() { m_ShowOpenProjectModal = true; }
		void RequestCreateEntity() { m_ShowCreateEntityPopup = true; }
		void ShowError(const std::string& message);

		void OnImGuiRender(
			const CreateProjectHandler& createProject,
			const OpenProjectHandler& openProject,
			const CreateEntityHandler& createEntity,
			Scene* preferredEntityTargetScene);

	private:
		bool m_ShowNewProjectModal = false;
		bool m_ShowOpenProjectModal = false;
		bool m_ShowCreateEntityPopup = false;
		bool m_ShowErrorPopup = false;
		int m_SelectedEntityCreator = 0;
		int m_SelectedRenderChannel = 0;
		int m_SelectedCollisionLayer = 0;
		char m_NewProjectDirBuf[512] = "";
		char m_NewProjectNameBuf[256] = "MyGame";
		char m_OpenProjectPathBuf[512] = "";
		char m_NewEntityNameBuf[256] = "NewEntity";
		char m_NewRenderChannelNameBuf[128] = "";
		char m_NewCollisionLayerNameBuf[128] = "";
		std::string m_ErrorMessage;
	};

} // namespace Ehu
