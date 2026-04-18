#include "ehupch.h"
#include "Scene.h"
#include "ECS/Components.h"
#include "ECS/ComponentReflection.h"
#include "Renderer/Camera/Camera.h"
#include "Scripting/IScriptRuntime.h"
#include "Core/Application.h"
#include "Physics/Physics2D.h"
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <unordered_map>

namespace Ehu {

	IScriptRuntime* Scene::s_ScriptRuntime = nullptr;

	namespace {

		void UpdateSceneCameraProjections(Scene& scene, uint32_t width, uint32_t height) {
			if (width == 0 || height == 0)
				return;

			scene.GetWorld().Each<CameraComponent>([&](Entity, CameraComponent& cameraComponent) {
				if (cameraComponent.FixedAspectRatio)
					return;
				Camera* camera = cameraComponent.Camera;
				if (!camera)
					return;

				const float aspect = static_cast<float>(width) / static_cast<float>(height);
				if (auto* perspective = dynamic_cast<PerspectiveCamera*>(camera)) {
					perspective->SetProjection(perspective->GetFov(), aspect, perspective->GetNearZ(), perspective->GetFarZ());
				} else if (auto* ortho = dynamic_cast<OrthographicCamera*>(camera)) {
					const float orthoHeight = ortho->GetTop() - ortho->GetBottom();
					const float orthoHalfHeight = orthoHeight * 0.5f;
					const float orthoHalfWidth = orthoHalfHeight * aspect;
					ortho->SetProjection(-orthoHalfWidth, orthoHalfWidth, -orthoHalfHeight, orthoHalfHeight, ortho->GetNearZ(), ortho->GetFarZ());
				}
			});
		}

		void SyncTransformsFromPhysics(World& world) {
			world.Each<TransformComponent, Rigidbody2DComponent>([&](Entity, TransformComponent& transform, Rigidbody2DComponent& rb) {
				if (!rb.RuntimeBody)
					return;
				auto* body = static_cast<b2Body*>(rb.RuntimeBody);
				const b2Vec2 p = body->GetPosition();
				transform.Position.x = p.x;
				transform.Position.y = p.y;
				transform.SetRotation(glm::angleAxis(body->GetAngle(), glm::vec3(0.0f, 0.0f, 1.0f)));
			});
		}

	}

	void Scene::SetScriptRuntime(IScriptRuntime* runtime) {
		s_ScriptRuntime = runtime;
	}

	Scene::~Scene() {
		OnPhysics2DStop();
		if (s_ScriptRuntime)
			s_ScriptRuntime->OnSceneDestroyed(this);
		// World 析构时自动释放所有组件；主相机 Entity 不再单独 delete
	}

