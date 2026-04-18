#include "ehupch.h"
#include "Shader.h"
#include "Core/FileSystem.h"
#include "Platform/Backend/GraphicsBackend.h"

#if defined(EHU_PLATFORM_WINDOWS)
#include "Backends/OpenGL_GLFW/OpenGLShader.h"
#endif

namespace Ehu {

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return new OpenGLShader(vertexSrc, fragmentSrc);
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
		std::string vs = FileSystem::ReadTextFile(vertexPath);
		std::string fs = FileSystem::ReadTextFile(fragmentPath);
		if (vs.empty() || fs.empty())
			return nullptr;
		return Create(vs, fs);
	}

	Shader* Shader::CreateDefault2D() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateDefault2D();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateDefault2DTextured() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateDefault2DTextured();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateBatch2D() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateBatch2D();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateBatch2DTextured() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateBatch2DTextured();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateBatch2DEntityID() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateBatch2DEntityID();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateBatch2DTexturedEntityID() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateBatch2DTexturedEntityID();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

	Shader* Shader::CreateDefault3D() {
		switch (GetGraphicsBackend()) {
			case GraphicsBackend::OpenGL_GLFW: {
#if defined(EHU_PLATFORM_WINDOWS)
				return OpenGLShader::CreateDefault3D();
#else
				return nullptr;
#endif
			}
			default:
				return nullptr;
		}
	}

} // namespace Ehu
