# OpenGL + GLFW 后端实现
set(OPENGL_GLFW_SOURCES
	WindowsWindow.cpp
	WindowsInput.cpp
	ImGuiBackendGLFWOpenGL.cpp
	OpenGLRendererAPI.cpp
	OpenGLVertexBuffer.cpp
	OpenGLIndexBuffer.cpp
	OpenGLShader.cpp
	OpenGLVertexArray.cpp
	OpenGLTexture2D.cpp
	OpenGLFramebuffer.cpp
	OpenGLPipeline.cpp
)
set(OPENGL_GLFW_HEADERS
	WindowsWindow.h
	WindowsInput.h
	ImGuiBackendGLFWOpenGL.h
	OpenGLRendererAPI.h
	OpenGLVertexBuffer.h
	OpenGLIndexBuffer.h
	OpenGLShader.h
	OpenGLVertexArray.h
	OpenGLTexture2D.h
	OpenGLFramebuffer.h
	OpenGLPipeline.h
)
target_sources(EhuLib PRIVATE ${OPENGL_GLFW_SOURCES} ${OPENGL_GLFW_HEADERS})
target_include_directories(EhuLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
