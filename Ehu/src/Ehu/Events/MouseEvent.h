#pragma once
#include "Event.h"

namespace Ehu {
    class EHU_API MouseEvent : public Event {
    public:
        enum class MouseAction {
            ButtonPressed, ButtonReleased, Moved, Scrolled
        };

        MouseEvent(MouseAction action, float x, float y, int button = -1)
            : m_X(x), m_Y(y), m_Button(button) {
            switch (action) {
            case MouseAction::ButtonPressed:
                m_Type = EventType::MouseButtonPressed;
                break;
            case MouseAction::ButtonReleased:
                m_Type = EventType::MouseButtonReleased;
                break;
            case MouseAction::Moved:
                m_Type = EventType::MouseMoved;
                break;
            case MouseAction::Scrolled:
                m_Type = EventType::MouseScrolled;
                break;
            default: m_Type = EventType::None;
            }

            auto category = static_cast<EventCategory>(
                EventCategoryInput | EventCategoryMouse
                );
            if (action == MouseAction::ButtonPressed ||
                action == MouseAction::ButtonReleased) {
                category = static_cast<EventCategory>(category | EventCategoryMouseButton);
            }

            m_Category = category;
        }

        float GetX() const { return m_X; }
        float GetY() const { return m_Y; }
        int GetButton() const { return m_Button; }  // 0=���, 1=�м�, 2=�Ҽ�

        static std::string ButtonToString(int button) {
            switch (button) {
            case 0: return "Left";
            case 1: return "Middle";
            case 2: return "Right";
            default: return "Unknown";
            }
        }

    private:
        float m_X, m_Y;
        int m_Button;
    };
}
