#pragma once

#include "Platform/Render/UniformBuffer.h"

namespace Ehu {

	class OpenGLUniformBuffer : public UniformBuffer {
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t bindingSlot);
		virtual ~OpenGLUniformBuffer();

		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		void Bind(uint32_t slot = 0) const override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size = 0;
		uint32_t m_BindingSlot = 0;
	};

} // namespace Ehu
