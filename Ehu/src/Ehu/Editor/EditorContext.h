#pragma once

#include "Core/Core.h"
#include "ECS/Entity.h"
#include <string>

namespace Ehu {

	class Scene;

	/// 选择/编辑上下文：维护当前选中的实体与资产，供编辑器面板读写；引擎侧无 ImGui 依赖
	class EHU_API EditorContext {
	public:
		static EditorContext& Get();

		/// 实体选择
		void SetSelectedEntity(Scene* scene, Entity entity);
		void ClearSelectedEntity();
		Scene* GetSelectedScene() const { return m_SelectedScene; }
		Entity GetSelectedEntity() const { return m_SelectedEntity; }
		bool HasEntitySelection() const { return m_SelectedScene != nullptr && m_SelectedEntity.id != 0; }

		/// 资产选择（相对路径，如 "Textures/foo.png"）
		void SetSelectedAsset(const std::string& relativePath);
		void ClearSelectedAsset();
		const std::string& GetSelectedAsset() const { return m_SelectedAsset; }
		bool HasAssetSelection() const { return !m_SelectedAsset.empty(); }

		/// 清除所有选择
		void ClearAll();

	private:
		EditorContext() = default;

		Scene* m_SelectedScene = nullptr;
		Entity m_SelectedEntity = { 0, 0 };
		std::string m_SelectedAsset;
	};

} // namespace Ehu
