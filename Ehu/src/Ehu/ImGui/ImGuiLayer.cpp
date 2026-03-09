#include "ehupch.h"
#include "ImGuiLayer.h"
#include "ImGuiWindowVisibility.h"
#include "Core/Application.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Ehu {

	// 将完全移出主视口工作区的浮动窗口拉回至最近边缘停靠（仅处理上一帧的窗口数据）
	static void SnapFloatingWindowsBackToViewport() {
		ImGuiContext& g = *ImGui::GetCurrentContext();
		ImGuiViewport* vp = ImGui::GetMainViewport();
		const ImVec2 vMin = vp->WorkPos;
		const ImVec2 vMax(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);
		const ImRect viewRect(vMin, vMax);

		for (int i = 0; i < g.Windows.Size; i++) {
			ImGuiWindow* w = g.Windows[i];
			if (!w || !w->WasActive) continue;
			if (w->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_DockNodeHost | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal | ImGuiWindowFlags_ChildMenu))
				continue;
			if (w->DockId != 0) continue; // 已停靠的不处理
			ImRect winRect(w->Pos, ImVec2(w->Pos.x + w->Size.x, w->Pos.y + w->Size.y));
			if (viewRect.Overlaps(winRect)) continue; // 仍在视口内不处理

			ImVec2 center(w->Pos.x + w->Size.x * 0.5f, w->Pos.y + w->Size.y * 0.5f);
			float dLeft = ImFabs(center.x - vMin.x);
			float dRight = ImFabs(center.x - vMax.x);
			float dTop = ImFabs(center.y - vMin.y);
			float dBottom = ImFabs(center.y - vMax.y);
			float dMin = ImMin(ImMin(dLeft, dRight), ImMin(dTop, dBottom));

			ImVec2 newPos = w->Pos;
			if (dMin == dLeft) {
				newPos.x = vMin.x;
				newPos.y = ImClamp(w->Pos.y, vMin.y, vMax.y - w->Size.y);
			} else if (dMin == dRight) {
				newPos.x = vMax.x - w->Size.x;
				newPos.y = ImClamp(w->Pos.y, vMin.y, vMax.y - w->Size.y);
			} else if (dMin == dTop) {
				newPos.x = ImClamp(w->Pos.x, vMin.x, vMax.x - w->Size.x);
				newPos.y = vMin.y;
			} else {
				newPos.x = ImClamp(w->Pos.x, vMin.x, vMax.x - w->Size.x);
				newPos.y = vMax.y - w->Size.y;
			}
			ImGui::SetWindowPos(w, newPos, ImGuiCond_Always);
		}
	}

	ImGuiLayer::ImGuiLayer(GraphicsBackend backend)
		: Layer("ImGuiLayer")
		, m_Backend(Scope<ImGuiBackend>(ImGuiBackend::Create(backend)))
	{
	}

	ImGuiLayer::~ImGuiLayer() {
		if (m_Backend)
			m_Backend->Shutdown();
	}

	void ImGuiLayer::OnAttach() {
		if (m_Backend)
			m_Backend->Init(&Application::Get().GetWindow());
	}

	void ImGuiLayer::OnDetach() {
	}

	void ImGuiLayer::Begin() {
		if (m_Backend)
			m_Backend->BeginFrame();
	}

	void ImGuiLayer::End() {
		if (m_Backend)
			m_Backend->EndFrame(&Application::Get().GetWindow());
	}

	void ImGuiLayer::OnImGuiRender() {
		// 全屏 Dockspace：背景透明 + 中央节点透传，以便底层 3D 场景可见
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::PopStyleColor();

		// 将上一帧中移出主视口的浮动窗口拉回至最近边缘
		SnapFloatingWindowsBackToViewport();

		// 顶部菜单栏：Window 下提供各窗口的打开/关闭
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Window")) {
				ImGui::MenuItem("Stats", nullptr, &ImGuiWindowVisibility::ShowStats);
				ImGui::MenuItem("Dashboard", "F3", &ImGuiWindowVisibility::ShowDashboard);
				ImGui::Separator();
				ImGui::MenuItem("ImGui Demo", nullptr, &ImGuiWindowVisibility::ShowImGuiDemo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ImGui::ShowDemoWindow(&ImGuiWindowVisibility::ShowImGuiDemo);
	}

	void ImGuiLayer::SetDarkThemeColors() {
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const {
		return 33;
	}

} // namespace Ehu
