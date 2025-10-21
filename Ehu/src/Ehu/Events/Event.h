#pragma once
#include "Core.h"

#include "ehupch.h"

using namespace std;

namespace Ehu {
	//事件类型枚举
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory {
    None = 0,
    EventCategoryApplication = BIT(0),  // 应用事件（启动、更新、渲染）
    EventCategoryInput = BIT(1),  // 所有输入事件
    EventCategoryKeyboard = BIT(2),  // 键盘事件
    EventCategoryMouse = BIT(3),  // 鼠标事件（通用）
    EventCategoryMouseButton = BIT(4)   // 鼠标按键事件
};

class EHU_API Event {
    public:
        virtual ~Event() = default; 

        EventType GetEventType() const { return m_Type; }
        EventCategory GetEventCategory() const { return m_Category; }

        bool IsHandled() const { return handled; }
        void SetHandled(bool inHandled) { handled = inHandled; }
        string GetNmae() const { return eventName; }
        void SetName(string name) { eventName = name; }

        inline bool IsInCategory(EventCategory category) const {
            return GetEventCategory() & category;
        }
    protected:
        bool handled = 0;
        string eventName;

        EventType m_Type;
        EventCategory m_Category;
};
}
