#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Texture2D.h"

namespace Ehu {

	struct EHU_API FramebufferSpec {
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t NumColorAttachments = 1;
		bool DepthStencil = true;
	};

	/// 帧缓冲抽象：离屏渲染、MRT、深度/模板附件
	class EHU_API Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual const FramebufferSpec& GetSpec() const = 0;

		static Framebuffer* Create(const FramebufferSpec& spec);
	};

} // namespace Ehu
