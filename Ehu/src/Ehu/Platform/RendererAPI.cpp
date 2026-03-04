#include "RendererAPI.h"
#include "GraphicsBackend.h"
#include "Core/Log.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLRendererAPI.h"
#endif

namespace Ehu {

	RendererAPI* RendererAPI::s_API = nullptr;

	void RendererAPI::Init() {
		if (s_API)
			return;
		GraphicsBackend backend = GetGraphicsBackend();
		switch (backend) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				s_API = new OpenGLRendererAPI();
				break;
#else
				EHU_CORE_ERROR("OpenGL_GLFW RendererAPI not implemented on this platform");
				break;
#endif
			}
			default:
				EHU_CORE_ERROR("Unknown or unsupported GraphicsBackend for RendererAPI");
				break;
		}
	}

	void RendererAPI::Shutdown() {
		if (s_API) {
			delete s_API;
			s_API = nullptr;
		}
	}

	RendererAPI& RendererAPI::Get() {
		EHU_CORE_ASSERT(s_API, "RendererAPI not initialized");
		return *s_API;
	}

} // namespace Ehu
