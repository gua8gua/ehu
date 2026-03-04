#include "ehupch.h"
#include "Window.h"
#include "Core/Log.h"
#include "GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/WindowsWindow.h"
#endif

namespace Ehu {

	Window* Window::Create(const WindowProps& props) {
		GraphicsBackend backend = GetGraphicsBackend();
		switch (backend) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new WindowsWindow(props);
#else
				EHU_CORE_ERROR("OpenGL_GLFW backend not implemented on this platform");
				return nullptr;
#endif
			}
			default:
				EHU_CORE_ERROR("Unknown or unsupported GraphicsBackend");
				return nullptr;
		}
	}

} // namespace Ehu
