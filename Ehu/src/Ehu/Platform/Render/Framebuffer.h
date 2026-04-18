#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Platform/Render/Resources/Texture2D.h"
#include <vector>

namespace Ehu {

	enum class FramebufferTextureFormat : uint8_t {
		None = 0,
		RGBA8,
		RED_INTEGER_R32UI
	};

	struct EHU_API FramebufferSpec {
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t NumColorAttachments = 1;
		bool DepthStencil = true;
		/// 与 NumColorAttachments 等长；为空则全部为 RGBA8
		std::vector<FramebufferTextureFormat> ColorAttachmentFormats;
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

		/// 将指定颜色附件清为浮点 RGBA（RGBA8 附件）
		virtual void ClearColorAttachmentRGBA(uint32_t attachmentIndex, float r, float g, float b, float a) = 0;
		/// 将指定颜色附件清为无符号整型值（用于 R32UI 拾取缓冲）
		virtual void ClearColorAttachmentUInt(uint32_t attachmentIndex, uint32_t value) = 0;
		/// 从指定颜色附件读取单像素（整型），坐标为左下角原点
		virtual uint32_t ReadPixelUInt(uint32_t attachmentIndex, uint32_t x, uint32_t y) = 0;

		static Framebuffer* Create(const FramebufferSpec& spec);
	};

} // namespace Ehu
