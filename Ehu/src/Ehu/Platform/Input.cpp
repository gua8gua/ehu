#include "ehupch.h"
#include "Input.h"
#include "Core/Log.h"
#include "GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/WindowsInput.h"
#endif

namespace Ehu {

	Input* Input::s_instance = nullptr;

	void Input::Init() {
		if (s_instance)
			return;
		GraphicsBackend backend = GetGraphicsBackend();
		switch (backend) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				s_instance = new WindowsInput();
				break;
#else
				EHU_CORE_ERROR("OpenGL_GLFW Input not implemented on this platform");
				break;
#endif
			}
			default:
				EHU_CORE_ERROR("Unknown or unsupported GraphicsBackend for Input");
				break;
		}
	}

} // namespace Ehu
