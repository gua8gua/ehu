#pragma once

#include "Platform/Render/Framebuffer.h"
#include <vector>

namespace Ehu {

	class OpenGLFramebuffer : public Framebuffer {
	public:
		explicit OpenGLFramebuffer(const FramebufferSpec& spec);
		virtual ~OpenGLFramebuffer();

		void Bind() override;
		void Unbind() override;
		void Resize(uint32_t width, uint32_t height) override;

		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override;
		const FramebufferSpec& GetSpec() const override { return m_Spec; }

		void ClearColorAttachmentRGBA(uint32_t attachmentIndex, float r, float g, float b, float a) override;
		void ClearColorAttachmentUInt(uint32_t attachmentIndex, uint32_t value) override;
		uint32_t ReadPixelUInt(uint32_t attachmentIndex, uint32_t x, uint32_t y) override;

	private:
		void Invalidate();

		FramebufferSpec m_Spec;
		uint32_t m_RendererID = 0;
		std::vector<uint32_t> m_ColorAttachmentIDs;
		uint32_t m_DepthAttachmentID = 0;
	};

} // namespace Ehu
