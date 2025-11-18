#include "WindowsWindow.h"
#include "Ehu/Log.h"

#include "Events/ApplicationEvent.h"


namespace Ehu {
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.title;
		m_Data.Width = props.width;
		m_Data.Height = props.height;

		EHU_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

		// 初始化 GLFW
		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			EHU_CORE_ASSERT(success, "Could not initialize GLFW!");
			s_GLFWInitialized = true;
		}	

		// 创建 GLFW 窗口
		m_Window = glfwCreateWindow((int)props.width, (int)props.height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// 设置 GLFW 回调函数
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		// 处理事件和渲染
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}