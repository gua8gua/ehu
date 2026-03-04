#include "ehupch.h"
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	SceneNode::SceneNode(const std::string& name) : m_Name(name) {}

	void SceneNode::AddChild(SceneNode* child) {
		if (child && child != this) {
			child->m_Parent = this;
			m_Children.push_back(child);
		}
	}

	void SceneNode::RemoveChild(SceneNode* child) {
		auto it = std::find(m_Children.begin(), m_Children.end(), child);
		if (it != m_Children.end()) {
			(*it)->m_Parent = nullptr;
			m_Children.erase(it);
		}
	}

	const glm::mat4& SceneNode::GetLocalTransform() const {
		if (m_TransformDirty) {
			m_LocalTransform = glm::translate(glm::mat4(1.0f), m_Position)
				* glm::mat4_cast(m_Rotation)
				* glm::scale(glm::mat4(1.0f), m_Scale);
			m_TransformDirty = false;
		}
		return m_LocalTransform;
	}

	glm::mat4 SceneNode::GetWorldTransform() const {
		glm::mat4 parentWorld = m_Parent ? m_Parent->GetWorldTransform() : glm::mat4(1.0f);
		return parentWorld * GetLocalTransform();
	}

} // namespace Ehu
