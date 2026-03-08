#include "WindowsWindow.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "Core/Log.h"
#include "Core/FileSystem.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <chrono>
#include <string>

namespace Ehu {

	static bool s_GLFWInitialized = false;
	static void GLFWErrorCallback(int error, const char* description) {
		EHU_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) {
		Init(props);
	}

	WindowsWindow::~WindowsWindow() {
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props) {
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"WindowsWindow.cpp:Init_start\",\"message\":\"Init entry\",\"data\":{\"step\":0},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"A\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		m_Data.Title = props.title;
		m_Data.Width = props.width;
		m_Data.Height = props.height;

		EHU_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

		if (!s_GLFWInitialized) {
			int success = glfwInit();
			EHU_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		// 请求 OpenGL 3.3 兼容 + 深度缓冲：保证有深度缓冲且避免 Core Profile 导致的闪退
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);

		m_Window = glfwCreateWindow((int)props.width, (int)props.height, m_Data.Title.c_str(), nullptr, nullptr);
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"WindowsWindow.cpp:post_create\",\"message\":\"glfwCreateWindow\",\"data\":{\"window_ok\":" + std::to_string(m_Window ? 1 : 0) + "},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"A\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		if (!m_Window) {
			const char* errDesc = nullptr;
			(void)glfwGetError(&errDesc);
			EHU_CORE_ERROR("glfwCreateWindow failed: {0}", errDesc ? errDesc : "unknown");
			EHU_CORE_ASSERT(false, "glfwCreateWindow failed");
			std::abort();
		}
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"WindowsWindow.cpp:post_glad\",\"message\":\"gladLoadGLLoader\",\"data\":{\"status\":" + std::to_string(status) + "},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"B\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		if (!status) {
			EHU_CORE_ERROR("gladLoadGLLoader failed");
			EHU_CORE_ASSERT(false, "Failed to initialize GLAD");
			std::abort();
		}
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;
			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
				case GLFW_PRESS:  { KeyPressedEvent event(key, 0); data.EventCallback(event); break; }
				case GLFW_RELEASE: { KeyReleasedEvent event(key); data.EventCallback(event); break; }
				case GLFW_REPEAT:  { KeyPressedEvent event(key, 1); data.EventCallback(event); break; }
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(codepoint);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
				case GLFW_PRESS:   { MouseButtonPressedEvent event(button); data.EventCallback(event); break; }
				case GLFW_RELEASE: { MouseButtonReleasedEvent event(button); data.EventCallback(event); break; }
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float)xoffset, (float)yoffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xpos, (float)ypos);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown() {
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate() {
		glfwPollEvents();
		SwapBuffers();
	}

	void WindowsWindow::SwapBuffers() {
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const {
		return m_Data.VSync;
	}

} // namespace Ehu
