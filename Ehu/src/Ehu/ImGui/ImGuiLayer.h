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
		bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent &e);
		bool OnMouseMoved(MouseMovedEvent &e);
		bool OnMouseScrolled(MouseScrolledEvent &e);
		bool OnKeyPressed(KeyPressedEvent &e);
		bool OnKeyReleased(KeyReleasedEvent &e);
		bool OnKeyTyped(KeyTypedEvent &e);
		bool OnWindowResized(WindowResizeEvent &e);

	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}
