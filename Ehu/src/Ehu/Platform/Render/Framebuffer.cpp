#include "ehupch.h"
#include "Framebuffer.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLFramebuffer.h"
#endif

namespace Ehu {

	Framebuffer* Framebuffer::Create(const FramebufferSpec& spec) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLFramebuffer(spec);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
