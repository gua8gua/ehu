#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "RendererAPI.h"

namespace Ehu {

	class Window;

	/// 渲染上下文：管理 GPU 资源与执行环境，封装 RendererAPI 的初始化/关闭，以及可选的交换链（Present）
	class EHU_API RenderContext {
	public:
		static void Init();
		static void Shutdown();

		static RendererAPI& GetAPI();

		/// 设置当前用于 Present 的窗口（SwapBuffers 时使用），由 Application 在创建窗口后调用
		static void SetCurrentWindow(Window* window);
		/// 交换当前窗口前后缓冲（Present），若无当前窗口则无操作
		static void SwapBuffers();

	private:
		static Window* s_CurrentWindow;
	};

} // namespace Ehu
