#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Core/TimeStep.h"
#include "ECS/Entity.h"
#include "ECS/World.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

namespace Ehu {

	class Camera;

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

		SceneNode* GetRoot() { return &m_Root; }
		const SceneNode* GetRoot() const { return &m_Root; }

		World& GetWorld() { return m_World; }
		const World& GetWorld() const { return m_World; }

		Entity CreateEntity();
		void DestroyEntity(Entity e);
		std::vector<Entity> GetEntities() const { return m_World.GetEntities(); }

		void SetMainCamera(Entity e) { m_MainCameraEntity = e; }
		Entity GetMainCameraEntity() const { return m_MainCameraEntity; }
		Camera* GetMainCamera() const;

		/// 逻辑 Tick：运行 CameraSync 等系统，由 Layer::OnUpdate 驱动
		virtual void OnUpdate(const TimeStep& timestep);

	private:
		SceneNode m_Root{ "Root" };
		World m_World;
		Entity m_MainCameraEntity;
	};

} // namespace Ehu
