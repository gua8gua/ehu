#include "ehupch.h"
#include "Texture2D.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLTexture2D.h"
#endif

namespace Ehu {

	Texture2D* Texture2D::Create(uint32_t width, uint32_t height, const void* data) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLTexture2D(width, height, data);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Texture2D* Texture2D::CreateFromFile(const std::string& path) {
#if !defined(EHU_USE_STB_IMAGE)
		(void)path;
		return nullptr;
#else
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLTexture2D(path);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
#endif
	}

} // namespace Ehu
