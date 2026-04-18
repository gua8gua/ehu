#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Core/TimeStep.h"
#include "Core/UUID.h"
#include "Core/Ref.h"
#include "ECS/Entity.h"
#include "ECS/World.h"
#include "Renderer/Camera/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class b2World;

namespace Ehu {

	class Camera;
	class IScriptRuntime;
	class SceneSerializer;

	enum class EHU_API SceneRuntimeState : uint8_t {
		Edit = 0,
		Runtime,
		Simulation
	};

	/// 场景图节点：层次结构、变换传播（仅数据，平台无关）
	class EHU_API SceneNode {
	public:
		SceneNode(const std::string& name = "Node");
		virtual ~SceneNode() = default;

		void AddChild(SceneNode* child);
		void RemoveChild(SceneNode* child);
		const std::vector<SceneNode*>& GetChildren() const { return m_Children; }

		void SetPosition(const glm::vec3& pos) { m_Position = pos; m_TransformDirty = true; }
		void SetRotation(const glm::vec3& euler) { m_Rotation = glm::quat(glm::radians(euler)); m_TransformDirty = true; }
		void SetScale(const glm::vec3& scale) { m_Scale = scale; m_TransformDirty = true; }
		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::quat& GetRotation() const { return m_Rotation; }
		const glm::vec3& GetScale() const { return m_Scale; }

		const glm::mat4& GetLocalTransform() const;
		glm::mat4 GetWorldTransform() const;

		const std::string& GetName() const { return m_Name; }
		SceneNode* GetParent() const { return m_Parent; }

	protected:
		std::string m_Name;
		SceneNode* m_Parent = nullptr;
		std::vector<SceneNode*> m_Children;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };

		mutable glm::mat4 m_LocalTransform = glm::mat4(1.0f);
		mutable bool m_TransformDirty = true;
	};

	/// 场景：持有 ECS World，管理实体与主相机；提交由 SceneLayer 通过 World 查询完成
	class EHU_API Scene {
	public:
		Scene() = default;
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		static Ref<Scene> Copy(const Scene& other);

		/// 场景图根节点（当前仅用于运行时层次/变换，SceneSerializer 不序列化场景图）
		SceneNode* GetRoot() { return &m_Root; }
		const SceneNode* GetRoot() const { return &m_Root; }

		World& GetWorld() { return m_World; }
		const World& GetWorld() const { return m_World; }

		Entity CreateEntity();
		void DestroyEntity(Entity e);
		std::vector<Entity> GetEntities() const { return m_World.GetEntities(); }

		/// 按 TagComponent.Name 查找第一个匹配实体，未找到返回无效 Entity（id==0）
		Entity FindEntityByName(const std::string& name) const;
		/// 按 IdComponent.Id 查找实体，未找到返回无效 Entity
		Entity FindEntityByUUID(UUID uuid) const;
		/// 按 ECS Entity::id 查找当前场景中的实体（用于 GPU 拾取等）
		Entity FindEntityByEntityId(uint32_t entityId) const;
		/// 复制实体及其可拷贝组件（Id 为新 UUID，Tag 可加后缀；不复制 CameraComponent）
		Entity DuplicateEntity(Entity e);

		void SetMainCamera(Entity e) { m_MainCameraEntity = e; }
		Entity GetMainCameraEntity() const { return m_MainCameraEntity; }
		Camera* GetMainCamera() const;

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulationStart();
		void OnSimulationStop();
		void OnViewportResize(uint32_t width, uint32_t height);
		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsPaused() const { return m_IsPaused; }
		bool IsRunning() const { return m_RuntimeState == SceneRuntimeState::Runtime; }
		bool IsSimulating() const { return m_RuntimeState == SceneRuntimeState::Simulation; }
		SceneRuntimeState GetRuntimeState() const { return m_RuntimeState; }
		void Step(uint32_t frames = 1) { m_StepFrames += static_cast<int>(frames); }

		/// 逻辑 Tick：根据当前 SceneRuntimeState 调度 Runtime/Simulation/Editor 路径
		virtual void OnUpdate(const TimeStep& timestep);
		virtual void OnUpdateRuntime(const TimeStep& timestep);
		virtual void OnUpdateSimulation(const TimeStep& timestep);
		virtual void OnUpdateEditor(const TimeStep& timestep);
		/// 固定步长逻辑：Runtime/Simulation 状态下驱动物理和脚本
		virtual void OnFixedUpdate(float fixedDeltaTime);

		/// 由场景持有相机所有权（反序列化时创建）；返回裸指针用于 CameraComponent
		Camera* AddOwnedCamera(Scope<Camera> cam);
		/// 清空场景持有的相机（Deserialize 开始时调用）
		void ClearOwnedCameras() { m_OwnedCameras.clear(); }
		/// 物理筛选规则入口：未来物理系统可复用该判定
		bool CanEntitiesCollide(Entity a, Entity b) const;

		/// 设置脚本运行时桥接（由 Application 初始化时注入）
		static void SetScriptRuntime(IScriptRuntime* runtime);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

	private:
		static IScriptRuntime* s_ScriptRuntime;
		b2World* m_PhysicsWorld = nullptr;
		SceneNode m_Root{ "Root" };
		World m_World;
		Entity m_MainCameraEntity;
		std::vector<Scope<Camera>> m_OwnedCameras;
		std::unordered_map<uint64_t, Entity> m_EntityMap;
		SceneRuntimeState m_RuntimeState = SceneRuntimeState::Edit;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		friend class SceneSerializer;
	};

} // namespace Ehu
