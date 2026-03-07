#pragma once

#include "Platform/Render/Resources/VertexArray.h"

namespace Ehu {

	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		void Bind() const override;
		void Unbind() const override;
		void AddVertexBuffer(VertexBuffer* vertexBuffer) override;
		void SetIndexBuffer(IndexBuffer* indexBuffer) override;

		const std::vector<VertexBuffer*>& GetVertexBuffers() const override { return m_VertexBuffers; }
		IndexBuffer* GetIndexBuffer() override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<VertexBuffer*> m_VertexBuffers;
		IndexBuffer* m_IndexBuffer = nullptr;
	};

} // namespace Ehu
