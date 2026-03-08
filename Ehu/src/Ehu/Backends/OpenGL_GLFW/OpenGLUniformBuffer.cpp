#include "ehupch.h"
#include "OpenGLUniformBuffer.h"
#include <glad/glad.h>

namespace Ehu {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t bindingSlot)
		: m_Size(size), m_BindingSlot(bindingSlot) {
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::Bind(uint32_t slot) const {
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_RendererID);
	}

} // namespace Ehu
