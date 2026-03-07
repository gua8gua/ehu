#include "ehupch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Ehu {

	OpenGLVertexArray::OpenGLVertexArray() {
		// 使用 3.3 兼容 API；glCreateVertexArrays 为 4.5 DSA，在 3.3 上下文中不可用会导致崩溃
		glGenVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const {
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const {
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
		Bind();
		vertexBuffer->Bind();

		// 布局：position (vec3) + color (vec4) = 7 floats, stride 7 * 4
		glEnableVertexAttribArray(m_VertexBufferIndex);
		glVertexAttribPointer(m_VertexBufferIndex, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void*)0);
		m_VertexBufferIndex++;
		glEnableVertexAttribArray(m_VertexBufferIndex);
		glVertexAttribPointer(m_VertexBufferIndex, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void*)(3 * sizeof(float)));
		m_VertexBufferIndex++;

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
		Bind();
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

} // namespace Ehu
