#pragma once

#include "Core.h"
#include "Ref.h"
#include "Platform/IO/Window.h"
#include "LayerStack.h"
#include "Events/ApplicationEvent.h"
#include "TimeStep.h"
#include <functional>
#include <mutex>
#include <vector>

namespace Ehu {

	struct ApplicationCommandLineArgs {
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const {
			return (index >= 0 && index < Count && Args) ? Args[index] : "";
		}
	};

	struct ApplicationSpecification {
		std::string Name = "Ehu Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
		bool EnableMainWindowSceneRendering = true;
	};

	class RenderQueue;
	class Scene;
	class Project;
	class ImGuiLayer;

	class EHU_API Application {
	public:
		Application(const ApplicationSpecification& specification = ApplicationSpecification());
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);

		/// Layer 所有权转移给 LayerStack，调用方不得再 delete；Application 析构时由 LayerStack::Clear() 统一释放
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		ImGuiLayer* EnableImGui();

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_Window; }
		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
		void Close();
		void SubmitToMainThread(const std::function<void()>& function);

		/// 场景注册：Application 持有 Ref<Scene>，析构或 UnregisterScene 时移除引用
		void RegisterScene(Ref<Scene> scene, bool activated = true);
		void UnregisterScene(Scene* scene);
		void SetSceneActivated(Scene* scene, bool activated);
		bool IsSceneActivated(Scene* scene) const;
		Ref<Scene> FindSceneRef(Scene* scene) const;
		void ConfigureProject(Project& project);
		Ref<Scene> LoadSceneFromProject(Project& project, const std::string& relativePath, bool activated = true);
		/// 当前已激活的场景列表（供 Layer 在逻辑 Tick 与 Extract 阶段使用）
		const std::vector<Scene*>& GetActivatedScenes() const;

		/// 根据 ProjectConfig.Scenes 中 Active 为 true 的条目加载并激活场景；返回成功加载的数量
		uint32_t ActivateScenesFromProject(Project& project);
		/// 清空当前已注册场景（用于关闭或切换项目）
		void DeactivateAllScenes();

		RenderQueue* GetRenderQueue() { return m_RenderQueue.get(); }
		const RenderQueue* GetRenderQueue() const { return m_RenderQueue.get(); }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		const ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		/// 层栈（供编辑器视口等多场景渲染使用）
		const LayerStack& GetLayerStack() const { return m_LayerStack; }

		/// 是否处于运行（Play）模式；编辑模式下为 false，部分脚本/逻辑可不执行
		bool IsPlayMode() const { return m_PlayMode; }
		void SetPlayMode(bool play);
		/// 是否将 Layer 提交的场景内容直接渲染到主窗口默认帧缓冲；编辑器可关闭，仅通过 Scene 视口显示
		void SetMainWindowSceneRenderingEnabled(bool enabled) { m_MainWindowSceneRenderingEnabled = enabled; }
		bool IsMainWindowSceneRenderingEnabled() const { return m_MainWindowSceneRenderingEnabled; }
		bool HasImGuiLayer() const { return m_ImGuiLayer != nullptr; }
		bool IsMinimized() const { return m_Minimized; }
		/// 上一帧 delta 时间（秒），供调试/统计用
		float GetDeltaTime() const { return m_TimeStep.GetDeltaTime(); }
		/// 固定时间步长（秒），用于脚本 OnPhysicsUpdate 与后续物理
		float GetFixedDeltaTime() const { return m_FixedDeltaTime; }
		/// 估算 FPS（1/delta），delta 为 0 时返回 0
		float GetFPS() const { float dt = m_TimeStep.GetDeltaTime(); return dt > 0.0f ? 1.0f / dt : 0.0f; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		void ExecuteMainThreadQueue();
		using ImGuiFrameThunk = void (*)(ImGuiLayer*);

		ApplicationSpecification m_Specification;
		LayerStack m_LayerStack;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer = nullptr;
		ImGuiFrameThunk m_BeginImGuiFrame = nullptr;
		ImGuiFrameThunk m_EndImGuiFrame = nullptr;
		Scope<RenderQueue> m_RenderQueue;
		TimeStep m_TimeStep;
		bool m_Running = true;
		bool m_Minimized = false;
		static Application* s_Instance;

		std::vector<std::pair<Ref<Scene>, bool>> m_Scenes;  /// scene + activated
		mutable std::vector<Scene*> m_ActivatedScenesCache;
		bool m_PlayMode = false;  /// 运行模式（与编辑模式相对，供脚本/逻辑按需分支）
		bool m_MainWindowSceneRenderingEnabled = true;
		float m_FixedDeltaTime = 1.0f / 60.0f;
		float m_FixedAccumulator = 0.0f;
		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace Ehu
