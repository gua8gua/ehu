#include "ehupch.h"
#include "Renderer2D.h"
#include "Platform/Render/RendererAPI.h"
#include "Platform/Render/Resources/Shader.h"
#include "Platform/Render/Resources/VertexArray.h"
#include "Platform/Render/Resources/VertexBuffer.h"
#include "Platform/Render/Resources/IndexBuffer.h"
#include "Platform/Render/Resources/Texture2D.h"
#include "Platform/Render/Resources/SubTexture2D.h"
#include "Text/Font.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	Renderer2D::SceneData* Renderer2D::s_SceneData = nullptr;
	Shader* Renderer2D::s_Shader = nullptr;
	Shader* Renderer2D::s_TexturedShader = nullptr;
	VertexArray* Renderer2D::s_QuadVertexArray = nullptr;
	VertexArray* Renderer2D::s_QuadTexturedVertexArray = nullptr;

	static VertexBuffer* s_QuadVertexBuffer = nullptr;
	static VertexBuffer* s_QuadTexturedVertexBuffer = nullptr;
	static IndexBuffer* s_QuadIndexBuffer = nullptr;

	void Renderer2D::Init() {
		s_SceneData = new SceneData();
		s_Shader = Shader::CreateDefault2D();
		s_TexturedShader = Shader::CreateDefault2DTextured();
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

		// 带 UV 的四边形：position(3) + color(4) + texCoord(2) = 9 floats
		float quadTexturedVertices[] = {
			-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f
		};
		s_QuadTexturedVertexArray = VertexArray::Create();
		s_QuadTexturedVertexBuffer = VertexBuffer::Create(quadTexturedVertices, sizeof(quadTexturedVertices));
		s_QuadTexturedVertexArray->AddVertexBuffer(s_QuadTexturedVertexBuffer, 9);
		s_QuadTexturedVertexArray->SetIndexBuffer(s_QuadIndexBuffer);
	}

	void Renderer2D::Shutdown() {
		delete s_SceneData;
		s_SceneData = nullptr;
		delete s_Shader;
		s_Shader = nullptr;
		delete s_TexturedShader;
		s_TexturedShader = nullptr;
		delete s_QuadVertexArray;
		s_QuadVertexArray = nullptr;
		delete s_QuadTexturedVertexArray;
		s_QuadTexturedVertexArray = nullptr;
		delete s_QuadVertexBuffer;
		s_QuadVertexBuffer = nullptr;
		delete s_QuadTexturedVertexBuffer;
		s_QuadTexturedVertexBuffer = nullptr;
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

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, texture, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint) {
		if (!texture) return;
		s_TexturedShader->Bind();
		s_TexturedShader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_TexturedShader->SetMat4("u_Transform",
			glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f)));
		s_TexturedShader->SetFloat4("u_Color", tint);
		s_TexturedShader->SetFloat2("u_UVMin", glm::vec2(0.0f, 0.0f));
		s_TexturedShader->SetFloat2("u_UVMax", glm::vec2(1.0f, 1.0f));
		texture->Bind(0);
		s_TexturedShader->SetInt("u_Texture", 0);
		s_QuadTexturedVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_QuadTexturedVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, subTexture, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint) {
		if (!subTexture.GetTexture()) return;
		s_TexturedShader->Bind();
		s_TexturedShader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_TexturedShader->SetMat4("u_Transform",
			glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f)));
		s_TexturedShader->SetFloat4("u_Color", tint);
		s_TexturedShader->SetFloat2("u_UVMin", subTexture.GetUVMin());
		s_TexturedShader->SetFloat2("u_UVMax", subTexture.GetUVMax());
		subTexture.GetTexture()->Bind(0);
		s_TexturedShader->SetInt("u_Texture", 0);
		s_QuadTexturedVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_QuadTexturedVertexArray);
	}

	void Renderer2D::DrawText(const glm::vec2& position, const std::string& text, Font* font, const glm::vec4& tint) {
		DrawText(glm::vec3(position.x, position.y, 0.0f), text, font, tint);
	}

	void Renderer2D::DrawText(const glm::vec3& position, const std::string& text, Font* font, const glm::vec4& tint) {
		if (!font || !font->GetAtlasTexture()) return;
		float x = position.x;
		const float y = position.y;
		const float z = position.z;
		for (unsigned char c : text) {
			const Font::GlyphInfo* g = font->GetGlyph(static_cast<int>(c));
			if (!g) continue;
			if (g->size.x > 0.0f && g->size.y > 0.0f) {
				SubTexture2D sub(font->GetAtlasTexture(), g->uvMin, g->uvMax);
				DrawQuad(glm::vec3(x, y, z), g->size, sub, tint);
			}
			x += g->advance;
		}
	}

} // namespace Ehu
