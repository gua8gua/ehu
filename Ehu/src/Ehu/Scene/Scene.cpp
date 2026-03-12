#include "ehupch.h"
#include "Scene.h"
#include "ECS/Components.h"
#include "Renderer/Camera/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Ehu {

	Scene::~Scene() {
		// World 析构时自动释放所有组件；主相机 Entity 不再单独 delete
	}

	Entity Scene::CreateEntity() {
		Entity e = m_World.CreateEntity();
		m_World.AddComponent(e, IdComponent{ UUID() });
		m_World.AddComponent(e, TagComponent{});
		return e;
	}

	void Scene::DestroyEntity(Entity e) {
		m_World.DestroyEntity(e);
		if (e == m_MainCameraEntity)
			m_MainCameraEntity = Entity{};
	}

	Entity Scene::FindEntityByName(const std::string& name) const {
		Entity found{};
		m_World.Each<TagComponent>([&](Entity e, TagComponent& tag) {
			if (tag.Name == name) {
				found = e;
			}
		});
		return found;
	}

	Entity Scene::FindEntityByUUID(UUID uuid) const {
		Entity found{};
		m_World.Each<IdComponent>([&](Entity e, IdComponent& idc) {
			if (idc.Id == uuid) {
				found = e;
			}
		});
		return found;
	}

	Entity Scene::DuplicateEntity(Entity e) {
		if (!m_World.IsValid(e)) return Entity{};
		Entity neu = CreateEntity();
		// IdComponent 已由 CreateEntity 赋新 UUID；仅覆盖 Tag 名称
		if (const TagComponent* tc = m_World.GetComponent<TagComponent>(e)) {
			TagComponent* dst = m_World.GetComponent<TagComponent>(neu);
			if (dst) {
				dst->Name = tc->Name.empty() ? "Copy" : tc->Name + " (Copy)";
				dst->RenderLayer = tc->RenderLayer;
			}
		}
		if (const TransformComponent* tr = m_World.GetComponent<TransformComponent>(e))
			m_World.AddComponent(neu, *tr);
		if (const SpriteComponent* sp = m_World.GetComponent<SpriteComponent>(e))
			m_World.AddComponent(neu, *sp);
		if (const MeshComponent* mc = m_World.GetComponent<MeshComponent>(e))
			m_World.AddComponent(neu, *mc);
		return neu;
	}

	Camera* Scene::GetMainCamera() const {
		if (!m_World.IsValid(m_MainCameraEntity)) return nullptr;
		const CameraComponent* cc = m_World.GetComponent<CameraComponent>(m_MainCameraEntity);
		return cc ? cc->Camera : nullptr;
	}

	void Scene::OnUpdate(const TimeStep& timestep) {
		(void)timestep;
		// CameraSync 在 SceneLayer::SubmitTo 中、提取前对每个 Scene 的 World 执行
	}

	Camera* Scene::AddOwnedCamera(Scope<Camera> cam) {
		m_OwnedCameras.push_back(std::move(cam));
		return m_OwnedCameras.back().get();
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

} // namespace Ehu
