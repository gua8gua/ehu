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
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace Ehu {

	Renderer2D::SceneData* Renderer2D::s_SceneData = nullptr;
	Shader* Renderer2D::s_Shader = nullptr;
	Shader* Renderer2D::s_TexturedShader = nullptr;
	VertexArray* Renderer2D::s_QuadVertexArray = nullptr;
	VertexArray* Renderer2D::s_QuadTexturedVertexArray = nullptr;
	Renderer2D::Statistics* Renderer2D::s_Stats = nullptr;
	bool Renderer2D::s_EntityIdPassEnabled = false;

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

	static Shader* s_BatchShaderEntityID = nullptr;
	static VertexArray* s_BatchVertexArrayEntityID = nullptr;
	static VertexBuffer* s_BatchVertexBufferEntityID = nullptr;
	static std::vector<float> s_BatchVertexDataEntity; // 每顶点 12 floats

	// 带纹理批渲染：按纹理 ID 断批，顶点格式 clipPos4 + color4 + texCoord2 = 10 floats
	static Shader* s_TexturedBatchShader = nullptr;
	static VertexArray* s_TexturedBatchVertexArray = nullptr;
	static VertexBuffer* s_TexturedBatchVertexBuffer = nullptr;
	static IndexBuffer* s_TexturedBatchIndexBuffer = nullptr;
	static std::vector<float> s_TexturedBatchVertexData;
	static uint32_t s_TexturedBatchQuadCount = 0;
	static Texture2D* s_TexturedBatchCurrentTexture = nullptr;

	static Shader* s_TexturedBatchShaderEntityID = nullptr;
	static VertexArray* s_TexturedBatchVertexArrayEntityID = nullptr;
	static VertexBuffer* s_TexturedBatchVertexBufferEntityID = nullptr;
	static std::vector<float> s_TexturedBatchVertexDataEntity; // 每顶点 14 floats
	static uint32_t s_TexturedBatchQuadCountEntity = 0;
	static Texture2D* s_TexturedBatchCurrentTextureEntity = nullptr;

	static void FlushTexturedBatchImpl();
	static void FlushTexturedBatchEntityImpl();

	namespace {

		constexpr glm::vec4 kQuadCorners[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		void PackEntityVec4(uint32_t entityId, float& o0, float& o1, float& o2, float& o3) {
			std::memcpy(&o0, &entityId, sizeof(float));
			o1 = o2 = o3 = 0.0f;
		}

		void PushColoredQuadVertices(const glm::mat4& vp, const glm::mat4& transform, const glm::vec4& color, uint32_t entityId) {
			if (Renderer2D::IsEntityIdPassEnabled()) {
				float ex, ey, ez, ew;
				PackEntityVec4(entityId, ex, ey, ez, ew);
				for (const glm::vec4& corner : kQuadCorners) {
					glm::vec4 clip = vp * transform * corner;
					s_BatchVertexDataEntity.push_back(clip.x);
					s_BatchVertexDataEntity.push_back(clip.y);
					s_BatchVertexDataEntity.push_back(clip.z);
					s_BatchVertexDataEntity.push_back(clip.w);
					s_BatchVertexDataEntity.push_back(color.r);
					s_BatchVertexDataEntity.push_back(color.g);
					s_BatchVertexDataEntity.push_back(color.b);
					s_BatchVertexDataEntity.push_back(color.a);
					s_BatchVertexDataEntity.push_back(ex);
					s_BatchVertexDataEntity.push_back(ey);
					s_BatchVertexDataEntity.push_back(ez);
					s_BatchVertexDataEntity.push_back(ew);
				}
			} else {
				for (const glm::vec4& corner : kQuadCorners) {
					glm::vec4 clip = vp * transform * corner;
					s_BatchVertexData.push_back(clip.x);
					s_BatchVertexData.push_back(clip.y);
					s_BatchVertexData.push_back(clip.z);
					s_BatchVertexData.push_back(clip.w);
					s_BatchVertexData.push_back(color.r);
					s_BatchVertexData.push_back(color.g);
					s_BatchVertexData.push_back(color.b);
					s_BatchVertexData.push_back(color.a);
				}
			}
			s_BatchQuadCount++;
			auto& stats = const_cast<Renderer2D::Statistics&>(Renderer2D::GetStats());
			stats.QuadCount++;
		}

		void PushTexturedQuadVertices(const glm::mat4& vp, const glm::mat4& transform, Texture2D* texture, const glm::vec2 (&uvs)[4], const glm::vec4& tint, uint32_t entityId) {
			if (!texture)
				return;
			if (Renderer2D::IsEntityIdPassEnabled()) {
				if (texture != s_TexturedBatchCurrentTextureEntity) {
					FlushTexturedBatchEntityImpl();
					s_TexturedBatchCurrentTextureEntity = texture;
				}
				if (s_TexturedBatchQuadCountEntity >= Renderer2D::kMaxBatchQuads)
					FlushTexturedBatchEntityImpl();

				float ex, ey, ez, ew;
				PackEntityVec4(entityId, ex, ey, ez, ew);
				for (int i = 0; i < 4; ++i) {
					glm::vec4 clip = vp * transform * kQuadCorners[i];
					s_TexturedBatchVertexDataEntity.push_back(clip.x);
					s_TexturedBatchVertexDataEntity.push_back(clip.y);
					s_TexturedBatchVertexDataEntity.push_back(clip.z);
					s_TexturedBatchVertexDataEntity.push_back(clip.w);
					s_TexturedBatchVertexDataEntity.push_back(tint.r);
					s_TexturedBatchVertexDataEntity.push_back(tint.g);
					s_TexturedBatchVertexDataEntity.push_back(tint.b);
					s_TexturedBatchVertexDataEntity.push_back(tint.a);
					s_TexturedBatchVertexDataEntity.push_back(uvs[i].x);
					s_TexturedBatchVertexDataEntity.push_back(uvs[i].y);
					s_TexturedBatchVertexDataEntity.push_back(ex);
					s_TexturedBatchVertexDataEntity.push_back(ey);
					s_TexturedBatchVertexDataEntity.push_back(ez);
					s_TexturedBatchVertexDataEntity.push_back(ew);
				}
				s_TexturedBatchQuadCountEntity++;
			} else {
				if (texture != s_TexturedBatchCurrentTexture) {
					FlushTexturedBatchImpl();
					s_TexturedBatchCurrentTexture = texture;
				}
				if (s_TexturedBatchQuadCount >= Renderer2D::kMaxBatchQuads)
					FlushTexturedBatchImpl();

				for (int i = 0; i < 4; ++i) {
					glm::vec4 clip = vp * transform * kQuadCorners[i];
					s_TexturedBatchVertexData.push_back(clip.x);
					s_TexturedBatchVertexData.push_back(clip.y);
					s_TexturedBatchVertexData.push_back(clip.z);
					s_TexturedBatchVertexData.push_back(clip.w);
					s_TexturedBatchVertexData.push_back(tint.r);
					s_TexturedBatchVertexData.push_back(tint.g);
					s_TexturedBatchVertexData.push_back(tint.b);
					s_TexturedBatchVertexData.push_back(tint.a);
					s_TexturedBatchVertexData.push_back(uvs[i].x);
					s_TexturedBatchVertexData.push_back(uvs[i].y);
				}
				s_TexturedBatchQuadCount++;
			}
			auto& stats = const_cast<Renderer2D::Statistics&>(Renderer2D::GetStats());
			stats.QuadCount++;
		}

		glm::mat4 ComposeTransform(const glm::vec3& position, const glm::vec2& size, float rotationRadians = 0.0f) {
			return glm::translate(glm::mat4(1.0f), position)
				* glm::rotate(glm::mat4(1.0f), rotationRadians, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		}

	}

	void Renderer2D::SetEntityIdPassEnabled(bool enabled) {
		s_EntityIdPassEnabled = enabled;
	}

	bool Renderer2D::IsEntityIdPassEnabled() {
		return s_EntityIdPassEnabled;
	}

	void Renderer2D::Init() {
		s_SceneData = new SceneData();
		s_Stats = new Statistics();
		s_Shader = Shader::CreateDefault2D();
		s_TexturedShader = Shader::CreateDefault2DTextured();
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

		s_BatchShader = Shader::CreateBatch2D();
		s_BatchShaderEntityID = Shader::CreateBatch2DEntityID();
		const uint32_t maxVerts = kMaxBatchQuads * 4;
		const uint32_t batchVBSize = maxVerts * 8 * sizeof(float);
		s_BatchVertexBuffer = VertexBuffer::Create(batchVBSize);
		s_BatchVertexArray = VertexArray::Create();
		s_BatchVertexArray->AddVertexBuffer(s_BatchVertexBuffer, 8);
		const uint32_t batchEntityVBSize = maxVerts * 12 * sizeof(float);
		s_BatchVertexBufferEntityID = VertexBuffer::Create(batchEntityVBSize);
		s_BatchVertexArrayEntityID = VertexArray::Create();
		s_BatchVertexArrayEntityID->AddVertexBuffer(s_BatchVertexBufferEntityID, 12);

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
		s_BatchVertexArrayEntityID->SetIndexBuffer(s_BatchIndexBuffer);
		s_BatchVertexData.reserve(maxVerts * 8);
		s_BatchVertexDataEntity.reserve(maxVerts * 12);

		s_TexturedBatchShader = Shader::CreateBatch2DTextured();
		s_TexturedBatchShaderEntityID = Shader::CreateBatch2DTexturedEntityID();
		const uint32_t texturedVBSize = maxVerts * 10 * sizeof(float);
		s_TexturedBatchVertexBuffer = VertexBuffer::Create(texturedVBSize);
		s_TexturedBatchVertexArray = VertexArray::Create();
		s_TexturedBatchVertexArray->AddVertexBuffer(s_TexturedBatchVertexBuffer, 10);
		const uint32_t texturedEntityVBSize = maxVerts * 14 * sizeof(float);
		s_TexturedBatchVertexBufferEntityID = VertexBuffer::Create(texturedEntityVBSize);
		s_TexturedBatchVertexArrayEntityID = VertexArray::Create();
		s_TexturedBatchVertexArrayEntityID->AddVertexBuffer(s_TexturedBatchVertexBufferEntityID, 14);
		s_TexturedBatchIndexBuffer = IndexBuffer::Create(batchIndices.data(), static_cast<uint32_t>(batchIndices.size()));
		s_TexturedBatchVertexArray->SetIndexBuffer(s_TexturedBatchIndexBuffer);
		s_TexturedBatchVertexArrayEntityID->SetIndexBuffer(s_TexturedBatchIndexBuffer);
		s_TexturedBatchVertexData.reserve(maxVerts * 10);
		s_TexturedBatchVertexDataEntity.reserve(maxVerts * 14);
	}

	void Renderer2D::Shutdown() {
		delete s_SceneData;
		s_SceneData = nullptr;
		delete s_Stats;
		s_Stats = nullptr;
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
		delete s_BatchShaderEntityID;
		s_BatchShaderEntityID = nullptr;
		delete s_BatchVertexArray;
		s_BatchVertexArray = nullptr;
		delete s_BatchVertexArrayEntityID;
		s_BatchVertexArrayEntityID = nullptr;
		delete s_BatchVertexBuffer;
		s_BatchVertexBuffer = nullptr;
		delete s_BatchVertexBufferEntityID;
		s_BatchVertexBufferEntityID = nullptr;
		delete s_BatchIndexBuffer;
		s_BatchIndexBuffer = nullptr;
		delete s_TexturedBatchShader;
		s_TexturedBatchShader = nullptr;
		delete s_TexturedBatchShaderEntityID;
		s_TexturedBatchShaderEntityID = nullptr;
		delete s_TexturedBatchVertexArray;
		s_TexturedBatchVertexArray = nullptr;
		delete s_TexturedBatchVertexArrayEntityID;
		s_TexturedBatchVertexArrayEntityID = nullptr;
		delete s_TexturedBatchVertexBuffer;
		s_TexturedBatchVertexBuffer = nullptr;
		delete s_TexturedBatchVertexBufferEntityID;
		s_TexturedBatchVertexBufferEntityID = nullptr;
		delete s_TexturedBatchIndexBuffer;
		s_TexturedBatchIndexBuffer = nullptr;
	}

	static void FlushBatchEntity() {
		if (s_BatchQuadCount == 0 || !s_BatchShaderEntityID) return;
		s_BatchShaderEntityID->Bind();
		s_BatchVertexBufferEntityID->SetData(s_BatchVertexDataEntity.data(), static_cast<uint32_t>(s_BatchVertexDataEntity.size() * sizeof(float)));
		s_BatchVertexArrayEntityID->Bind();
		RendererAPI::Get().DrawIndexed(s_BatchVertexArrayEntityID, s_BatchQuadCount * 6);
		s_BatchDrawCallsThisScene++;
		const_cast<Renderer2D::Statistics&>(Renderer2D::GetStats()).DrawCalls++;
		s_BatchQuadCount = 0;
		s_BatchVertexDataEntity.clear();
	}

	void Renderer2D::FlushBatch() {
		if (s_EntityIdPassEnabled) {
			FlushBatchEntity();
			return;
		}
		if (s_BatchQuadCount == 0) return;
		s_BatchShader->Bind();
		s_BatchVertexBuffer->SetData(s_BatchVertexData.data(), static_cast<uint32_t>(s_BatchVertexData.size() * sizeof(float)));
		s_BatchVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_BatchVertexArray, s_BatchQuadCount * 6);
		s_BatchDrawCallsThisScene++;
		const_cast<Statistics&>(GetStats()).DrawCalls++;
		s_BatchQuadCount = 0;
		s_BatchVertexData.clear();
	}

	static void FlushTexturedBatchEntityImpl() {
		if (s_TexturedBatchQuadCountEntity == 0 || !s_TexturedBatchCurrentTextureEntity) return;
		s_TexturedBatchShaderEntityID->Bind();
		s_TexturedBatchShaderEntityID->SetInt("u_Texture", 0);
		s_TexturedBatchCurrentTextureEntity->Bind(0);
		s_TexturedBatchVertexBufferEntityID->SetData(s_TexturedBatchVertexDataEntity.data(), static_cast<uint32_t>(s_TexturedBatchVertexDataEntity.size() * sizeof(float)));
		s_TexturedBatchVertexArrayEntityID->Bind();
		RendererAPI::Get().DrawIndexed(s_TexturedBatchVertexArrayEntityID, s_TexturedBatchQuadCountEntity * 6);
		s_BatchDrawCallsThisScene++;
		const_cast<Renderer2D::Statistics&>(Renderer2D::GetStats()).DrawCalls++;
		s_TexturedBatchQuadCountEntity = 0;
		s_TexturedBatchVertexDataEntity.clear();
		s_TexturedBatchCurrentTextureEntity = nullptr;
	}

	static void FlushTexturedBatchImpl() {
		if (Renderer2D::IsEntityIdPassEnabled()) {
			FlushTexturedBatchEntityImpl();
			return;
		}
		if (s_TexturedBatchQuadCount == 0 || !s_TexturedBatchCurrentTexture) return;
		s_TexturedBatchShader->Bind();
		s_TexturedBatchShader->SetInt("u_Texture", 0);
		s_TexturedBatchCurrentTexture->Bind(0);
		s_TexturedBatchVertexBuffer->SetData(s_TexturedBatchVertexData.data(), static_cast<uint32_t>(s_TexturedBatchVertexData.size() * sizeof(float)));
		s_TexturedBatchVertexArray->Bind();
		RendererAPI::Get().DrawIndexed(s_TexturedBatchVertexArray, s_TexturedBatchQuadCount * 6);
		s_BatchDrawCallsThisScene++;
		const_cast<Renderer2D::Statistics&>(Renderer2D::GetStats()).DrawCalls++;
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchVertexData.clear();
		s_TexturedBatchCurrentTexture = nullptr;
	}

	uint32_t Renderer2D::GetBatchDrawCallsThisScene() {
		return s_BatchDrawCallsThisScene;
	}

	const Renderer2D::Statistics& Renderer2D::GetStats() {
		return *s_Stats;
	}

	void Renderer2D::ResetStats() {
		if (s_Stats)
			*s_Stats = Statistics{};
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_BatchQuadCount = 0;
		s_BatchDrawCallsThisScene = 0;
		ResetStats();
		s_BatchVertexData.clear();
		s_BatchVertexDataEntity.clear();
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchQuadCountEntity = 0;
		s_TexturedBatchCurrentTexture = nullptr;
		s_TexturedBatchCurrentTextureEntity = nullptr;
		s_TexturedBatchVertexData.clear();
		s_TexturedBatchVertexDataEntity.clear();
	}

	void Renderer2D::BeginScene(const Camera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		s_BatchQuadCount = 0;
		s_BatchDrawCallsThisScene = 0;
		ResetStats();
		s_BatchVertexData.clear();
		s_BatchVertexDataEntity.clear();
		s_TexturedBatchQuadCount = 0;
		s_TexturedBatchQuadCountEntity = 0;
		s_TexturedBatchCurrentTexture = nullptr;
		s_TexturedBatchCurrentTextureEntity = nullptr;
		s_TexturedBatchVertexData.clear();
		s_TexturedBatchVertexDataEntity.clear();
	}

	void Renderer2D::EndScene() {
		FlushBatch();
		FlushTexturedBatchImpl();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t entityId) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, color, entityId);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, uint32_t entityId) {
		if (s_BatchQuadCount >= kMaxBatchQuads)
			FlushBatch();
		PushColoredQuadVertices(s_SceneData->ViewProjectionMatrix, ComposeTransform(position, size), color, entityId);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t entityId) {
		if (s_BatchQuadCount >= kMaxBatchQuads)
			FlushBatch();
		PushColoredQuadVertices(s_SceneData->ViewProjectionMatrix, transform, color, entityId);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color, uint32_t entityId) {
		DrawRotatedQuad(glm::vec3(position.x, position.y, 0.0f), size, rotationRadians, color, entityId);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color, uint32_t entityId) {
		if (s_BatchQuadCount >= kMaxBatchQuads)
			FlushBatch();
		PushColoredQuadVertices(s_SceneData->ViewProjectionMatrix, ComposeTransform(position, size, rotationRadians), color, entityId);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint, uint32_t entityId) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, texture, tint, entityId);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint, uint32_t entityId) {
		const glm::vec2 uvs[4] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		PushTexturedQuadVertices(s_SceneData->ViewProjectionMatrix, ComposeTransform(position, size), texture, uvs, tint, entityId);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, Texture2D* texture, const glm::vec4& tint, float tilingFactor, uint32_t entityId) {
		const float t = std::max(0.0001f, tilingFactor);
		const glm::vec2 uvs[4] = { { 0.0f, 0.0f }, { t, 0.0f }, { t, t }, { 0.0f, t } };
		PushTexturedQuadVertices(s_SceneData->ViewProjectionMatrix, transform, texture, uvs, tint, entityId);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint, uint32_t entityId) {
		DrawQuad(glm::vec3(position.x, position.y, 0.0f), size, subTexture, tint, entityId);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint, uint32_t entityId) {
		const glm::vec2 uvMin = subTexture.GetUVMin();
		const glm::vec2 uvMax = subTexture.GetUVMax();
		const glm::vec2 uvs[4] = { { uvMin.x, uvMin.y }, { uvMax.x, uvMin.y }, { uvMax.x, uvMax.y }, { uvMin.x, uvMax.y } };
		PushTexturedQuadVertices(s_SceneData->ViewProjectionMatrix, ComposeTransform(position, size), subTexture.GetTexture(), uvs, tint, entityId);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const SubTexture2D& subTexture, const glm::vec4& tint, uint32_t entityId) {
		const glm::vec2 uvMin = subTexture.GetUVMin();
		const glm::vec2 uvMax = subTexture.GetUVMax();
		const glm::vec2 uvs[4] = { { uvMin.x, uvMin.y }, { uvMax.x, uvMin.y }, { uvMax.x, uvMax.y }, { uvMin.x, uvMax.y } };
		PushTexturedQuadVertices(s_SceneData->ViewProjectionMatrix, transform, subTexture.GetTexture(), uvs, tint, entityId);
	}

	void Renderer2D::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness, uint32_t entityId) {
		DrawLine(glm::vec3(start.x, start.y, 0.0f), glm::vec3(end.x, end.y, 0.0f), color, thickness, entityId);
	}

	void Renderer2D::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float thickness, uint32_t entityId) {
		const glm::vec3 delta = end - start;
		const float length = glm::length(glm::vec2(delta.x, delta.y));
		if (length <= 0.0001f)
			return;
		const float angle = std::atan2(delta.y, delta.x);
		const glm::vec3 center = (start + end) * 0.5f;
		DrawRotatedQuad(center, { length, thickness }, angle, color, entityId);
		s_Stats->LineCount++;
	}

	void Renderer2D::DrawRect(const glm::vec2& center, const glm::vec2& size, const glm::vec4& color, float thickness) {
		DrawRect(glm::vec3(center.x, center.y, 0.0f), size, color, thickness);
	}

	void Renderer2D::DrawRect(const glm::vec3& center, const glm::vec2& size, const glm::vec4& color, float thickness) {
		const glm::vec2 half = size * 0.5f;
		const glm::vec3 tl(center.x - half.x, center.y + half.y, center.z);
		const glm::vec3 tr(center.x + half.x, center.y + half.y, center.z);
		const glm::vec3 bl(center.x - half.x, center.y - half.y, center.z);
		const glm::vec3 br(center.x + half.x, center.y - half.y, center.z);
		DrawLine(tl, tr, color, thickness);
		DrawLine(tr, br, color, thickness);
		DrawLine(br, bl, color, thickness);
		DrawLine(bl, tl, color, thickness);
	}

	void Renderer2D::DrawCircle(const glm::vec2& center, float radius, const glm::vec4& color, float thickness, uint32_t segments, uint32_t entityId) {
		DrawCircle(glm::vec3(center.x, center.y, 0.0f), radius, color, thickness, segments, entityId);
	}

	void Renderer2D::DrawCircle(const glm::vec3& center, float radius, const glm::vec4& color, float thickness, uint32_t segments, uint32_t entityId) {
		if (segments < 3 || radius <= 0.0f)
			return;
		const float twoPi = 6.28318530718f;
		for (uint32_t i = 0; i < segments; ++i) {
			const float a0 = (static_cast<float>(i) / static_cast<float>(segments)) * twoPi;
			const float a1 = (static_cast<float>(i + 1) / static_cast<float>(segments)) * twoPi;
			const glm::vec3 p0(center.x + std::cos(a0) * radius, center.y + std::sin(a0) * radius, center.z);
			const glm::vec3 p1(center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius, center.z);
			DrawLine(p0, p1, color, thickness, entityId);
		}
		s_Stats->CircleCount++;
	}

	void Renderer2D::DrawText(const glm::vec2& position, const std::string& text, Font* font, const glm::vec4& tint, uint32_t entityId) {
		DrawText(glm::vec3(position.x, position.y, 0.0f), text, font, tint, entityId);
	}

	void Renderer2D::DrawText(const glm::vec3& position, const std::string& text, Font* font, const glm::vec4& tint, uint32_t entityId) {
		if (!font || !font->GetAtlasTexture()) return;
		float x = position.x;
		float y = position.y;
		const float z = position.z;
		for (unsigned char c : text) {
			if (c == '\n') {
				x = position.x;
				y -= font->GetPixelHeight();
				continue;
			}
			const Font::GlyphInfo* g = font->GetGlyph(static_cast<int>(c));
			if (!g) continue;
			if (g->size.x > 0.0f && g->size.y > 0.0f) {
				SubTexture2D sub(font->GetAtlasTexture(), g->uvMin, g->uvMax);
				const glm::vec3 glyphCenter(
					x + g->bearing.x + g->size.x * 0.5f,
					y - g->bearing.y - g->size.y * 0.5f,
					z);
				DrawQuad(glyphCenter, g->size, sub, tint, entityId);
				s_Stats->TextGlyphCount++;
			}
			x += g->advance;
		}
	}

} // namespace Ehu