	Ref<Scene> Scene::Copy(const Scene& other) {
		Ref<Scene> copy = CreateRef<Scene>();
		std::unordered_map<uint64_t, Entity> entityByRawId;
		copy->m_ViewportWidth = other.m_ViewportWidth;
		copy->m_ViewportHeight = other.m_ViewportHeight;
		copy->m_IsPaused = other.m_IsPaused;

		for (Entity src : other.GetEntities()) {
			const IdComponent* srcId = other.GetWorld().GetComponent<IdComponent>(src);
			if (!srcId)
				continue;

			Entity dst = copy->CreateEntity();
			if (IdComponent* dstId = copy->GetWorld().GetComponent<IdComponent>(dst)) {
				copy->m_EntityMap.erase(dstId->Id.Raw());
				dstId->Id = srcId->Id;
			}
			if (const TagComponent* srcTag = other.GetWorld().GetComponent<TagComponent>(src)) {
				if (TagComponent* dstTag = copy->GetWorld().GetComponent<TagComponent>(dst))
					dstTag->Name = srcTag->Name;
			}

			for (const ComponentCloneDescriptor& descriptor : ComponentReflectionRegistry::GetCloneDescriptors()) {
				if (descriptor.CloneFn)
					descriptor.CloneFn(other.GetWorld(), src, copy->GetWorld(), dst);
			}

			entityByRawId[srcId->Id.Raw()] = dst;
			copy->m_EntityMap[srcId->Id.Raw()] = dst;
		}

		for (Entity src : other.GetEntities()) {
			const IdComponent* srcId = other.GetWorld().GetComponent<IdComponent>(src);
			if (!srcId)
				continue;
			auto dstIt = entityByRawId.find(srcId->Id.Raw());
			if (dstIt == entityByRawId.end())
				continue;
			Entity dst = dstIt->second;

			const CameraComponent* srcCameraComponent = other.GetWorld().GetComponent<CameraComponent>(src);
			if (srcCameraComponent && srcCameraComponent->Camera) {
				Camera* clonedCamera = nullptr;
				if (const auto* perspective = dynamic_cast<const PerspectiveCamera*>(srcCameraComponent->Camera)) {
					auto camera = CreateScope<PerspectiveCamera>(perspective->GetFov(), perspective->GetAspect(), perspective->GetNearZ(), perspective->GetFarZ());
					camera->SetPosition(perspective->GetPosition());
					camera->SetRotation(perspective->GetRotation());
					clonedCamera = copy->AddOwnedCamera(std::move(camera));
				} else if (const auto* ortho = dynamic_cast<const OrthographicCamera*>(srcCameraComponent->Camera)) {
					auto camera = CreateScope<OrthographicCamera>(ortho->GetLeft(), ortho->GetRight(), ortho->GetBottom(), ortho->GetTop(), ortho->GetNearZ(), ortho->GetFarZ());
					camera->SetPosition(ortho->GetPosition());
					camera->SetRotation(ortho->GetRotation());
					clonedCamera = copy->AddOwnedCamera(std::move(camera));
				}

				if (clonedCamera)
					copy->GetWorld().AddComponent(dst, CameraComponent{ clonedCamera });
			}
		}

		const IdComponent* mainCameraId = other.GetWorld().GetComponent<IdComponent>(other.GetMainCameraEntity());
		if (mainCameraId) {
			Entity copiedMainCamera = copy->FindEntityByUUID(mainCameraId->Id);
			if (copy->GetWorld().IsValid(copiedMainCamera))
				copy->SetMainCamera(copiedMainCamera);
		}

		return copy;
	}

	Entity Scene::CreateEntity() {
		Entity e = m_World.CreateEntity();
		IdComponent id{ UUID() };
		m_World.AddComponent(e, id);
		m_World.AddComponent(e, TagComponent{});
		m_EntityMap[id.Id.Raw()] = e;
		return e;
	}

	void Scene::DestroyEntity(Entity e) {
		if (s_ScriptRuntime)
			s_ScriptRuntime->OnEntityDestroyed(this, e);
		if (const IdComponent* id = m_World.GetComponent<IdComponent>(e))
			m_EntityMap.erase(id->Id.Raw());
		m_World.DestroyEntity(e);
		if (e == m_MainCameraEntity)
			m_MainCameraEntity = Entity{};
	}

	Entity Scene::FindEntityByName(const std::string& name) const {
		Entity found{};
		m_World.Each<TagComponent>([&](Entity e, const TagComponent& tag) {
			if (tag.Name == name) {
				found = e;
			}
		});
		return found;
	}

	Entity Scene::FindEntityByUUID(UUID uuid) const {
		auto it = m_EntityMap.find(uuid.Raw());
		return it != m_EntityMap.end() ? it->second : Entity{};
	}

	Entity Scene::FindEntityByEntityId(uint32_t entityId) const {
		if (entityId == 0)
			return {};
		for (Entity e : GetEntities()) {
			if (e.id == entityId)
				return e;
		}
		return {};
	}

