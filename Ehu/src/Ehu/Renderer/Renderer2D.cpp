#include "ehupch.h"
#include "Renderer2D.h"
#include "Platform/RendererAPI.h"
#include "Platform/Shader.h"
#include "Platform/VertexArray.h"
#include "Platform/VertexBuffer.h"
#include "Platform/IndexBuffer.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	Renderer2D::SceneData* Renderer2D::s_SceneData = nullptr;
	Shader* Renderer2D::s_Shader = nullptr;
	VertexArray* Renderer2D::s_QuadVertexArray = nullptr;

	static VertexBuffer* s_QuadVertexBuffer = nullptr;
	static IndexBuffer* s_QuadIndexBuffer = nullptr;

	static const char* s_VertexShaderSrc = R"(
		#version 330 core
		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in vec4 a_Color;
		uniform mat4 u_ViewProjection;
		uniform mat4 u_Transform;
		out vec4 v_Color;
		void main() {
			v_Color = a_Color;
			gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
		}
	)";
	static const char* s_FragmentShaderSrc = R"(
		#version 330 core
		in vec4 v_Color;
		uniform vec4 u_Color;
		out vec4 FragColor;
		void main() {
			FragColor = v_Color * u_Color;
		}
	)";

	void Renderer2D::Init() {
		s_SceneData = new SceneData();
		s_Shader = Shader::Create(s_VertexShaderSrc, s_FragmentShaderSrc);
		// 单位四边形（中心在原点）：position(3) + color(4) = 7 floats per vertex
		float quadVertices[] = {
			-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f
		};
		uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };
		s_QuadVertexArray = VertexArray::Create();
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
