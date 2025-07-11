#pragma once
#include "Event.h"

namespace Ehu {
    class EHU_API KeyEvent : public Event {
    public:
        // 键盘事件类型
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

        // KeyEvent 特有方法
        int GetKeyCode() const { return m_KeyCode; }
        int GetModifiers() const { return m_Modifiers; }
        bool IsModifierDown(int modifier) const { return m_Modifiers & modifier; }

        // 静态方法：键码转字符串（如 VK_A -> "A"）
        static std::string KeyCodeToString(int keyCode) {
            // 示例：实际实现需映射所有键码（参考 VK_ 常量）
            if (keyCode >= 'A' && keyCode <= 'Z') return std::string(1, (char)keyCode);
            if (keyCode >= '0' && keyCode <= '9') return std::string(1, (char)keyCode);
            return "Unknown";
        }

    private:
        int m_KeyCode;      // 物理键码（如 VK_A）
        int m_Modifiers;    // 修饰键掩码（Alt/Ctrl/Shift）
    };
}