	Entity Scene::DuplicateEntity(Entity e) {
		if (!m_World.IsValid(e)) return Entity{};
		Entity neu = CreateEntity();
		// IdComponent 已由 CreateEntity 赋新 UUID；仅覆盖 Tag 名称
		if (const TagComponent* tc = m_World.GetComponent<TagComponent>(e)) {
			TagComponent* dst = m_World.GetComponent<TagComponent>(neu);
			if (dst) {
				dst->Name = tc->Name.empty() ? "Copy" : tc->Name + " (Copy)";
			}
		}
		for (const ComponentCloneDescriptor& descriptor : ComponentReflectionRegistry::GetCloneDescriptors()) {
			if (descriptor.CloneFn)
				descriptor.CloneFn(m_World, e, m_World, neu);
		}
		return neu;
	}

	Camera* Scene::GetMainCamera() const {
		Entity primary{};
		m_World.Each<CameraComponent>([&](Entity e, const CameraComponent& cc) {
			if (cc.Primary && cc.Camera)
				primary = e;
		});
		if (m_World.IsValid(primary)) {
			const CameraComponent* cc = m_World.GetComponent<CameraComponent>(primary);
			return cc ? cc->Camera : nullptr;
		}
		if (!m_World.IsValid(m_MainCameraEntity)) return nullptr;
		const CameraComponent* cc = m_World.GetComponent<CameraComponent>(m_MainCameraEntity);
		return cc ? cc->Camera : nullptr;
	}

	void Scene::OnRuntimeStart() {
		m_RuntimeState = SceneRuntimeState::Runtime;
		m_IsPaused = false;
		m_StepFrames = 0;
		OnPhysics2DStart();
	}

	void Scene::OnRuntimeStop() {
		OnPhysics2DStop();
		m_RuntimeState = SceneRuntimeState::Edit;
		m_IsPaused = false;
		m_StepFrames = 0;
	}

	void Scene::OnSimulationStart() {
		m_RuntimeState = SceneRuntimeState::Simulation;
		m_IsPaused = false;
		m_StepFrames = 0;
		OnPhysics2DStart();
	}

	void Scene::OnSimulationStop() {
		OnPhysics2DStop();
		m_RuntimeState = SceneRuntimeState::Edit;
		m_IsPaused = false;
		m_StepFrames = 0;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		UpdateSceneCameraProjections(*this, width, height);
	}

	void Scene::OnUpdate(const TimeStep& timestep) {
		switch (m_RuntimeState) {
		case SceneRuntimeState::Runtime:
			OnUpdateRuntime(timestep);
			break;
		case SceneRuntimeState::Simulation:
			OnUpdateSimulation(timestep);
			break;
		case SceneRuntimeState::Edit:
		default:
			OnUpdateEditor(timestep);
			break;
		}
	}

	void Scene::OnUpdateRuntime(const TimeStep& timestep) {
		if (s_ScriptRuntime && (!m_IsPaused || m_StepFrames > 0))
			s_ScriptRuntime->OnSceneUpdate(this, timestep.GetDeltaTime());
	}

	void Scene::OnUpdateSimulation(const TimeStep& timestep) {
		(void)timestep;
	}

	void Scene::OnUpdateEditor(const TimeStep& timestep) {
		(void)timestep;
	}

	void Scene::OnFixedUpdate(float fixedDeltaTime) {
		const bool stepAllowed = !m_IsPaused || m_StepFrames > 0;
		if (!stepAllowed)
			return;

		if (m_PhysicsWorld && (m_RuntimeState == SceneRuntimeState::Runtime || m_RuntimeState == SceneRuntimeState::Simulation)) {
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_PhysicsWorld->Step(fixedDeltaTime, velocityIterations, positionIterations);
			SyncTransformsFromPhysics(m_World);
		}
		if (m_RuntimeState == SceneRuntimeState::Runtime && s_ScriptRuntime)
			s_ScriptRuntime->OnSceneFixedUpdate(this, fixedDeltaTime);
		if (m_StepFrames > 0)
			m_StepFrames--;
	}

