#include "ehupch.h"
#include "ImGuiBackendGLFWOpenGL.h"
#include "Platform/IO/Window.h"
#include "Core/FileSystem.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Ehu {

	// 初始化 ImGui
	void ImGuiBackendGLFWOpenGL::Init(Window* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// 暂不启用 ViewportsEnable，避免多视口渲染路径在主窗口上清屏导致 3D 场景被擦除；与 OpenGL 3.3 一致使用 330
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		const ImWchar* cjkRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
		const char* fontCandidates[] = {
			"C:/Windows/Fonts/msyh.ttc",
			"C:/Windows/Fonts/msyh.ttf",
			"C:/Windows/Fonts/simhei.ttf",
			"C:/Windows/Fonts/simsun.ttc"
		};
		bool loadedCjkFont = false;
		for (const char* fontPath : fontCandidates) {
			if (!FileSystem::IsFile(fontPath))
				continue;
			if (io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, nullptr, cjkRanges)) {
				loadedCjkFont = true;
				break;
			}
		}
		if (!loadedCjkFont)
			io.Fonts->AddFontDefault();

		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* native = static_cast<GLFWwindow*>(window->GetNativeWindow());
		ImGui_ImplGlfw_InitForOpenGL(native, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	// 开始 ImGui 帧
	void ImGuiBackendGLFWOpenGL::BeginFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	// 结束 ImGui 帧
	void ImGuiBackendGLFWOpenGL::EndFrame(Window* window) {
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}
	}

	// 关闭 ImGui
	void ImGuiBackendGLFWOpenGL::Shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace Ehu
