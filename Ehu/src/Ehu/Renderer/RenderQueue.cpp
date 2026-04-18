#include "ehupch.h"
#include "RenderQueue.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Platform/Render/RendererAPI.h"
#include "Platform/Render/Resources/VertexArray.h"
#include "Platform/Render/Resources/Texture2D.h"
#include "Renderer/Text/Font.h"
#include "Camera/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Ehu {

	namespace {

		void DispatchDraw2D(const DrawCommand2D& cmd) {
			switch (cmd.Kind) {
			case Draw2DKind::ColorQuad:
				Renderer2D::DrawQuad(cmd.Position, cmd.Size, cmd.Color, cmd.EntityId);
				break;
			case Draw2DKind::TexturedQuad:
				if (cmd.Texture)
					Renderer2D::DrawQuad(cmd.WorldMatrix, cmd.Texture, cmd.Color, cmd.TilingFactor, cmd.EntityId);
				break;
			case Draw2DKind::Circle: {
				const glm::vec3 c(cmd.WorldMatrix[3]);
				const float sx = glm::length(glm::vec3(cmd.WorldMatrix[0]));
				const float sy = glm::length(glm::vec3(cmd.WorldMatrix[1]));
				const float radius = 0.5f * std::max(sx, sy);
				Renderer2D::DrawCircle(c, radius, cmd.Color, cmd.CircleThickness, 48, cmd.EntityId);
				break;
			}
			case Draw2DKind::Text:
				if (cmd.TextFont && !cmd.TextContent.empty())
					Renderer2D::DrawText(cmd.Position, cmd.TextContent, cmd.TextFont, cmd.Color, cmd.EntityId);
				break;
			}
		}

	} // namespace

	void RenderQueue::Reserve2D(size_t count) { m_Commands2D.reserve(count); }
	void RenderQueue::Reserve3D(size_t count) { m_Commands3D.reserve(count); }

	void RenderQueue::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey, uint32_t entityId) {
		DrawCommand2D cmd;
		cmd.Kind = Draw2DKind::ColorQuad;
		cmd.Position = position;
		cmd.Size = size;
		cmd.Color = color;
		cmd.SortKey = sortKey;
		cmd.Transparent = transparent;
		cmd.LayerIndex = m_CurrentLayerIndex;
		cmd.ViewCamera = viewCamera;
		cmd.MaterialKey = materialKey;
		cmd.EntityId = entityId;
		m_Commands2D.push_back(cmd);
	}

	void RenderQueue::SubmitQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey, uint32_t entityId) {
		SubmitQuad(glm::vec3(position.x, position.y, 0.0f), size, color, sortKey, transparent, viewCamera, materialKey, entityId);
	}

	void RenderQueue::SubmitTexturedQuad(const glm::mat4& world, Texture2D* texture, float tiling, const glm::vec4& tint,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey, uint32_t entityId) {
		if (!texture)
			return;
		DrawCommand2D cmd;
		cmd.Kind = Draw2DKind::TexturedQuad;
		cmd.WorldMatrix = world;
		cmd.Texture = texture;
		cmd.TilingFactor = tiling;
		cmd.Color = tint;
		cmd.SortKey = sortKey;
		cmd.Transparent = transparent;
		cmd.LayerIndex = m_CurrentLayerIndex;
		cmd.ViewCamera = viewCamera;
		cmd.MaterialKey = materialKey;
		cmd.EntityId = entityId;
		m_Commands2D.push_back(cmd);
	}

	void RenderQueue::SubmitCircle(const glm::mat4& world, const glm::vec4& color, float thickness, float fade,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey, uint32_t entityId) {
		(void)fade;
		DrawCommand2D cmd;
		cmd.Kind = Draw2DKind::Circle;
		cmd.WorldMatrix = world;
		cmd.Color = color;
		cmd.CircleThickness = thickness;
		cmd.CircleFade = fade;
		cmd.SortKey = sortKey;
		cmd.Transparent = transparent;
		cmd.LayerIndex = m_CurrentLayerIndex;
		cmd.ViewCamera = viewCamera;
		cmd.MaterialKey = materialKey;
		cmd.EntityId = entityId;
		m_Commands2D.push_back(cmd);
	}

	void RenderQueue::SubmitText(const glm::vec3& position, const std::string& text, Font* font, const glm::vec4& color,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey, uint32_t entityId) {
		if (!font || text.empty())
			return;
		DrawCommand2D cmd;
		cmd.Kind = Draw2DKind::Text;
		cmd.Position = position;
		cmd.TextContent = text;
		cmd.TextFont = font;
		cmd.Color = color;
		cmd.SortKey = sortKey;
		cmd.Transparent = transparent;
		cmd.LayerIndex = m_CurrentLayerIndex;
		cmd.ViewCamera = viewCamera;
		cmd.MaterialKey = materialKey;
		cmd.EntityId = entityId;
		m_Commands2D.push_back(cmd);
	}

	void RenderQueue::SubmitMesh(VertexArray* vertexArray, uint32_t indexCount, const glm::mat4& transform, const glm::vec4& color,
		float sortKey, bool transparent, Camera* viewCamera, uint32_t materialKey) {
		if (!vertexArray || indexCount == 0) return;
		DrawCommand3D cmd;
		cmd.VAO = vertexArray;
		cmd.IndexCount = indexCount;
		cmd.Transform = transform;
		cmd.Color = color;
		cmd.SortKey = sortKey;
		cmd.Transparent = transparent;
		cmd.LayerIndex = m_CurrentLayerIndex;
		cmd.ViewCamera = viewCamera;
		cmd.MaterialKey = materialKey;
		m_Commands3D.push_back(cmd);
	}

	void RenderQueue::Sort() {
		// Phase3 调度：不透明按 Layer -> Material/Shader -> 深度(Front-to-Back)；透明按 Layer -> 深度(Back-to-Front)
		auto sortOpaque2D = [this](size_t a, size_t b) {
			const auto& ca = m_Commands2D[a], cb = m_Commands2D[b];
			if (ca.LayerIndex != cb.LayerIndex) return ca.LayerIndex < cb.LayerIndex;
			if (ca.ViewCamera != cb.ViewCamera) return ca.ViewCamera < cb.ViewCamera;
			if (ca.MaterialKey != cb.MaterialKey) return ca.MaterialKey < cb.MaterialKey;
			return ca.SortKey < cb.SortKey;
		};
		auto sortTransparent2D = [this](size_t a, size_t b) {
			const auto& ca = m_Commands2D[a], cb = m_Commands2D[b];
			if (ca.LayerIndex != cb.LayerIndex) return ca.LayerIndex < cb.LayerIndex;
			if (ca.ViewCamera != cb.ViewCamera) return ca.ViewCamera < cb.ViewCamera;
			return ca.SortKey > cb.SortKey;
		};
		auto sortOpaque3D = [this](size_t a, size_t b) {
			const auto& ca = m_Commands3D[a], cb = m_Commands3D[b];
			if (ca.LayerIndex != cb.LayerIndex) return ca.LayerIndex < cb.LayerIndex;
			if (ca.ViewCamera != cb.ViewCamera) return ca.ViewCamera < cb.ViewCamera;
			if (ca.MaterialKey != cb.MaterialKey) return ca.MaterialKey < cb.MaterialKey;
			return ca.SortKey < cb.SortKey;
		};
		auto sortTransparent3D = [this](size_t a, size_t b) {
			const auto& ca = m_Commands3D[a], cb = m_Commands3D[b];
			if (ca.LayerIndex != cb.LayerIndex) return ca.LayerIndex < cb.LayerIndex;
			if (ca.ViewCamera != cb.ViewCamera) return ca.ViewCamera < cb.ViewCamera;
			return ca.SortKey > cb.SortKey;
		};

		m_SortedOpaque2D.clear();
		m_SortedTransparent2D.clear();
		for (size_t i = 0; i < m_Commands2D.size(); ++i) {
			if (m_Commands2D[i].Transparent)
				m_SortedTransparent2D.push_back(i);
			else
				m_SortedOpaque2D.push_back(i);
		}
		std::sort(m_SortedOpaque2D.begin(), m_SortedOpaque2D.end(), sortOpaque2D);
		std::sort(m_SortedTransparent2D.begin(), m_SortedTransparent2D.end(), sortTransparent2D);

		m_SortedOpaque3D.clear();
		m_SortedTransparent3D.clear();
		for (size_t i = 0; i < m_Commands3D.size(); ++i) {
			if (m_Commands3D[i].Transparent)
				m_SortedTransparent3D.push_back(i);
			else
				m_SortedOpaque3D.push_back(i);
		}
		std::sort(m_SortedOpaque3D.begin(), m_SortedOpaque3D.end(), sortOpaque3D);
		std::sort(m_SortedTransparent3D.begin(), m_SortedTransparent3D.end(), sortTransparent3D);
	}

	void RenderQueue::Clear() {
		m_Commands2D.clear();
		m_Commands3D.clear();
		m_SortedOpaque2D.clear();
		m_SortedTransparent2D.clear();
		m_SortedOpaque3D.clear();
		m_SortedTransparent3D.clear();
	}

	void RenderQueue::Flush2D() const {
		auto& api = RendererAPI::Get();
		api.SetBlend(false);
		for (size_t i : m_SortedOpaque2D)
			DispatchDraw2D(m_Commands2D[i]);
		api.SetBlend(true);
		for (size_t i : m_SortedTransparent2D)
			DispatchDraw2D(m_Commands2D[i]);
		api.SetBlend(false);
	}

	void RenderQueue::Flush3D() const {
		auto& api = RendererAPI::Get();
		api.SetBlend(false);
		for (size_t i : m_SortedOpaque3D) {
			const auto& cmd = m_Commands3D[i];
			Renderer3D::Submit(cmd.VAO, cmd.IndexCount, cmd.Transform, cmd.Color);
		}
		api.SetBlend(true);
		for (size_t i : m_SortedTransparent3D) {
			const auto& cmd = m_Commands3D[i];
			Renderer3D::Submit(cmd.VAO, cmd.IndexCount, cmd.Transform, cmd.Color);
		}
		api.SetBlend(false);
	}

	void RenderQueue::FlushAll() const {
		m_LastFrameStats = RenderStats{};

		// 获取所有不同的相机
		std::vector<Camera*> cameras;
		cameras.reserve(8);
		auto pushUnique = [&cameras](Camera* cam) {
			if (!cam) return;
			if (std::find(cameras.begin(), cameras.end(), cam) == cameras.end())
				cameras.push_back(cam);
		};

		for (size_t i : m_SortedOpaque3D) pushUnique(m_Commands3D[i].ViewCamera);
		for (size_t i : m_SortedTransparent3D) pushUnique(m_Commands3D[i].ViewCamera);
		for (size_t i : m_SortedOpaque2D) pushUnique(m_Commands2D[i].ViewCamera);
		for (size_t i : m_SortedTransparent2D) pushUnique(m_Commands2D[i].ViewCamera);


		// 分相机绘制
		auto& api = RendererAPI::Get();

		for (Camera* cam : cameras) {
			// 分3D和2D绘制
			const bool has3D = std::any_of(m_SortedOpaque3D.begin(), m_SortedOpaque3D.end(), [&](size_t i) { return m_Commands3D[i].ViewCamera == cam; })
				|| std::any_of(m_SortedTransparent3D.begin(), m_SortedTransparent3D.end(), [&](size_t i) { return m_Commands3D[i].ViewCamera == cam; });
			const bool has2D = std::any_of(m_SortedOpaque2D.begin(), m_SortedOpaque2D.end(), [&](size_t i) { return m_Commands2D[i].ViewCamera == cam; })
				|| std::any_of(m_SortedTransparent2D.begin(), m_SortedTransparent2D.end(), [&](size_t i) { return m_Commands2D[i].ViewCamera == cam; });

			if (has3D) {
				if (auto* pCam = dynamic_cast<PerspectiveCamera*>(cam)) {
					// 分不透明和透明绘制
					Renderer3D::BeginScene(*pCam);
					api.SetBlend(false);
					for (size_t i : m_SortedOpaque3D) {
						const auto& cmd = m_Commands3D[i];
						if (cmd.ViewCamera != cam) continue;
						Renderer3D::Submit(cmd.VAO, cmd.IndexCount, cmd.Transform, cmd.Color);
						m_LastFrameStats.DrawCalls3D++;
						m_LastFrameStats.Triangles3D += cmd.IndexCount / 3;
					}
					api.SetBlend(true);
					for (size_t i : m_SortedTransparent3D) {
						const auto& cmd = m_Commands3D[i];
						if (cmd.ViewCamera != cam) continue;
						Renderer3D::Submit(cmd.VAO, cmd.IndexCount, cmd.Transform, cmd.Color);
						m_LastFrameStats.DrawCalls3D++;
						m_LastFrameStats.Triangles3D += cmd.IndexCount / 3;
					}
					api.SetBlend(false);
					Renderer3D::EndScene();
				}
			}

			if (has2D) {
				Renderer2D::BeginScene(*cam);
				api.SetBlend(false);
				for (size_t i : m_SortedOpaque2D) {
					const auto& cmd = m_Commands2D[i];
					if (cmd.ViewCamera != cam) continue;
					DispatchDraw2D(cmd);
					m_LastFrameStats.Triangles2D += 2;
				}
				api.SetBlend(true);
				for (size_t i : m_SortedTransparent2D) {
					const auto& cmd = m_Commands2D[i];
					if (cmd.ViewCamera != cam) continue;
					DispatchDraw2D(cmd);
					m_LastFrameStats.Triangles2D += 2;
				}
				api.SetBlend(false);
				Renderer2D::EndScene();
				m_LastFrameStats.DrawCalls2D += Renderer2D::GetBatchDrawCallsThisScene();
			}
		}
	}

} // namespace Ehu