	void Scene::OnPhysics2DStart() {
		OnPhysics2DStop();
		m_PhysicsWorld = new b2World(b2Vec2(0.0f, -9.8f));

		m_World.Each<Rigidbody2DComponent, TransformComponent>([&](Entity e, Rigidbody2DComponent& rb, TransformComponent& tr) {
			b2BodyDef bodyDef;
			bodyDef.type = Physics2D::ToBox2DBody(rb.Type);
			bodyDef.position.Set(tr.Position.x, tr.Position.y);
			bodyDef.angle = Physics2D::QuatToZAngle(tr.Rotation);
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb.FixedRotation);
			body->SetGravityScale(rb.GravityScale);
			body->SetLinearVelocity(b2Vec2(rb.LinearVelocity.x, rb.LinearVelocity.y));
			rb.RuntimeBody = body;

			if (BoxCollider2DComponent* box = m_World.GetComponent<BoxCollider2DComponent>(e)) {
				b2PolygonShape boxShape;
				boxShape.SetAsBox(
					box->Size.x * tr.Scale.x,
					box->Size.y * tr.Scale.y,
					b2Vec2(box->Offset.x, box->Offset.y),
					0.0f);
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = box->Density;
				fixtureDef.friction = box->Friction;
				fixtureDef.restitution = box->Restitution;
				fixtureDef.restitutionThreshold = box->RestitutionThreshold;
				if (const PhysicsFilterComponent* pf = m_World.GetComponent<PhysicsFilterComponent>(e)) {
					fixtureDef.filter.categoryBits = static_cast<uint16_t>(pf->CollisionLayer & 0xFFFFu);
					fixtureDef.filter.maskBits = static_cast<uint16_t>(pf->CollisionMask & 0xFFFFu);
				}
				b2Fixture* fix = body->CreateFixture(&fixtureDef);
				box->RuntimeFixture = fix;
			}

			if (CircleCollider2DComponent* circle = m_World.GetComponent<CircleCollider2DComponent>(e)) {
				b2CircleShape circleShape;
				circleShape.m_p.Set(circle->Offset.x, circle->Offset.y);
				circleShape.m_radius = tr.Scale.x * circle->Radius;
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = circle->Density;
				fixtureDef.friction = circle->Friction;
				fixtureDef.restitution = circle->Restitution;
				fixtureDef.restitutionThreshold = circle->RestitutionThreshold;
				if (const PhysicsFilterComponent* pf = m_World.GetComponent<PhysicsFilterComponent>(e)) {
					fixtureDef.filter.categoryBits = static_cast<uint16_t>(pf->CollisionLayer & 0xFFFFu);
					fixtureDef.filter.maskBits = static_cast<uint16_t>(pf->CollisionMask & 0xFFFFu);
				}
				b2Fixture* fix = body->CreateFixture(&fixtureDef);
				circle->RuntimeFixture = fix;
			}
		});
	}

	void Scene::OnPhysics2DStop() {
		m_World.Each<Rigidbody2DComponent>([&](Entity, Rigidbody2DComponent& rb) {
			rb.RuntimeBody = nullptr;
		});
		m_World.Each<BoxCollider2DComponent>([&](Entity, BoxCollider2DComponent& box) {
			box.RuntimeFixture = nullptr;
		});
		m_World.Each<CircleCollider2DComponent>([&](Entity, CircleCollider2DComponent& c) {
			c.RuntimeFixture = nullptr;
		});
		if (m_PhysicsWorld) {
			delete m_PhysicsWorld;
			m_PhysicsWorld = nullptr;
		}
	}

	Camera* Scene::AddOwnedCamera(Scope<Camera> cam) {
		m_OwnedCameras.push_back(std::move(cam));
		return m_OwnedCameras.back().get();
	}

	bool Scene::CanEntitiesCollide(Entity a, Entity b) const {
		if (!m_World.IsValid(a) || !m_World.IsValid(b))
			return false;
		const PhysicsFilterComponent* fa = m_World.GetComponent<PhysicsFilterComponent>(a);
		const PhysicsFilterComponent* fb = m_World.GetComponent<PhysicsFilterComponent>(b);
		if (!fa || !fb)
			return false;
		return ShouldCollide(*fa, *fb);
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
