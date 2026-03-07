#include "Pipeline.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLPipeline.h"
#endif

namespace Ehu {

	Pipeline* Pipeline::Create() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLPipeline();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
