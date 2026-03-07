#include "ehupch.h"
#include "Renderer2D.h"
#include "Platform/Render/RendererAPI.h"
#include "Platform/Render/Resources/Shader.h"
#include "Platform/Render/Resources/VertexArray.h"
#include "Platform/Render/Resources/VertexBuffer.h"
#include "Platform/Render/Resources/IndexBuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <chrono>

namespace Ehu {

	Renderer2D::SceneData* Renderer2D::s_SceneData = nullptr;
	Shader* Renderer2D::s_Shader = nullptr;
	VertexArray* Renderer2D::s_QuadVertexArray = nullptr;

	static VertexBuffer* s_QuadVertexBuffer = nullptr;
	static IndexBuffer* s_QuadIndexBuffer = nullptr;

	void Renderer2D::Init() {
		// #region agent log
		{ std::ofstream _f("debug-8e1d5b.log", std::ios::app); if (_f) _f << "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer2D.cpp:init_step\",\"message\":\"step\",\"data\":{\"step\":0},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << ",\"hypothesisId\":\"C\"}\n"; }
		// #endregion
		s_SceneData = new SceneData();
		// #region agent log
		{ std::ofstream _f("debug-8e1d5b.log", std::ios::app); if (_f) _f << "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer2D.cpp:init_step\",\"message\":\"step\",\"data\":{\"step\":1},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << ",\"hypothesisId\":\"C\"}\n"; }
		// #endregion
		s_Shader = Shader::CreateDefault2D();
		// #region agent log
		{ std::ofstream _f("debug-8e1d5b.log", std::ios::app); if (_f) _f << "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer2D.cpp:init_step\",\"message\":\"step\",\"data\":{\"step\":2},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << ",\"hypothesisId\":\"C\"}\n"; }
		// #endregion
		// 单位四边形（中心在原点）：position(3) + color(4) = 7 floats per vertex
		float quadVertices[] = {
			-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f
		};
		uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };
		s_QuadVertexArray = VertexArray::Create();
		// #region agent log
		{ std::ofstream _f("debug-8e1d5b.log", std::ios::app); if (_f) _f << "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer2D.cpp:init_step\",\"message\":\"step\",\"data\":{\"step\":3},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << ",\"hypothesisId\":\"C\"}\n"; }
		// #endregion
		s_QuadVertexBuffer = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
		s_QuadVertexArray->AddVertexBuffer(s_QuadVertexBuffer);
		s_QuadIndexBuffer = IndexBuffer::Create(quadIndices, 6);
		s_QuadVertexArray->SetIndexBuffer(s_QuadIndexBuffer);
	}

	void Renderer2D::Shutdown() {
		delete s_SceneData;
		s_SceneData = nullptr;
		delete s_Shader;
		s_Shader = nullptr;
		delete s_QuadVertexArray;
		s_QuadVertexArray = nullptr;
		delete s_QuadVertexBuffer;
		s_QuadVertexBuffer = nullptr;
		delete s_QuadIndexBuffer;
		s_QuadIndexBuffer = nullptr;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	}

	void Renderer2D::BeginScene(const Camera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	}

	void Renderer2D::EndScene() {
		// 当前为立即模式，每 DrawQuad 即提交，此处无需 flush
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
		s_Shader->SetMat4("u_Transform",
			glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f)));
		s_Shader->SetFloat4("u_Color", color);
		s_QuadVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_QuadVertexArray);
	}

} // namespace Ehu
