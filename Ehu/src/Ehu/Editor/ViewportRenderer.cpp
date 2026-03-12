#include "ViewportRenderer.h"
#include "Core/Application.h"
#include "Core/LayerStack.h"
#include "Platform/Render/Framebuffer.h"
#include "Platform/Render/RenderContext.h"
#include "Platform/Render/RendererAPI.h"
#include "Renderer/RenderQueue.h"
#include "Renderer/Drawable.h"
#include "Renderer/Camera/EditorCamera.h"
#include "Core/Layer.h"

namespace Ehu {

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
		if (m_Framebuffer && m_Framebuffer->GetSpec().Width == width && m_Framebuffer->GetSpec().Height == height)
			return;
		if (m_Framebuffer) {
			m_Framebuffer->Resize(width, height);
			return;
		}
		FramebufferSpec spec;
		spec.Width = width;
		spec.Height = height;
		spec.NumColorAttachments = 1;
		spec.DepthStencil = true;
		m_Framebuffer = Framebuffer::Create(spec);
	}

	void ViewportRenderer::Render(Application& app) {
		if (!m_Framebuffer || m_Width == 0 || m_Height == 0) return;

		RendererAPI& api = RenderContext::GetAPI();
		api.SetViewport(0, 0, m_Width, m_Height);
		api.BeginRenderPass(m_Framebuffer);
		api.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		api.Clear(RendererAPI::ClearColor | RendererAPI::ClearDepth);

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

		api.EndRenderPass();
	}

	uint32_t ViewportRenderer::GetColorAttachmentTextureID(uint32_t index) const {
		if (!m_Framebuffer) return 0;
		return m_Framebuffer->GetColorAttachmentRendererID(index);
	}

} // namespace Ehu
