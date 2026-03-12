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
#include <vector>

namespace Ehu {

	Renderer2D::SceneData* Renderer2D::s_SceneData = nullptr;
	Shader* Renderer2D::s_Shader = nullptr;
	Shader* Renderer2D::s_TexturedShader = nullptr;
	VertexArray* Renderer2D::s_QuadVertexArray = nullptr;
	VertexArray* Renderer2D::s_QuadTexturedVertexArray = nullptr;

	static VertexBuffer* s_QuadVertexBuffer = nullptr;
	static VertexBuffer* s_QuadTexturedVertexBuffer = nullptr;
	static IndexBuffer* s_QuadIndexBuffer = nullptr;

	// 批渲染：纯色四边形合并为单次 Draw
	static Shader* s_BatchShader = nullptr;
	static VertexArray* s_BatchVertexArray = nullptr;
	static VertexBuffer* s_BatchVertexBuffer = nullptr;
	static IndexBuffer* s_BatchIndexBuffer = nullptr;
	static std::vector<float> s_BatchVertexData;  // 每顶点 8 floats (clipPos4 + color4)，每四边形 4 顶点
	static uint32_t s_BatchQuadCount = 0;
	static uint32_t s_BatchDrawCallsThisScene = 0;

	// 带纹理批渲染：按纹理 ID 断批，顶点格式 clipPos4 + color4 + texCoord2 = 10 floats
	static Shader* s_TexturedBatchShader = nullptr;
	static VertexArray* s_TexturedBatchVertexArray = nullptr;
	static VertexBuffer* s_TexturedBatchVertexBuffer = nullptr;
	static IndexBuffer* s_TexturedBatchIndexBuffer = nullptr;
	static std::vector<float> s_TexturedBatchVertexData;
	static uint32_t s_TexturedBatchQuadCount = 0;
	static Texture2D* s_TexturedBatchCurrentTexture = nullptr;

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

		// 批渲染：动态 VB（clipPos4 + color4 = 8 floats/vertex），预生成索引
		s_BatchShader = Shader::CreateBatch2D();
		const uint32_t maxVerts = kMaxBatchQuads * 4;
		const uint32_t batchVBSize = maxVerts * 8 * sizeof(float);
		s_BatchVertexBuffer = VertexBuffer::Create(batchVBSize);
		s_BatchVertexArray = VertexArray::Create();
		s_BatchVertexArray->AddVertexBuffer(s_BatchVertexBuffer, 8);  // 8 floats per vertex
		std::vector<uint32_t> batchIndices;
		batchIndices.reserve(kMaxBatchQuads * 6);
		for (uint32_t q = 0; q < kMaxBatchQuads; ++q) {
			uint32_t base = q * 4;
			batchIndices.push_back(base + 0);
			batchIndices.push_back(base + 1);
			batchIndices.push_back(base + 2);
			batchIndices.push_back(base + 2);
			batchIndices.push_back(base + 3);
			batchIndices.push_back(base + 0);
		}
		s_BatchIndexBuffer = IndexBuffer::Create(batchIndices.data(), static_cast<uint32_t>(batchIndices.size()));
		s_BatchVertexArray->SetIndexBuffer(s_BatchIndexBuffer);
		s_BatchVertexData.reserve(maxVerts * 8);

