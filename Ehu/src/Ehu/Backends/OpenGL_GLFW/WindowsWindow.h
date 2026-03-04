#pragma once

#include "ehupch.h"
#include "Platform/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Ehu {

	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;
		void SwapBuffers() override;
		unsigned int GetWidth() override { return m_Data.Width; }
		unsigned int GetHeight() override { return m_Data.Height; }
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		void* GetNativeWindow() const override { return m_Window; }
	private:
		void Init(const WindowProps& props);
		void Shutdown();

		GLFWwindow* m_Window = nullptr;
		struct WindowData {
			std::string Title;
			unsigned int Width, Height;
			bool VSync = false;
			EventCallbackFn EventCallback;
		};
		WindowData m_Data;
	};

} // namespace Ehu
