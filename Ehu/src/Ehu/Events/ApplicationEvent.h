#pragma once
#include "Event.h"

namespace Ehu {
    // 应用每帧开始的更新事件（逻辑、系统状态）
    class EHU_API AppTickEvent : public Event {
    public:
        AppTickEvent() {
            m_Type = EventType::AppTick;
            m_Category = EventCategoryApplication;
            eventName = "AppTickEvent";
        }
    };

    // 应用主更新事件（游戏逻辑处理）
    class EHU_API AppUpdateEvent : public Event {
    public:
        AppUpdateEvent() {
            m_Type = EventType::AppUpdate;
            m_Category = EventCategoryApplication;
            eventName = "AppUpdateEvent";
        }
    };

    // 应用渲染事件（通常在所有逻辑更新后发出）
    class EHU_API AppRenderEvent : public Event {
    public:
        AppRenderEvent() {
            m_Type = EventType::AppRender;
            m_Category = EventCategoryApplication;
            eventName = "AppRenderEvent";
        }
    };
}


