#pragma once

#include "Platform/Render/Resources/VertexBuffer.h"

namespace Ehu {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(const void* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		void Bind() const override;
		void Unbind() const override;
		void SetData(const void* data, uint32_t size) override;

	private:
		uint32_t m_RendererID = 0;
	};

} // namespace Ehu
