#pragma once

#include "Platform/Render/RendererAPI.h"

namespace Ehu {

	class OpenGLRendererAPI : public RendererAPI {
	public:
		void SetClearColor(float r, float g, float b, float a) override;
		void Clear(uint32_t clearFlags = 1) override;
		void DrawIndexed(VertexArray* vertexArray, uint32_t indexCount = 0) override;

		void SetDepthTest(bool enable) override;
		void SetBlend(bool enable) override;
		void SetCullFace(bool enable, bool frontCCW = true) override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void BeginRenderPass(Framebuffer* framebuffer = nullptr) override;
		void EndRenderPass() override;
	};

} // namespace Ehu
