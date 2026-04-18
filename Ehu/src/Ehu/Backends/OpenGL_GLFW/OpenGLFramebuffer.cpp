#include "ehupch.h"
#include "OpenGLFramebuffer.h"
#include <glad/glad.h>

namespace Ehu {

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpec& spec) : m_Spec(spec) {
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), m_ColorAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);
	}

	void OpenGLFramebuffer::Invalidate() {
		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), m_ColorAttachmentIDs.data());
			glDeleteTextures(1, &m_DepthAttachmentID);
			m_RendererID = 0;
			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		if (m_Spec.ColorAttachmentFormats.size() != m_Spec.NumColorAttachments) {
			m_Spec.ColorAttachmentFormats.assign(m_Spec.NumColorAttachments, FramebufferTextureFormat::RGBA8);
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		m_ColorAttachmentIDs.resize(m_Spec.NumColorAttachments);
		for (size_t i = 0; i < m_ColorAttachmentIDs.size(); ++i) {
			glGenTextures(1, &m_ColorAttachmentIDs[i]);
			glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[i]);
			const FramebufferTextureFormat fmt = m_Spec.ColorAttachmentFormats[i];
			if (fmt == FramebufferTextureFormat::RED_INTEGER_R32UI) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_Spec.Width, m_Spec.Height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			} else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Spec.Width, m_Spec.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_TEXTURE_2D, m_ColorAttachmentIDs[i], 0);
		}

		if (m_Spec.DepthStencil) {
			glGenTextures(1, &m_DepthAttachmentID);
			glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Spec.Width, m_Spec.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachmentID, 0);
		}

		if (m_ColorAttachmentIDs.size() > 1) {
			GLenum buffers[8] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
				GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
			glDrawBuffers(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), buffers);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Spec.Width, m_Spec.Height);
	}

	void OpenGLFramebuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
		m_Spec.Width = width;
		m_Spec.Height = height;
		Invalidate();
	}

	uint32_t OpenGLFramebuffer::GetColorAttachmentRendererID(uint32_t index) const {
		if (index >= m_ColorAttachmentIDs.size()) return 0;
		return m_ColorAttachmentIDs[index];
	}

	void OpenGLFramebuffer::ClearColorAttachmentRGBA(uint32_t attachmentIndex, float r, float g, float b, float a) {
		if (attachmentIndex >= m_ColorAttachmentIDs.size())
			return;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		const GLfloat c[4] = { r, g, b, a };
		glClearBufferfv(GL_COLOR, static_cast<GLint>(attachmentIndex), c);
	}

	void OpenGLFramebuffer::ClearColorAttachmentUInt(uint32_t attachmentIndex, uint32_t value) {
		if (attachmentIndex >= m_ColorAttachmentIDs.size())
			return;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLuint v = value;
		glClearBufferuiv(GL_COLOR, static_cast<GLint>(attachmentIndex), &v);
	}

	uint32_t OpenGLFramebuffer::ReadPixelUInt(uint32_t attachmentIndex, uint32_t x, uint32_t y) {
		if (attachmentIndex >= m_ColorAttachmentIDs.size())
			return 0;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		GLuint pixel = 0;
		glReadPixels(static_cast<GLint>(x), static_cast<GLint>(y), 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return pixel;
	}

} // namespace Ehu
