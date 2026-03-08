#include "Ehu.h"
#include "Renderer/RendererModule.h"
#include "Core/Application.h"
#include "Core/Layer.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include "Renderer/Camera/Camera.h"
#include "Core/Ref.h"
#include "Platform/Render/Resources/VertexArray.h"
#include "Platform/Render/Resources/VertexBuffer.h"
#include "Platform/Render/Resources/IndexBuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

// 单位立方体几何（中心原点，边长 1）：与 Renderer3D Shader 布局一致，position(3) + color(4) = 7 floats/顶点
static void CreateCubeMesh(float* outVertices, uint32_t* outIndices) {
	const float c = 1.0f;
	float v[] = {
		-0.5f, -0.5f, -0.5f,  c,c,c,c,
		 0.5f, -0.5f, -0.5f,  c,c,c,c,
		 0.5f,  0.5f, -0.5f,  c,c,c,c,
		-0.5f,  0.5f, -0.5f,  c,c,c,c,
		-0.5f, -0.5f,  0.5f,  c,c,c,c,
		 0.5f, -0.5f,  0.5f,  c,c,c,c,
		 0.5f,  0.5f,  0.5f,  c,c,c,c,
		-0.5f,  0.5f,  0.5f,  c,c,c,c
	};
	for (int i = 0; i < 8 * 7; ++i) outVertices[i] = v[i];
	uint32_t i[] = {
		0, 1, 2,  2, 3, 0,
		5, 4, 7,  7, 6, 5,
		1, 5, 6,  6, 2, 1,
		4, 0, 3,  3, 7, 4,
		0, 4, 5,  5, 1, 0,
		3, 2, 6,  6, 7, 3
	};
	for (int j = 0; j < 36; ++j) outIndices[j] = i[j];
}

struct CubeData {
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec3 RotSpeed;
};

/// 示例 3D 场景：ECS 实体 + 立方体几何；OnUpdate 中更新 Transform 组件
class Example3DScene : public Ehu::Scene {
public:
	explicit Example3DScene(Ehu::Layer* renderLayer) : m_RenderLayer(renderLayer) {
		m_MainCamera = Ehu::CreateScope<Ehu::PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Ehu::Entity camEnt = CreateEntity();
		GetWorld().AddComponent(camEnt, Ehu::TransformComponent{});
		Ehu::TransformComponent* tc = GetWorld().GetComponent<Ehu::TransformComponent>(camEnt);
		tc->SetPosition(0.0f, 0.0f, 8.0f);
		tc->SetRotation(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
		GetWorld().AddComponent(camEnt, Ehu::CameraComponent{ m_MainCamera.get() });
		SetMainCamera(camEnt);

		std::srand(static_cast<unsigned>(std::time(nullptr)));
		m_Cubes.push_back({ { -2.0f,  0.0f,  0.0f }, { 0.9f, 0.2f, 0.2f, 1.0f }, { 0.7f, 0.3f, 0.5f } });
		m_Cubes.push_back({ {  0.0f,  0.0f,  0.0f }, { 0.2f, 0.9f, 0.2f, 1.0f }, { 0.4f, 0.8f, 0.2f } });
		m_Cubes.push_back({ {  2.0f,  0.0f,  0.0f }, { 0.2f, 0.3f, 0.9f, 1.0f }, { 0.3f, 0.5f, 0.9f } });
		m_Cubes.push_back({ { -1.0f, -1.5f, -1.0f }, { 0.95f, 0.9f, 0.2f, 1.0f }, { 0.6f, 0.1f, 0.4f } });
		m_Cubes.push_back({ {  1.0f,  1.2f, -0.5f }, { 0.6f, 0.2f, 0.8f, 1.0f }, { 0.2f, 0.7f, 0.6f } });
	}

	~Example3DScene() override {
		for (Ehu::Entity e : m_CubeEntities) {
			Ehu::MeshComponent* m = GetWorld().GetComponent<Ehu::MeshComponent>(e);
			if (m) m->VAO = nullptr;
		}
		m_CubeEntities.clear();
		delete m_CubeVertexArray;
		m_CubeVertexArray = nullptr;
		delete m_CubeVertexBuffer;
		m_CubeVertexBuffer = nullptr;
		delete m_CubeIndexBuffer;
		m_CubeIndexBuffer = nullptr;
	}

	void SetupCubes() {
		float cubeVertices[8 * 7];
		uint32_t cubeIndices[36];
		CreateCubeMesh(cubeVertices, cubeIndices);

		m_CubeVertexArray = Ehu::VertexArray::Create();
		m_CubeVertexBuffer = Ehu::VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
		m_CubeVertexArray->AddVertexBuffer(m_CubeVertexBuffer);
		m_CubeIndexBuffer = Ehu::IndexBuffer::Create(cubeIndices, 36);
		m_CubeVertexArray->SetIndexBuffer(m_CubeIndexBuffer);

		Ehu::World& w = GetWorld();
		for (const CubeData& cube : m_Cubes) {
			Ehu::Entity e = CreateEntity();
			Ehu::TransformComponent tr;
			tr.SetPosition(cube.Position);
			w.AddComponent(e, tr);
			Ehu::MeshComponent mesh;
			mesh.VAO = m_CubeVertexArray;
			mesh.IndexCount = 36;
			mesh.Color = cube.Color;
			mesh.SortKey = 0.0f;
			mesh.Transparent = false;
			w.AddComponent(e, mesh);
			Ehu::TagComponent* tag = w.GetComponent<Ehu::TagComponent>(e);
			if (tag) tag->RenderLayer = m_RenderLayer;
			m_CubeEntities.push_back(e);
		}
	}

	void OnUpdate(const Ehu::TimeStep& timestep) override {
		Ehu::Scene::OnUpdate(timestep);
		float t = timestep.GetSeconds();
		Ehu::World& w = GetWorld();
		for (size_t i = 0; i < m_Cubes.size() && i < m_CubeEntities.size(); ++i) {
			const CubeData& c = m_Cubes[i];
			Ehu::TransformComponent* tc = w.GetComponent<Ehu::TransformComponent>(m_CubeEntities[i]);
			if (!tc) continue;
			tc->SetPosition(c.Position);
			tc->SetRotation(glm::vec3(c.RotSpeed.x * t, c.RotSpeed.y * t, c.RotSpeed.z * t));
		}
	}

private:
	Ehu::Layer* m_RenderLayer = nullptr;
	Ehu::Scope<Ehu::Camera> m_MainCamera;
	std::vector<CubeData> m_Cubes;
	std::vector<Ehu::Entity> m_CubeEntities;
	Ehu::VertexArray* m_CubeVertexArray = nullptr;
	Ehu::VertexBuffer* m_CubeVertexBuffer = nullptr;
	Ehu::IndexBuffer* m_CubeIndexBuffer = nullptr;
};

class Example3DLayer : public Ehu::SceneLayer {
public:
	Example3DLayer() : SceneLayer("Example3D") {}
};

class SandApp : public Ehu::Application {
public:
	SandApp() {
		Example3DLayer* layer = new Example3DLayer();
		Ehu::Ref<Example3DScene> scene = Ehu::CreateRef<Example3DScene>(layer);
		scene->SetupCubes();
		RegisterScene(scene, true);
		PushLayer(layer);
	}
	~SandApp() {}
};

Ehu::Application* Ehu::CreateApplication() {
	return new SandApp();
}
