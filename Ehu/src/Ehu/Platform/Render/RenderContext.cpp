#include "RenderContext.h"
#include "Platform/Render/RendererAPI.h"
#include "Platform/IO/Window.h"

namespace Ehu {

	Window* RenderContext::s_CurrentWindow = nullptr;

	void RenderContext::Init() {
		RendererAPI::Init();
	}

	void RenderContext::Shutdown() {
		RendererAPI::Shutdown();
		s_CurrentWindow = nullptr;
	}

	RendererAPI& RenderContext::GetAPI() {
		return RendererAPI::Get();
	}

	void RenderContext::SetCurrentWindow(Window* window) {
		s_CurrentWindow = window;
	}

	void RenderContext::SwapBuffers() {
		if (s_CurrentWindow)
			s_CurrentWindow->SwapBuffers();
	}

} // namespace Ehu
