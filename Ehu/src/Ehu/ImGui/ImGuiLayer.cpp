#include "ehupch.h"
#include "ImGuiLayer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Ehu {
    ImGuiLayer::ImGuiLayer()
        :Layer("ImGuiLayer"){
    }

    void ImGuiLayer::OnAttach() {
        ImGui::CreatContext();
    }

    void ImGuiLayer::OnDetach() {
    }

    void ImGuiLayer::OnEvent(Event &e) {
    }

    void ImGuiLayer::Begin() {
    }

    void ImGuiLayer::End() {
    }

    void ImGuiLayer::SetDarkThemeColors() {
    }

    uint32_t ImGuiLayer::GetActiveWidgetID() const {
    }
}
