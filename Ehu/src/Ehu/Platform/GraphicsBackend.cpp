#include "GraphicsBackend.h"
#include "Core/Log.h"
#include <cstdlib>
#include <cstring>

namespace Ehu {

	static GraphicsBackend s_Backend = GraphicsBackend::None;

	GraphicsBackend GetGraphicsBackend() {
		if (s_Backend != GraphicsBackend::None)
			return s_Backend;

#ifdef EHU_PLATFORM_WINDOWS
		const char* env = std::getenv("EHU_GRAPHICS_BACKEND");
		if (env && std::strcmp(env, "OpenGL_GLFW") == 0) {
			s_Backend = GraphicsBackend::OpenGL_GLFW;
			EHU_CORE_TRACE("Graphics backend: OpenGL_GLFW (from env EHU_GRAPHICS_BACKEND)");
		} else {
			s_Backend = GraphicsBackend::OpenGL_GLFW;
			EHU_CORE_TRACE("Graphics backend: OpenGL_GLFW (default)");
		}
#else
		s_Backend = GraphicsBackend::OpenGL_GLFW;
		EHU_CORE_TRACE("Graphics backend: OpenGL_GLFW (default)");
#endif
		return s_Backend;
	}

} // namespace Ehu
