#pragma once

#include "Core/Core.h"
#include "ECS/Entity.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Ehu {

	class Scene;
	class World;

	/// 层级条目：供 Hierarchy 面板展示
	struct EHU_API HierarchyEntry {
		std::string Label;
		uint64_t Handle = 0;
		bool IsSelected = false;
	};

	/// Hierarchy 快照：后端经校验的数据
	struct EHU_API HierarchySnapshot {
		std::vector<HierarchyEntry> Entries;
		bool HasProject = false;
	};

	/// Inspector 状态
	enum class EHU_API InspectorState {
		NoSelection,
		AssetSelection,
		EntitySelection,
		SelectionInvalid,
	};

	/// 实体组件值副本：供 Inspector 展示与编辑
	struct EHU_API InspectorEntityData {
		bool HasTag = false;
		std::string TagName;

		bool HasTransform = false;
		glm::vec3 Position{ 0 };
		glm::vec3 Scale{ 1 };
		glm::vec3 RotationEulerDeg{ 0 };

		bool HasSprite = false;
		glm::vec2 SpriteSize{ 1, 1 };
		glm::vec4 SpriteColor{ 1, 1, 1, 1 };

		bool HasMesh = false;
		uint32_t MeshIndexCount = 0;
		glm::vec4 MeshColor{ 1, 1, 1, 1 };
	};

	/// Inspector 快照：后端经校验的数据
	struct EHU_API InspectorSnapshot {
		InspectorState State = InspectorState::NoSelection;
		std::string AssetPath;
		InspectorEntityData EntityData;
	};

	/// 面板数据提供者：后端，校验数据、处理边界情况，面板仅消费快照
	class EHU_API EditorPanelDataProvider {
	public:
		static EditorPanelDataProvider& Get();

		HierarchySnapshot GetHierarchySnapshot();
		InspectorSnapshot GetInspectorSnapshot();

		void SelectEntity(uint64_t handle);
		void SetEntityTag(const std::string& name);
		void SetEntityTransform(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rotationEulerDeg);
		void SetEntitySprite(const glm::vec2& size, const glm::vec4& color);
		void SetEntityMeshColor(const glm::vec4& color);

	private:
		EditorPanelDataProvider() = default;
		uint64_t m_NextHandle = 1;
		std::unordered_map<uint64_t, std::pair<Scene*, Entity>> m_HandleToSceneEntity;
	};

} // namespace Ehu
