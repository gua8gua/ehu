#pragma once
#include "Event.h"

#include <functional>
#include <unordered_map>

namespace Ehu {
    class EventHandler {
    public:
        EventHandler() {
            eventHandlers.reserve(20);
        }

        template<typename EventTypeT>
        void RegisterHandler(std::function<void(EventTypeT&)> handler) {
            EventType eventType = EventTypeT().GetEventType();
            auto& handlers = eventHandlers[eventType];
            handlers.push_back(handler);
        }

        void DispatchEvent(Event& event) {
            auto& handlers = eventHandlers[event.GetEventType()];
            for (auto& handler : handlers) {
                handler(event);
            }
        }

    private:
        std::unordered_map<EventType, std::vector<std::function<void(Event&)>>> eventHandlers;
    };
}