		// 带纹理批渲染：10 floats/vertex
		s_TexturedBatchShader = Shader::CreateBatch2DTextured();
		const uint32_t texturedVBSize = maxVerts * 10 * sizeof(float);
		s_TexturedBatchVertexBuffer = VertexBuffer::Create(texturedVBSize);
		s_TexturedBatchVertexArray = VertexArray::Create();
		s_TexturedBatchVertexArray->AddVertexBuffer(s_TexturedBatchVertexBuffer, 10);
		s_TexturedBatchIndexBuffer = IndexBuffer::Create(batchIndices.data(), static_cast<uint32_t>(batchIndices.size()));
		s_TexturedBatchVertexArray->SetIndexBuffer(s_TexturedBatchIndexBuffer);
		s_TexturedBatchVertexData.reserve(maxVerts * 10);
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
		delete s_BatchShader;
		s_BatchShader = nullptr;
		delete s_BatchVertexArray;
		s_BatchVertexArray = nullptr;
		delete s_BatchVertexBuffer;
		s_BatchVertexBuffer = nullptr;
		delete s_BatchIndexBuffer;
		s_BatchIndexBuffer = nullptr;
		delete s_TexturedBatchShader;
		s_TexturedBatchShader = nullptr;
		delete s_TexturedBatchVertexArray;
		s_TexturedBatchVertexArray = nullptr;
		delete s_TexturedBatchVertexBuffer;
		s_TexturedBatchVertexBuffer = nullptr;
		delete s_TexturedBatchIndexBuffer;
		s_TexturedBatchIndexBuffer = nullptr;
	}

	void Renderer2D::FlushBatch() {
		if (s_BatchQuadCount == 0) return;
		s_BatchShader->Bind();
		s_BatchVertexBuffer->SetData(s_BatchVertexData.data(), static_cast<uint32_t>(s_BatchVertexData.size() * sizeof(float)));
		s_BatchVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_BatchVertexArray, s_BatchQuadCount * 6);
		s_BatchDrawCallsThisScene++;
		s_BatchQuadCount = 0;
		s_BatchVertexData.clear();
	}

	static void FlushTexturedBatch() {
		if (s_TexturedBatchQuadCount == 0 || !s_TexturedBatchCurrentTexture) return;
		s_TexturedBatchShader->Bind();
		s_TexturedBatchShader->SetInt("u_Texture", 0);
		s_TexturedBatchCurrentTexture->Bind(0);
		s_TexturedBatchVertexBuffer->SetData(s_TexturedBatchVertexData.data(), static_cast<uint32_t>(s_TexturedBatchVertexData.size() * sizeof(float)));
		s_TexturedBatchVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_TexturedBatchVertexArray, s_TexturedBatchQuadCount * 6);
		s_BatchDrawCallsThisScene++;
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchVertexData.clear();
		s_TexturedBatchCurrentTexture = nullptr;
	}

	uint32_t Renderer2D::GetBatchDrawCallsThisScene() {
		return s_BatchDrawCallsThisScene;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_BatchQuadCount = 0;
		s_BatchDrawCallsThisScene = 0;
		s_BatchVertexData.clear();
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchCurrentTexture = nullptr;
		s_TexturedBatchVertexData.clear();
	}

	void Renderer2D::BeginScene(const Camera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_BatchQuadCount = 0;
		s_BatchDrawCallsThisScene = 0;
		s_BatchVertexData.clear();
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchCurrentTexture = nullptr;
		s_TexturedBatchVertexData.clear();
	}

	void Renderer2D::EndScene() {
		FlushBatch();
		FlushTexturedBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
		if (s_BatchQuadCount >= kMaxBatchQuads)
			FlushBatch();
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		const glm::mat4& vp = s_SceneData->ViewProjectionMatrix;
		const float* c = &color.r;
		// 单位四边形四角（中心在原点）
		const float corners[4][3] = {
			{ -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { -0.5f, 0.5f, 0.0f }
		};
		for (int i = 0; i < 4; ++i) {
			glm::vec4 world = transform * glm::vec4(corners[i][0], corners[i][1], corners[i][2], 1.0f);
			glm::vec4 clip = vp * world;
			s_BatchVertexData.push_back(clip.x);
			s_BatchVertexData.push_back(clip.y);
			s_BatchVertexData.push_back(clip.z);
			s_BatchVertexData.push_back(clip.w);
			s_BatchVertexData.push_back(c[0]);
			s_BatchVertexData.push_back(c[1]);
			s_BatchVertexData.push_back(c[2]);
			s_BatchVertexData.push_back(c[3]);
		}
		s_BatchQuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, texture, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint) {
		if (!texture) return;
		if (texture != s_TexturedBatchCurrentTexture) {
			FlushTexturedBatch();
			s_TexturedBatchCurrentTexture = texture;
		}
		if (s_TexturedBatchQuadCount >= kMaxBatchQuads)
			FlushTexturedBatch();
		if (s_TexturedBatchCurrentTexture != texture)
			s_TexturedBatchCurrentTexture = texture;
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		const glm::mat4& vp = s_SceneData->ViewProjectionMatrix;
		const float corners[4][3] = { { -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { -0.5f, 0.5f, 0.0f } };
		const float uvs[4][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		for (int i = 0; i < 4; ++i) {
			glm::vec4 world = transform * glm::vec4(corners[i][0], corners[i][1], corners[i][2], 1.0f);
			glm::vec4 clip = vp * world;
			s_TexturedBatchVertexData.push_back(clip.x); s_TexturedBatchVertexData.push_back(clip.y); s_TexturedBatchVertexData.push_back(clip.z); s_TexturedBatchVertexData.push_back(clip.w);
			s_TexturedBatchVertexData.push_back(tint.r); s_TexturedBatchVertexData.push_back(tint.g); s_TexturedBatchVertexData.push_back(tint.b); s_TexturedBatchVertexData.push_back(tint.a);
			s_TexturedBatchVertexData.push_back(uvs[i][0]); s_TexturedBatchVertexData.push_back(uvs[i][1]);
		}
		s_TexturedBatchQuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, subTexture, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint) {
		Texture2D* texture = subTexture.GetTexture();
		if (!texture) return;
		if (texture != s_TexturedBatchCurrentTexture) {
			FlushTexturedBatch();
			s_TexturedBatchCurrentTexture = texture;
		}
		if (s_TexturedBatchQuadCount >= kMaxBatchQuads)
			FlushTexturedBatch();
		if (s_TexturedBatchCurrentTexture != texture)
			s_TexturedBatchCurrentTexture = texture;
		const glm::vec2 uvMin = subTexture.GetUVMin();
		const glm::vec2 uvMax = subTexture.GetUVMax();
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		const glm::mat4& vp = s_SceneData->ViewProjectionMatrix;
		const float uvs[4][2] = { { uvMin.x, uvMin.y }, { uvMax.x, uvMin.y }, { uvMax.x, uvMax.y }, { uvMin.x, uvMax.y } };
		const float corners[4][3] = { { -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { -0.5f, 0.5f, 0.0f } };
		for (int i = 0; i < 4; ++i) {
			glm::vec4 world = transform * glm::vec4(corners[i][0], corners[i][1], corners[i][2], 1.0f);
			glm::vec4 clip = vp * world;
			s_TexturedBatchVertexData.push_back(clip.x); s_TexturedBatchVertexData.push_back(clip.y); s_TexturedBatchVertexData.push_back(clip.z); s_TexturedBatchVertexData.push_back(clip.w);
			s_TexturedBatchVertexData.push_back(tint.r); s_TexturedBatchVertexData.push_back(tint.g); s_TexturedBatchVertexData.push_back(tint.b); s_TexturedBatchVertexData.push_back(tint.a);
			s_TexturedBatchVertexData.push_back(uvs[i][0]); s_TexturedBatchVertexData.push_back(uvs[i][1]);
		}
		s_TexturedBatchQuadCount++;
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
