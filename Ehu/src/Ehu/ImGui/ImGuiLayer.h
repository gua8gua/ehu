#pragma once

#include "Core/Layer.h"
#include "ImGuiBackend.h"
#include "Platform/GraphicsBackend.h"
#include "Events/ApplicationEvent.h"
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

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;

	private:
		std::unique_ptr<ImGuiBackend> m_Backend;
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}
