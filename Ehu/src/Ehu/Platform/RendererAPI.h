#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	class VertexArray;
	class Framebuffer;

	/// 渲染 API 抽象：清屏、绘制、状态、RenderPass 等，由各 Backend 实现
	class EHU_API RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		/// 清屏：bitmask 为 1=color, 2=depth, 4=stencil 组合
		virtual void Clear(uint32_t clearFlags = 1) = 0;

		/// 绘制索引几何
		virtual void DrawIndexed(VertexArray* vertexArray, uint32_t indexCount = 0) = 0;

		/// 渲染状态
		virtual void SetDepthTest(bool enable) = 0;
		virtual void SetBlend(bool enable) = 0;
		virtual void SetCullFace(bool enable, bool frontCCW = true) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		/// 渲染通道：Begin 绑定 Framebuffer（nullptr=默认帧缓冲），End 解绑
		virtual void BeginRenderPass(Framebuffer* framebuffer = nullptr) = 0;
		virtual void EndRenderPass() = 0;

		static void Init();
		static void Shutdown();
		static RendererAPI& Get();

		enum ClearFlag { ClearColor = 1, ClearDepth = 2, ClearStencil = 4 };

	private:
		static RendererAPI* s_API;
	};

} // namespace Ehu
