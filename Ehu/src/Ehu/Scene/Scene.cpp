#include "ehupch.h"
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

namespace Ehu {

	// --- Scene ---
	Scene::~Scene() {
		if (m_MainCamera && std::find(m_Cameras.begin(), m_Cameras.end(), m_MainCamera) == m_Cameras.end())
			m_MainCamera = nullptr;
		for (SceneCameraEntity* c : m_Cameras)
			delete c;
		m_Cameras.clear();
		m_MainCamera = nullptr;

		for (SceneEntity* e : m_Entities)
			delete e;
		m_Entities.clear();
	}

	void Scene::AddEntity(SceneEntity* entity) {
		if (entity && std::find(m_Entities.begin(), m_Entities.end(), entity) == m_Entities.end())
			m_Entities.push_back(entity);
	}

	void Scene::RemoveEntity(SceneEntity* entity) {
		auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
		if (it != m_Entities.end()) {
			delete *it;
			m_Entities.erase(it);
		}
	}

	void Scene::AddCamera(SceneCameraEntity* camera) {
		if (!camera) return;
		if (std::find(m_Cameras.begin(), m_Cameras.end(), camera) == m_Cameras.end())
			m_Cameras.push_back(camera);
		if (!m_MainCamera)
			m_MainCamera = camera;
	}

	void Scene::RemoveCamera(SceneCameraEntity* camera) {
		auto it = std::find(m_Cameras.begin(), m_Cameras.end(), camera);
		if (it != m_Cameras.end()) {
			if (m_MainCamera == *it)
				m_MainCamera = nullptr;
			delete *it;
			m_Cameras.erase(it);
		}
		if (!m_MainCamera && !m_Cameras.empty())
			m_MainCamera = m_Cameras.front();
	}

	void Scene::SetMainCamera(SceneCameraEntity* camera) {
		if (!camera) {
			m_MainCamera = nullptr;
			return;
		}
		if (std::find(m_Cameras.begin(), m_Cameras.end(), camera) == m_Cameras.end())
			m_Cameras.push_back(camera);
		m_MainCamera = camera;
	}

	// --- SceneNode ---
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

	// --- SceneEntity ---
	void SceneEntity::SetRotation(const glm::vec3& eulerRadians) {
		m_Rotation = glm::quat_cast(glm::eulerAngleXYZ(eulerRadians.x, eulerRadians.y, eulerRadians.z));
		m_TransformDirty = true;
	}

	void SceneEntity::SetTransform(const glm::mat4& t) {
		glm::vec3 skew;
		glm::vec4 persp;
		if (glm::decompose(t, m_Scale, m_Rotation, m_Position, skew, persp))
			m_TransformDirty = false;
		m_Transform = t;
	}

	const glm::mat4& SceneEntity::GetTransform() const {
		if (m_TransformDirty) {
			m_Transform = glm::translate(glm::mat4(1.0f), m_Position)
				* glm::mat4_cast(m_Rotation)
				* glm::scale(glm::mat4(1.0f), m_Scale);
			m_TransformDirty = false;
		}
		return m_Transform;
	}

} // namespace Ehu
