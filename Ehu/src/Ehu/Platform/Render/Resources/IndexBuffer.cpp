#include "ehupch.h"
#include "IndexBuffer.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLIndexBuffer.h"
#endif

namespace Ehu {

	IndexBuffer* IndexBuffer::Create(const uint32_t* indices, uint32_t count) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLIndexBuffer(indices, count);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
