#include "ehupch.h"
#include "VertexArray.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLVertexArray.h"
#endif

namespace Ehu {

	VertexArray* VertexArray::Create() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLVertexArray();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
