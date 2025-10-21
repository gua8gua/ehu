#pragma once
#include "Core.h"

#include "ehupch.h"

using namespace std;

namespace Ehu {
	//�¼�����ö��
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory {
    None = 0,
    EventCategoryApplication = BIT(0),  // Ӧ���¼������������¡���Ⱦ��
    EventCategoryInput = BIT(1),  // ���������¼�
    EventCategoryKeyboard = BIT(2),  // �����¼�
    EventCategoryMouse = BIT(3),  // ����¼���ͨ�ã�
    EventCategoryMouseButton = BIT(4)   // ��갴���¼�
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
