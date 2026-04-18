#pragma once

#include "Core/Layer.h"
#include "Core/Ref.h"
#include "ImGuiBackend.h"
#include "Platform/Backend/GraphicsBackend.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Ehu {

	class EHU_API ImGuiLayer : public Layer
	{
	public:
		explicit ImGuiLayer(GraphicsBackend backend);
		~ImGuiLayer() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		/// 是否绘制默认主菜单栏（Window 等）；编辑器模式下可设为 false，由 EditorLayer 绘制唯一菜单栏
		void SetDrawMainMenuBar(bool draw) { m_DrawMainMenuBar = draw; }
		bool GetDrawMainMenuBar() const { return m_DrawMainMenuBar; }

		/// 当前主视口 Dockspace 的根节点 ID（DockSpaceOverViewport 返回值），供 EditorLayer DockBuilder 使用
		unsigned int GetDockspaceRootId() const { return m_DockspaceRootId; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;

	private:
		Scope<ImGuiBackend> m_Backend;
		bool m_BlockEvents = true;
		bool m_DrawMainMenuBar = false;
		unsigned int m_DockspaceRootId = 0;
		float m_Time = 0.0f;
	};

}
