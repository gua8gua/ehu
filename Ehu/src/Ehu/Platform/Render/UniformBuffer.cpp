#include "ehupch.h"
#include "UniformBuffer.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLUniformBuffer.h"
#endif

namespace Ehu {

	UniformBuffer* UniformBuffer::Create(uint32_t size, uint32_t bindingSlot) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLUniformBuffer(size, bindingSlot);
#else
				(void)size;
				(void)bindingSlot;
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
