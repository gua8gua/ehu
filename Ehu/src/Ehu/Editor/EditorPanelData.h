#pragma once

#include "Core/Core.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
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
	struct EHU_API InspectorScriptInstanceData {
		UUID InstanceId = UUID(0);
		std::string ClassName;
		std::vector<std::pair<std::string, ScriptFieldValue>> Fields;
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
		std::string SpriteTexturePath;
		float SpriteTilingFactor = 1.0f;

		bool HasCircleRenderer = false;
		glm::vec4 CircleColor{ 1, 1, 1, 1 };
		float CircleThickness = 0.05f;
		float CircleFade = 0.005f;

		bool HasText = false;
		std::string TextString;
		std::string TextFontPath;
		float TextPixelHeight = 32.0f;
		float TextKerning = 0.0f;
		float TextLineSpacing = 0.0f;
		glm::vec4 TextColor{ 1, 1, 1, 1 };

		bool HasCamera = false;
		bool CameraPrimary = true;
		bool CameraFixedAspect = false;

		bool HasMesh = false;
		uint32_t MeshIndexCount = 0;
		glm::vec4 MeshColor{ 1, 1, 1, 1 };

		bool HasRenderFilter = false;
		RenderChannelId RenderChannelValue = BuiltinRenderChannels::Default;

		bool HasPhysicsFilter = false;
		uint32_t CollisionLayer = 1u;
		uint32_t CollisionMask = 0xFFFFFFFFu;

		bool HasScript = false;
		std::vector<InspectorScriptInstanceData> ScriptInstances;
	};

	/// Inspector 快照：后端经校验的数据
	struct EHU_API InspectorSnapshot {
		InspectorState State = InspectorState::NoSelection;
		std::string AssetPath;
		InspectorEntityData EntityData;
		/// 仅编辑模式（非 Play/Simulate 运行时）下允许修改组件
		bool AllowComponentEdit = true;
	};

	/// 面板数据提供者：后端，校验数据、处理边界情况，面板仅消费快照
	class EHU_API EditorPanelDataProvider {
	public:
		static EditorPanelDataProvider& Get();

		HierarchySnapshot GetHierarchySnapshot();
		InspectorSnapshot GetInspectorSnapshot();

		void SelectEntity(uint64_t handle);
		void SelectEntity(Scene* scene, Entity entity);
		void SetEntityTag(const std::string& name);
		void SetEntityTransform(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rotationEulerDeg);
		void SetEntitySprite(const glm::vec2& size, const glm::vec4& color);
		void SetEntitySpriteTexture(const std::string& texturePath, float tiling);
		void SetEntityCircleRenderer(const glm::vec4& color, float thickness, float fade);
		void SetEntityText(const std::string& text, const std::string& fontPath, float pixelHeight, float kerning, float lineSpacing, const glm::vec4& color);
		void SetEntityCameraFlags(bool primary, bool fixedAspect);
		void SetEntityMeshColor(const glm::vec4& color);
		void SetEntityRenderChannel(RenderChannelId channel);
		void SetEntityPhysicsFilter(uint32_t collisionLayer, uint32_t collisionMask);
		void AddEntityScriptInstance(const std::string& className);
		void RemoveEntityScriptInstance(UUID instanceId);
		void MoveEntityScriptInstance(UUID instanceId, bool moveUp);
		void SetEntityScriptField(UUID instanceId, const std::string& fieldName, const ScriptFieldValue& value);
		bool CreateEntity(const std::string& creatorId, Scene* scene, const std::string& name, RenderChannelId channel, uint32_t collisionLayer, uint32_t collisionMask);

	private:
		EditorPanelDataProvider() = default;
		uint64_t m_NextHandle = 1;
		std::unordered_map<uint64_t, std::pair<Scene*, Entity>> m_HandleToSceneEntity;
	};

} // namespace Ehu
