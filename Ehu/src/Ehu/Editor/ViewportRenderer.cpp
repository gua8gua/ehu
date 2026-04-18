#include "ViewportRenderer.h"
#include "Core/Application.h"
#include "Core/LayerStack.h"
#include "ECS/Components.h"
#include "Platform/Render/Framebuffer.h"
#include "Platform/Render/RenderContext.h"
#include "Platform/Render/RendererAPI.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderQueue.h"
#include "Renderer/Drawable.h"
#include "Renderer/Camera/EditorCamera.h"
#include "Core/RuntimeStats.h"
#include "Core/Layer.h"
#include "Scene/Scene.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Ehu {

	namespace {

		void DrawWireRectXY(const glm::mat4& m, const glm::vec4& color, float thickness) {
			const glm::vec4 corners[4] = {
				m * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
				m * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f),
				m * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f),
				m * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f),
			};
			for (int i = 0; i < 4; ++i) {
				glm::vec3 a = glm::vec3(corners[i]) / corners[i].w;
				glm::vec3 b = glm::vec3(corners[(i + 1) % 4]) / corners[(i + 1) % 4].w;
				Renderer2D::DrawLine(a, b, color, thickness, 0);
			}
		}

		void DrawViewportEditorOverlays(Scene& scene, EditorCamera& cam, bool showPhysicsColliders, bool showSelectionOutline, Entity selected) {
			World& world = scene.GetWorld();
			Renderer2D::SetEntityIdPassEnabled(false);
			Renderer2D::BeginScene(static_cast<Camera&>(cam));

			if (showPhysicsColliders) {
				for (Entity e : scene.GetEntities()) {
					TransformComponent* tc = world.GetComponent<TransformComponent>(e);
					if (!tc)
						continue;
					if (BoxCollider2DComponent* bc = world.GetComponent<BoxCollider2DComponent>(e)) {
						glm::mat4 m = tc->GetWorldMatrix() * glm::translate(glm::mat4(1.0f), glm::vec3(bc->Offset, 0.0f))
							* glm::scale(glm::mat4(1.0f), glm::vec3(bc->Size.x * 2.0f, bc->Size.y * 2.0f, 1.0f));
						DrawWireRectXY(m, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.02f);
					}
					if (CircleCollider2DComponent* cc = world.GetComponent<CircleCollider2DComponent>(e)) {
						glm::mat4 wm = tc->GetWorldMatrix();
						glm::vec3 wcenter = glm::vec3(wm * glm::vec4(cc->Offset.x, cc->Offset.y, 0.0f, 1.0f));
						float r = cc->Radius * std::max(std::abs(tc->Scale.x), std::abs(tc->Scale.y));
						Renderer2D::DrawCircle(wcenter, r, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.02f, 48, 0);
					}
				}
			}

			if (showSelectionOutline && world.IsValid(selected)) {
				if (TransformComponent* tc = world.GetComponent<TransformComponent>(selected)) {
					glm::vec2 size(1.0f, 1.0f);
					if (SpriteComponent* sp = world.GetComponent<SpriteComponent>(selected))
						size = sp->Size;
					glm::mat4 m = tc->GetWorldMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
					DrawWireRectXY(m, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), 0.03f);
				}
			}

			Renderer2D::EndScene();
		}

	} // namespace

	ViewportRenderer::ViewportRenderer() {
		m_EditorCamera = CreateScope<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
		m_RenderQueue = CreateScope<RenderQueue>();
	}

	ViewportRenderer::~ViewportRenderer() {
		delete m_Framebuffer;
		m_Framebuffer = nullptr;
	}

	void ViewportRenderer::SetSize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) return;
		m_Width = width;
		m_Height = height;
		EnsureFramebuffer(width, height);
		if (m_EditorCamera)
			m_EditorCamera->SetViewportSize(static_cast<float>(width), static_cast<float>(height));
	}

	void ViewportRenderer::EnsureFramebuffer(uint32_t width, uint32_t height) {
		if (m_Framebuffer && m_Framebuffer->GetSpec().NumColorAttachments < 2) {
			delete m_Framebuffer;
			m_Framebuffer = nullptr;
		}
		if (m_Framebuffer && m_Framebuffer->GetSpec().Width == width && m_Framebuffer->GetSpec().Height == height)
			return;
		if (m_Framebuffer) {
			m_Framebuffer->Resize(width, height);
			return;
		}
		FramebufferSpec spec;
		spec.Width = width;
		spec.Height = height;
		spec.NumColorAttachments = 2;
		spec.DepthStencil = true;
		spec.ColorAttachmentFormats = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER_R32UI };
		m_Framebuffer = Framebuffer::Create(spec);
	}

	void ViewportRenderer::Render(Application& app, Scene* overlayScene, bool showPhysicsColliders, bool showSelectionOutline, Entity selectedForOutline) {
		if (!m_Framebuffer || m_Width == 0 || m_Height == 0) return;

		RendererAPI& api = RenderContext::GetAPI();
		api.SetViewport(0, 0, m_Width, m_Height);
		api.BeginRenderPass(m_Framebuffer);
		m_Framebuffer->ClearColorAttachmentRGBA(0, 0.2f, 0.3f, 0.3f, 1.0f);
		m_Framebuffer->ClearColorAttachmentUInt(1, 0);
		api.Clear(RendererAPI::ClearDepth);

		Renderer2D::SetEntityIdPassEnabled(true);

		m_RenderQueue->Clear();
		uint32_t layerIndex = 0;
		for (Layer* layer : app.GetLayerStack()) {
			if (IDrawable* d = dynamic_cast<IDrawable*>(layer)) {
				m_RenderQueue->SetCurrentLayerIndex(layerIndex);
				d->SubmitTo(*m_RenderQueue, m_EditorCamera.get());
			}
			layerIndex++;
		}
		m_RenderQueue->Sort();

		api.SetDepthTest(true);
		api.SetCullFace(false, true);
		m_RenderQueue->FlushAll();

		if (overlayScene && (showPhysicsColliders || (showSelectionOutline && selectedForOutline.id != 0)))
			DrawViewportEditorOverlays(*overlayScene, *m_EditorCamera, showPhysicsColliders, showSelectionOutline, selectedForOutline);

		if (!app.IsMainWindowSceneRenderingEnabled()) {
			RuntimeStats& stats = RuntimeStats::Get();
			const RenderStats& renderStats = m_RenderQueue->GetLastFrameStats();
			stats.DrawCalls2D = renderStats.DrawCalls2D;
			stats.DrawCalls3D = renderStats.DrawCalls3D;
			stats.Triangles2D = renderStats.Triangles2D;
			stats.Triangles3D = renderStats.Triangles3D;
			stats.Vertices2D = renderStats.Triangles2D * 2;
			stats.Vertices3D = renderStats.Triangles3D * 3;

			uint32_t activeEntities = 0;
			for (Scene* scene : app.GetActivatedScenes()) {
				if (scene)
					activeEntities += static_cast<uint32_t>(scene->GetEntities().size());
			}
			stats.ActiveEntities = activeEntities;
		}

		Renderer2D::SetEntityIdPassEnabled(false);
		api.EndRenderPass();
	}

	uint32_t ViewportRenderer::ReadEntityIdAt(uint32_t x, uint32_t y) const {
		if (!m_Framebuffer || x >= m_Width || y >= m_Height)
			return 0;
		return m_Framebuffer->ReadPixelUInt(1, x, y);
	}

	uint32_t ViewportRenderer::GetColorAttachmentTextureID(uint32_t index) const {
		if (!m_Framebuffer) return 0;
		return m_Framebuffer->GetColorAttachmentRendererID(index);
	}

} // namespace Ehu
