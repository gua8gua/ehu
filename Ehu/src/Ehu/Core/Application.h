#pragma once

#include "Core.h"
#include "Platform/IO/Window.h"
#include "LayerStack.h"
#include <vector>
#include "TimeStep.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "ImGui/ImGuiLayer.h"

namespace Ehu {

	class RenderQueue;
	class Scene;

	class EHU_API Application {
	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);

		/// Layer 所有权转移给 LayerStack，调用方不得再 delete；Application 析构时由 LayerStack::Clear() 统一释放
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_Window; }

		/// 场景注册：Application 取得 Scene 所有权，析构或 UnregisterScene 时释放
		void RegisterScene(Scene* scene, bool activated = true);
		void UnregisterScene(Scene* scene);
		void SetSceneActivated(Scene* scene, bool activated);
		/// 当前已激活的场景列表（供 Layer 在逻辑 Tick 与 Extract 阶段使用）
		const std::vector<Scene*>& GetActivatedScenes() const;

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

		LayerStack m_LayerStack;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer = nullptr;
		RenderQueue* m_RenderQueue = nullptr;
		TimeStep m_TimeStep;
		bool m_Running = true;
		static Application* s_Instance;

		std::vector<std::pair<Scene*, bool>> m_Scenes;  /// scene + activated
		mutable std::vector<Scene*> m_ActivatedScenesCache;
	};

	Application* CreateApplication();

} // namespace Ehu
