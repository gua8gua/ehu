#include "EditorContext.h"
// Scene 仅作指针使用，头文件已前向声明，无需包含 Scene.h（避免间接拉取 Camera 完整类型）

namespace Ehu {

	EditorContext& EditorContext::Get() {
		static EditorContext s_Instance;
		return s_Instance;
	}

	void EditorContext::SetSelectedEntity(Scene* scene, Entity entity) {
		m_SelectedScene = scene;
		m_SelectedEntity = entity;
	}

	void EditorContext::ClearSelectedEntity() {
		m_SelectedScene = nullptr;
		m_SelectedEntity = { 0, 0 };
	}

	void EditorContext::SetActiveScene(Scene* scene, const std::string& relativePath) {
		m_ActiveScene = scene;
		m_ActiveScenePath = relativePath;
		if (m_SelectedScene && m_SelectedScene != m_ActiveScene)
			ClearSelectedEntity();
	}

	void EditorContext::ClearActiveScene() {
		m_ActiveScene = nullptr;
		m_ActiveScenePath.clear();
		if (m_SelectedScene)
			ClearSelectedEntity();
	}

	void EditorContext::SetSelectedAsset(const std::string& relativePath) {
		m_SelectedAsset = relativePath;
	}

	void EditorContext::ClearSelectedAsset() {
		m_SelectedAsset.clear();
	}

	void EditorContext::ClearAll() {
		ClearActiveScene();
		ClearSelectedEntity();
		ClearSelectedAsset();
	}

} // namespace Ehu
