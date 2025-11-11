#pragma once
#include "Event.h"

namespace Ehu {
    class EHU_API AppTickEvent : public Event {
    public:
        AppTickEvent() {
            m_Type = EventType::AppTick;
            m_Category = EventCategoryApplication;
            eventName = "AppTickEvent";
        }
    };

    class EHU_API AppUpdateEvent : public Event {
    public:
        AppUpdateEvent() {
            m_Type = EventType::AppUpdate;
            m_Category = EventCategoryApplication;
            eventName = "AppUpdateEvent";
        }
    };

    class EHU_API AppRenderEvent : public Event {
    public:
        AppRenderEvent() {
            m_Type = EventType::AppRender;
            m_Category = EventCategoryApplication;
            eventName = "AppRenderEvent";
        }
    };
}


