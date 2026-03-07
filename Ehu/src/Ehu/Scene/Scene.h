#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "SceneEntity.h"
#include "SceneCameraEntity.h"
#include "Core/TimeStep.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

namespace Ehu {

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

	/// 场景：纯数据容器，管理场景图与实体；实体由场景拥有（AddEntity 取得所有权，RemoveEntity 释放）
	/// 不实现 IDrawable；提交由渲染层 SceneLayer 通过 SubmitSceneToQueue 完成
	class EHU_API Scene {
	public:
		Scene() = default;
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		SceneNode* GetRoot() { return &m_Root; }
		const SceneNode* GetRoot() const { return &m_Root; }

		/// 加入实体并取得所有权；RemoveEntity 或析构时释放
		void AddEntity(SceneEntity* entity);
		/// 移除并 delete 该实体
		void RemoveEntity(SceneEntity* entity);
		const std::vector<SceneEntity*>& GetEntities() const { return m_Entities; }

		/// 相机管理：相机作为物体（SceneCameraEntity）存在于场景中，由场景选择主相机
		void AddCamera(SceneCameraEntity* camera);
		void RemoveCamera(SceneCameraEntity* camera);
		const std::vector<SceneCameraEntity*>& GetCameras() const { return m_Cameras; }

		void SetMainCamera(SceneCameraEntity* camera);
		SceneCameraEntity* GetMainCamera() const { return m_MainCamera; }

		/// 逻辑 Tick：执行 ECS/Systems（物理、脚本等），仅更新状态不渲染；由 Layer::OnUpdate 驱动
		virtual void OnUpdate(const TimeStep& timestep) { (void)timestep; }

	private:
		SceneNode m_Root{ "Root" };
		std::vector<SceneEntity*> m_Entities;
		std::vector<SceneCameraEntity*> m_Cameras;
		SceneCameraEntity* m_MainCamera = nullptr;
	};

} // namespace Ehu
