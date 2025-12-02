#pragma once

#include "Ehu/Layer.h"

#include "Ehu/Events/ApplicationEvent.h"
#include "Ehu/Events/KeyEvent.h"
#include "Ehu/Events/MouseEvent.h"

namespace Ehu {

	class EHU_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnEvent(Event& e) override;


		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}
