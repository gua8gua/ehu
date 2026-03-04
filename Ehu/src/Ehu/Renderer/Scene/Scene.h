#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

namespace Ehu {

	/// 场景图节点：层次结构、变换传播（平台无关）
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

	/// 场景根：管理场景图与渲染排序（占位）
	class EHU_API Scene {
	public:
		Scene() = default;
		~Scene() = default;

		SceneNode* GetRoot() { return &m_Root; }
		const SceneNode* GetRoot() const { return &m_Root; }

	private:
		SceneNode m_Root{ "Root" };
	};

} // namespace Ehu
