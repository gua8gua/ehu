#pragma once
#include "ehupch.h"

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Ehu {

    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key) { return s_instance -> IsKeyPressedImpl(key); }

        static bool IsMouseButtonPressed(MouseCode button){ return  s_instance -> IsMouseButtonPressedImpl(button); }
        static std::pair<float,float> GetMousePosition() { return s_instance -> GetMousePositionImpl(); }
        static float GetMouseX() { return s_instance -> GetMouseXImpl(); }
        static float GetMouseY() { return s_instance -> GetMouseYImpl(); }
    protected:
        virtual bool IsKeyPressedImpl(const KeyCode& key) = 0;
        virtual bool IsMouseButtonPressedImpl(const MouseCode& button)= 0;
        virtual std::pair<float,float> GetMousePositionImpl()= 0;
        virtual float GetMouseXImpl()= 0;
        virtual float GetMouseYImpl()= 0;
    private:
        static Input* s_instance;
    };
}