#include "ImGuiBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/ImGuiBackendGLFWOpenGL.h"
#endif

namespace Ehu {

	ImGuiBackend* ImGuiBackend::Create(GraphicsBackend backend) {
		switch (backend) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new ImGuiBackendGLFWOpenGL();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
