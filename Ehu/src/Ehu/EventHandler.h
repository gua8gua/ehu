#pragma once
#include "Events/Event.h"

#include <ehupch.h>

namespace Ehu {
    class EventHandler {
    public:
        static void RegisterHandler(EventType type, std::function<void(Event&)> handler) {
            eventHandlers[type].push_back(handler);
        }

        static void DispatchEvent(Event& event) {
            auto& handlers = eventHandlers[event.GetEventType()];
            for (auto& handler : handlers) {
                handler(event);
            }
        }

    private:
        static std::unordered_map<EventType, std::vector<std::function<void(Event&)>>> eventHandlers;
    };
}

