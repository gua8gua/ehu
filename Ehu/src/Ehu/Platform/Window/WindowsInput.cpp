
#include "WindowsInput.h"
#include "Ehu/Application.h"
#include "Ehu/Input.h"
#include <GLFW/glfw3.h>

namespace Ehu {
    Input* Input::s_instance = new WindowsInput();

    bool WindowsInput::IsKeyPressedImpl(const KeyCode& key) {
        // 获取 GLFW 窗口指针
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WindowsInput::IsMouseButtonPressedImpl(const MouseCode& button) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        // 调用 GLFW 获取鼠标按键状态
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> WindowsInput::GetMousePositionImpl() {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        return { (float)xpos, (float)ypos };
    }

    float WindowsInput::GetMouseXImpl() {
        auto [x, y] = GetMousePositionImpl();
        return x;
    }

    float WindowsInput::GetMouseYImpl() {
        auto [x, y] = GetMousePositionImpl();
        return y;
    }
}