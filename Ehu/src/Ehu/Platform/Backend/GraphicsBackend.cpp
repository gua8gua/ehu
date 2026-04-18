#include "GraphicsBackend.h"
#include <cstdlib>
#include <cstring>

namespace Ehu {

	static GraphicsBackend s_Backend = GraphicsBackend::None;

	GraphicsBackend GetGraphicsBackend() {
		if (s_Backend != GraphicsBackend::None)
			return s_Backend;

#ifdef EHU_PLATFORM_WINDOWS
		const char* env = std::getenv("EHU_GRAPHICS_BACKEND");
		if (env && std::strcmp(env, "OpenGL_GLFW") == 0)
			s_Backend = GraphicsBackend::OpenGL_GLFW;
		else
			s_Backend = GraphicsBackend::OpenGL_GLFW;
#else
		s_Backend = GraphicsBackend::OpenGL_GLFW;
#endif
		return s_Backend;
	}

} // namespace Ehu
