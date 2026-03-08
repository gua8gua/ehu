#include "ehupch.h"
#include "Application.h"
#include "Log.h"
#include "Ref.h"
#include "FileSystem.h"
#include "Platform/Backend/GraphicsBackend.h"
#include "Platform/IO/Input.h"
#include "Platform/Render/RenderContext.h"
#include "Platform/Render/RendererAPI.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderQueue.h"
#include "Renderer/Drawable.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Events/ApplicationEvent.h"
#include "ImGui/DebugLayer.h"
#include "Scene/Scene.h"
#include <chrono>
#include <algorithm>
#include <sstream>

namespace Ehu {

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		EHU_ASSERT(s_Instance == nullptr, "Application already exists");
		s_Instance = this;
		m_Window = Scope<Window>(Window::Create());
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_window\",\"message\":\"Window::Create done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		m_Window->SetEventCallback(EHU_BIND_EVENT_FN(Application::OnEvent));
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_set_callback\",\"message\":\"SetEventCallback done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		Input::Init();
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_input_init\",\"message\":\"Input::Init done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		RenderContext::Init();
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_render_context_init\",\"message\":\"RenderContext::Init done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		RenderContext::SetCurrentWindow(m_Window.get());
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_set_current_window\",\"message\":\"SetCurrentWindow done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		Renderer::Init();
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_renderer_init\",\"message\":\"Renderer::Init done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		m_RenderQueue = CreateScope<RenderQueue>();

		m_ImGuiLayer = new ImGuiLayer(GetGraphicsBackend());
		PushOverlay(m_ImGuiLayer);
		PushOverlay(new DebugLayer());
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:post_push_overlay\",\"message\":\"PushOverlay done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
	}

	Application::~Application() {
		m_LayerStack.Clear();
		m_Scenes.clear();
		m_ActivatedScenesCache.clear();
		m_RenderQueue.reset();
		Input::Shutdown();
		Renderer::Shutdown();
		RenderContext::Shutdown();
	}

	void Application::RegisterScene(Ref<Scene> scene, bool activated) {
		if (!scene) return;
		auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [&scene](const std::pair<Ref<Scene>, bool>& p) { return p.first.get() == scene.get(); });
		if (it != m_Scenes.end()) {
			it->second = activated;
			return;
		}
		m_Scenes.emplace_back(scene, activated);
	}

	void Application::UnregisterScene(Scene* scene) {
		auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [scene](const std::pair<Ref<Scene>, bool>& p) { return p.first.get() == scene; });
		if (it != m_Scenes.end())
			m_Scenes.erase(it);
		m_ActivatedScenesCache.clear();
	}

	void Application::SetSceneActivated(Scene* scene, bool activated) {
		auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [scene](const std::pair<Ref<Scene>, bool>& p) { return p.first.get() == scene; });
		if (it != m_Scenes.end())
			it->second = activated;
		m_ActivatedScenesCache.clear();
	}

	const std::vector<Scene*>& Application::GetActivatedScenes() const {
		m_ActivatedScenesCache.clear();
		for (const auto& p : m_Scenes)
			if (p.second)
				m_ActivatedScenesCache.push_back(p.first.get());
		return m_ActivatedScenesCache;
	}

	void Application::Run() {
		using namespace std::chrono;
		auto runStart = steady_clock::now();
		static int s_frame = 0;

		while (m_Running) {
			if (s_frame == 0) { std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Application.cpp:run_loop\",\"message\":\"first frame start\",\"data\":{\"frame\":0},\"timestamp\":" + std::to_string(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"E\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
			float timeSec = duration_cast<duration<float>>(steady_clock::now() - runStart).count();
			m_TimeStep.Update(timeSec);

			RenderContext::GetAPI().SetViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			RenderContext::GetAPI().BeginRenderPass(nullptr); // 绑定默认帧缓冲，保证 Clear 与 3D/2D 绘制到窗口
			RenderContext::GetAPI().SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			RenderContext::GetAPI().Clear(RendererAPI::ClearColor | RendererAPI::ClearDepth);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(m_TimeStep);

			m_RenderQueue->Clear();
			uint32_t layerIndex = 0;
			for (Layer* layer : m_LayerStack) {
				if (IDrawable* d = dynamic_cast<IDrawable*>(layer)) {
					m_RenderQueue->SetCurrentLayerIndex(layerIndex);
					d->SubmitTo(*m_RenderQueue);
				}
				layerIndex++;
			}
			m_RenderQueue->Sort();
			RenderContext::GetAPI().SetDepthTest(true);
			RenderContext::GetAPI().SetCullFace(false, true);
			m_RenderQueue->FlushAll();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
			if (s_frame == 0) s_frame++;
		}
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(EHU_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(EHU_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.Handled) break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		RenderContext::GetAPI().SetViewport(0, 0, e.GetWidth(), e.GetHeight());
		return false;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

} // namespace Ehu
