#include "ehupch.h"
#include "Application.h"
#include "Log.h"
#include "Ref.h"
#include "RuntimeStats.h"
#include "Platform/IO/Input.h"
#include "Platform/Render/RenderContext.h"
#include "Platform/Render/RendererAPI.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderQueue.h"
#include "Renderer/Drawable.h"
#include "Events/ApplicationEvent.h"
#include "Scripting/ScriptEngine.h"
#include "Scripting/IScriptRuntime.h"
#include "Core/Timer.h"
#include "Platform/MemoryStats.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Project/Project.h"
#include "ECS/LayerRegistry.h"
#include <chrono>
#include <algorithm>
#include <filesystem>

namespace Ehu {

	Application* Application::s_Instance = nullptr;

	namespace {
		std::string ResolveProjectRelativePath(const Project& project, const std::string& path) {
			if (path.empty())
				return {};
			std::filesystem::path p(path);
			if (p.is_absolute())
				return p.lexically_normal().string();
			std::filesystem::path root(project.GetProjectDirectory());
			return (root / p).lexically_normal().string();
		}
	}

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification) {
		EHU_ASSERT(s_Instance == nullptr, "Application already exists");
		s_Instance = this;
		m_MainWindowSceneRenderingEnabled = m_Specification.EnableMainWindowSceneRendering;

		if (!m_Specification.WorkingDirectory.empty()) {
			std::error_code ec;
			std::filesystem::current_path(m_Specification.WorkingDirectory, ec);
		}

		// 窗口和渲染
		WindowProps props(m_Specification.Name.empty() ? "Ehu Application" : m_Specification.Name);
		m_Window = Scope<Window>(Window::Create(props));
		m_Window->SetEventCallback(EHU_BIND_EVENT_FN(Application::OnEvent));
		Input::Init();
		RenderContext::Init();
		RenderContext::SetCurrentWindow(m_Window.get());
		Renderer::Init();
		// 脚本引擎初始化。实际程序集在项目加载后再配置。
		ScriptEngine::Init("");
		Scene::SetScriptRuntime(ScriptEngine::GetRuntimeBridge());
		m_RenderQueue = CreateScope<RenderQueue>();

	}

	Application::~Application() {
		m_LayerStack.Clear();
		m_Scenes.clear();
		m_ActivatedScenesCache.clear();
		m_RenderQueue.reset();
		Scene::SetScriptRuntime(nullptr);
		ScriptEngine::Shutdown();
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
		if (m_Window)
			scene->OnViewportResize(m_Window->GetWidth(), m_Window->GetHeight());
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

	bool Application::IsSceneActivated(Scene* scene) const {
		if (!scene)
			return false;
		auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [scene](const std::pair<Ref<Scene>, bool>& p) { return p.first.get() == scene; });
		return it != m_Scenes.end() && it->second;
	}

	Ref<Scene> Application::FindSceneRef(Scene* scene) const {
		if (!scene)
			return nullptr;
		auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [scene](const std::pair<Ref<Scene>, bool>& p) { return p.first.get() == scene; });
		return it != m_Scenes.end() ? it->first : nullptr;
	}

	void Application::ConfigureProject(Project& project) {
		const ProjectConfig& cfg = project.GetConfig();
		LayerRegistry::ResetToBuiltins();
		for (const RenderChannelConfigEntry& entry : cfg.RenderChannels) {
			if (!entry.Name.empty())
				LayerRegistry::RegisterRenderChannelWithId(entry.Name, entry.Id);
		}
		for (const CollisionLayerConfigEntry& entry : cfg.CollisionLayers) {
			if (entry.Name.empty() || entry.Bit == 0u)
				continue;
			uint32_t defaultMask = 0xFFFFFFFFu;
			auto maskIt = cfg.CollisionDefaultMasks.find(entry.Bit);
			if (maskIt != cfg.CollisionDefaultMasks.end())
				defaultMask = maskIt->second;
			LayerRegistry::RegisterCollisionLayerWithBit(entry.Name, entry.Bit, defaultMask);
		}

		const std::string coreAssembly = ResolveProjectRelativePath(project, cfg.ScriptCoreAssemblyPath);
		const std::string appAssembly = ResolveProjectRelativePath(project, cfg.ScriptAppAssemblyPath);
		ScriptEngine::ConfigureAssemblies(coreAssembly, appAssembly);
	}

	Ref<Scene> Application::LoadSceneFromProject(Project& project, const std::string& relativePath, bool activated) {
		if (relativePath.empty())
			return nullptr;
		SceneSerializer serializer;
		const std::string scenePath = project.GetAssetFileSystemPath(relativePath);
		Ref<Scene> scene = CreateRef<Scene>();
		if (!serializer.Deserialize(scene.get(), scenePath))
			return nullptr;
		RegisterScene(scene, activated);
		return scene;
	}

	const std::vector<Scene*>& Application::GetActivatedScenes() const {
		m_ActivatedScenesCache.clear();
		for (const auto& p : m_Scenes)
			if (p.second)
				m_ActivatedScenesCache.push_back(p.first.get());
		return m_ActivatedScenesCache;
	}

	uint32_t Application::ActivateScenesFromProject(Project& project) {
		const ProjectConfig& cfg = project.GetConfig();
		ConfigureProject(project);
		uint32_t loaded = 0;
		for (const ProjectSceneEntry& entry : cfg.Scenes) {
			if (!entry.Active) continue;
			if (LoadSceneFromProject(project, entry.RelativePath, true))
				loaded++;
		}
		return loaded;
	}

	void Application::DeactivateAllScenes() {
		m_Scenes.clear();
		m_ActivatedScenesCache.clear();
		LayerRegistry::ResetToBuiltins();
		ScriptEngine::LoadAppAssembly("");
	}

	void Application::SetPlayMode(bool play) {
		if (m_PlayMode == play)
			return;
		for (Scene* scene : GetActivatedScenes()) {
			if (!scene)
				continue;
			if (play)
				scene->OnRuntimeStart();
			else
				scene->OnRuntimeStop();
		}
		m_PlayMode = play;
		if (!play)
			m_FixedAccumulator = 0.0f;
		if (IScriptRuntime* runtime = ScriptEngine::GetRuntimeBridge())
			runtime->OnPlayModeChanged(play);
	}

	void Application::Close() {
		m_Running = false;
	}

	void Application::SubmitToMainThread(const std::function<void()>& function) {
		if (!function)
			return;
		std::scoped_lock lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.push_back(function);
	}

	void Application::ExecuteMainThreadQueue() {
		std::vector<std::function<void()>> queue;
		{
			std::scoped_lock lock(m_MainThreadQueueMutex);
			queue.swap(m_MainThreadQueue);
		}
		for (const auto& fn : queue)
			fn();
	}

	void Application::Run() {
		using namespace std::chrono;
		auto runStart = steady_clock::now();

		while (m_Running) {
			float timeSec = duration_cast<duration<float>>(steady_clock::now() - runStart).count();
			m_TimeStep.Update(timeSec);

			RuntimeStats& dashStats = RuntimeStats::Get();
			dashStats.SnapshotRenderingCounters();  // 上一帧的纹理/着色器计数写入展示并清零
			dashStats.GpuTimeMs = RenderContext::GetAPI().GetLastGpuTimeMs();
			dashStats.ScriptingMs = 0.0f;
			ExecuteMainThreadQueue();

			if (m_Minimized) {
				m_Window->OnUpdate();
				continue;
			}

			RendererAPI& api = RenderContext::GetAPI();
			api.SetViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			api.BeginRenderPass(nullptr); // 绑定默认帧缓冲，保证 Clear 与后续 ImGui 绘制到窗口
			api.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			api.Clear(RendererAPI::ClearColor | RendererAPI::ClearDepth);

			Timer updateTimer;
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(m_TimeStep);
			dashStats.UpdateMs = updateTimer.ElapsedMs();

			if (m_PlayMode) {
				m_FixedAccumulator += m_TimeStep.GetDeltaTime();
				const int maxCatchUpSteps = 5;
				int fixedSteps = 0;
				while (m_FixedAccumulator >= m_FixedDeltaTime && fixedSteps < maxCatchUpSteps) {
					const std::vector<Scene*>& scenes = GetActivatedScenes();
					for (Scene* scene : scenes)
						scene->OnFixedUpdate(m_FixedDeltaTime);
					m_FixedAccumulator -= m_FixedDeltaTime;
					fixedSteps++;
				}
			}

			m_RenderQueue->Clear();
			Timer submitTimer;
			if (m_MainWindowSceneRenderingEnabled) {
				uint32_t layerIndex = 0;
				for (Layer* layer : m_LayerStack) {
					if (IDrawable* d = dynamic_cast<IDrawable*>(layer)) {
						m_RenderQueue->SetCurrentLayerIndex(layerIndex);
						d->SubmitTo(*m_RenderQueue);
					}
					layerIndex++;
				}
			}
			m_RenderQueue->Sort();
			dashStats.RenderSubmitMs = m_MainWindowSceneRenderingEnabled ? submitTimer.ElapsedMs() : 0.0f;

			if (m_MainWindowSceneRenderingEnabled) {
				api.SetDepthTest(true);
				api.SetCullFace(false, true);
				api.BeginGpuTiming();
			}
			m_RenderQueue->FlushAll();
			if (m_MainWindowSceneRenderingEnabled)
				api.EndGpuTiming();

			// 写入仪表盘：Timing、来自 RenderQueue 的渲染统计、Active Entities
			float dtMs = m_TimeStep.GetDeltaTime() * 1000.0f;
			dashStats.FrameTimeMs = dtMs;
			dashStats.CpuTimeMs = dtMs;
			dashStats.PhysicsMs = 0.0f;

			const RenderQueue* queue = GetRenderQueue();
			if (queue) {
				const RenderStats& rs = queue->GetLastFrameStats();
				dashStats.DrawCalls2D = rs.DrawCalls2D;
				dashStats.DrawCalls3D = rs.DrawCalls3D;
				dashStats.Triangles2D = rs.Triangles2D;
				dashStats.Triangles3D = rs.Triangles3D;
				dashStats.Vertices2D = rs.Triangles2D * 2;  // 每四边形 2 三角形、4 顶点
				dashStats.Vertices3D = rs.Triangles3D * 3;
			}

			uint32_t activeEntities = 0;
			for (Scene* scene : GetActivatedScenes())
				activeEntities += static_cast<uint32_t>(scene->GetEntities().size());
			dashStats.ActiveEntities = activeEntities;

			dashStats.HeapBytes = MemoryStats::GetProcessHeapBytes();
			dashStats.VramBytes = RenderContext::GetAPI().GetVramBytes();

			dashStats.PushHistory();

			if (m_ImGuiLayer && m_BeginImGuiFrame && m_EndImGuiFrame) {
				m_BeginImGuiFrame(m_ImGuiLayer);
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_EndImGuiFrame(m_ImGuiLayer);
			}

			m_Window->OnUpdate();
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
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		m_Minimized = (e.GetWidth() == 0 || e.GetHeight() == 0);
		if (m_Minimized)
			return false;
		RenderContext::GetAPI().SetViewport(0, 0, e.GetWidth(), e.GetHeight());
		for (Scene* scene : GetActivatedScenes()) {
			if (scene)
				scene->OnViewportResize(e.GetWidth(), e.GetHeight());
		}
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
