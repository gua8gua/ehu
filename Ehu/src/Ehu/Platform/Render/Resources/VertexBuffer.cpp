#include "ehupch.h"
#include "VertexBuffer.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLVertexBuffer.h"
#endif

namespace Ehu {

	VertexBuffer* VertexBuffer::Create(uint32_t size) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLVertexBuffer(size);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	VertexBuffer* VertexBuffer::Create(const void* vertices, uint32_t size) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLVertexBuffer(vertices, size);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
