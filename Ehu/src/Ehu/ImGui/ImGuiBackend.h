#pragma once

#include "ehupch.h"
#include "Platform/Backend/GraphicsBackend.h"

namespace Ehu {

	class Window;

	/// ImGui 与底层窗口/渲染 API 的绑定抽象，便于按 GraphicsBackend 切换实现。
	///
	/// ImGui 本身只生成顶点/索引和绘制命令（ImDrawData），不负责：
	/// - 窗口与输入（由“平台后端”提供，如 imgui_impl_glfw）；
	/// - 把 ImDrawData 提交到 GPU（由“渲染后端”提供，如 imgui_impl_opengl3）。
	/// 本类把“平台后端 + 渲染后端”打包成一套接口，通过 Create(backend) 按环境
	/// 返回不同实现（如 ImGuiBackendGLFWOpenGL 内部调用上述两个 impl），
	/// 从而在不同环境下提供不同的“绘制/窗口”实现。
	class ImGuiBackend {
	public:
		virtual ~ImGuiBackend() = default;

		virtual void Init(Window* window) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame(Window* window) = 0;
		virtual void Shutdown() = 0;

		/// 根据当前后端创建对应实现，调用方负责 delete
		static ImGuiBackend* Create(GraphicsBackend backend);
	};

} // namespace Ehu
