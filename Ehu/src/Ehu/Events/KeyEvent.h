#pragma once
#include "Event.h"

namespace Ehu {
    class EHU_API KeyEvent : public Event {
    public:
        // �����¼�����
        enum class KeyAction { Pressed, Released, Typed };

        KeyEvent(KeyAction action, int keyCode, int modifiers = 0)
            : m_KeyCode(keyCode), m_Modifiers(modifiers) {
            switch (action) {
            case KeyAction::Pressed:
                m_Type = EventType::KeyPressed;
                break;
            case KeyAction::Released:
                m_Type = EventType::KeyReleased;
                break;
            case KeyAction::Typed:
                m_Type = EventType::KeyTyped;
                break;
            default: m_Type = EventType::None;
            }
            m_Category = static_cast<EventCategory>(
                EventCategoryInput | EventCategoryKeyboard
                );
        }

        // KeyEvent ���з���
        int GetKeyCode() const { return m_KeyCode; }
        int GetModifiers() const { return m_Modifiers; }
        bool IsModifierDown(int modifier) const { return m_Modifiers & modifier; }

        // ��̬����������ת�ַ������� VK_A -> "A"��
        static std::string KeyCodeToString(int keyCode) {
            // ʾ����ʵ��ʵ����ӳ�����м��루�ο� VK_ ������
            if (keyCode >= 'A' && keyCode <= 'Z') return std::string(1, (char)keyCode);
            if (keyCode >= '0' && keyCode <= '9') return std::string(1, (char)keyCode);
            return "Unknown";
        }

    private:
        int m_KeyCode;      // ������루�� VK_A��
        int m_Modifiers;    // ���μ����루Alt/Ctrl/Shift��
    };
}
