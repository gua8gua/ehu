#pragma once
#include "ehupch.h"
#include "Core.h"
#include "Events/Event.h"

namespace Ehu
{
	struct WindowProps {
		std::string title;
		unsigned int width, height;

		WindowProps(const std::string& title = "Ehu Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: title(title), width(width), height(height) {}
	};

	class EHU_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {};

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}
