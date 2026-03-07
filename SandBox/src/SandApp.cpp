#include "Ehu.h"
#include "Renderer/RendererModule.h"
#include "Core/Application.h"
#include "Core/Layer.h"
#include "Scene/SceneCameraEntity.h"
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

/// 示例 3D 场景：持有立方体几何与逻辑数据，Phase1 在 OnUpdate 中更新实体变换；实体归属由 SetRenderLayer 指定
class Example3DScene : public Ehu::Scene {
public:
	explicit Example3DScene(Ehu::Layer* renderLayer) : m_RenderLayer(renderLayer) {
		// 主相机由场景管理：相机作为物体（SceneCameraEntity）存在于场景中，并在 Extract/Dispatch 阶段使用
		auto* camEnt = new Ehu::SceneCameraEntity(new Ehu::PerspectiveCamera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f));
		camEnt->SetPosition({ 0.0f, 0.0f, 8.0f });
		camEnt->SetRotation(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
		AddCamera(camEnt);
		SetMainCamera(camEnt);

		std::srand(static_cast<unsigned>(std::time(nullptr)));
		m_Cubes.push_back({ { -2.0f,  0.0f,  0.0f }, { 0.9f, 0.2f, 0.2f, 1.0f }, { 0.7f, 0.3f, 0.5f } });
		m_Cubes.push_back({ {  0.0f,  0.0f,  0.0f }, { 0.2f, 0.9f, 0.2f, 1.0f }, { 0.4f, 0.8f, 0.2f } });
		m_Cubes.push_back({ {  2.0f,  0.0f,  0.0f }, { 0.2f, 0.3f, 0.9f, 1.0f }, { 0.3f, 0.5f, 0.9f } });
		m_Cubes.push_back({ { -1.0f, -1.5f, -1.0f }, { 0.95f, 0.9f, 0.2f, 1.0f }, { 0.6f, 0.1f, 0.4f } });
		m_Cubes.push_back({ {  1.0f,  1.2f, -0.5f }, { 0.6f, 0.2f, 0.8f, 1.0f }, { 0.2f, 0.7f, 0.6f } });
	}

	~Example3DScene() override {
		for (Ehu::SceneEntity* e : GetEntities()) {
			if (e->HasMeshComponent()) {
				Ehu::MeshComponent m = e->GetMeshComponent();
				m.VAO = nullptr;
				e->SetMeshComponent(m);
			}
		}
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

		for (const CubeData& cube : m_Cubes) {
			Ehu::SceneEntity* e = new Ehu::SceneEntity();
			Ehu::MeshComponent mesh;
			mesh.VAO = m_CubeVertexArray;
			mesh.IndexCount = 36;
			mesh.Color = cube.Color;
			mesh.SortKey = 0.0f;
			mesh.Transparent = false;
			e->SetMeshComponent(mesh);
			e->SetPosition(cube.Position);
			e->SetRenderLayer(m_RenderLayer);
			AddEntity(e);
		}
	}

	void OnUpdate(const Ehu::TimeStep& timestep) override {
		float t = timestep.GetSeconds();
		const std::vector<Ehu::SceneEntity*>& entities = GetEntities();
		for (size_t i = 0; i < m_Cubes.size() && i < entities.size(); ++i) {
			const CubeData& c = m_Cubes[i];
			Ehu::SceneEntity* e = entities[i];
			e->SetPosition(c.Position);
			glm::vec3 euler(c.RotSpeed.x * t, c.RotSpeed.y * t, c.RotSpeed.z * t);
			e->SetRotation(euler);
		}
	}

private:
	Ehu::Layer* m_RenderLayer = nullptr;
	std::vector<CubeData> m_Cubes;
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
		Example3DScene* scene = new Example3DScene(layer);
		scene->SetupCubes();
		RegisterScene(scene, true);
		PushLayer(layer);
	}
	~SandApp() {}
};

Ehu::Application* Ehu::CreateApplication() {
	return new SandApp();
}
