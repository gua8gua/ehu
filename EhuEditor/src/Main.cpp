#include "Core/Application.h"
#include "EntryPoint.h"
#include "EditorLayer.h"
#include "ImGui/ImGuiLayer.h"

namespace Ehu {

	class EditorApp : public Application {
	public:
		EditorApp(const ApplicationSpecification& specification)
			: Application(specification) {
			if (ImGuiLayer* imgui = EnableImGui())
				imgui->SetDrawMainMenuBar(false);
			PushOverlay(new EditorLayer());
		}
	};

} // namespace Ehu

Ehu::Application* Ehu::CreateApplication(Ehu::ApplicationCommandLineArgs args) {
	Ehu::ApplicationSpecification specification;
	specification.Name = "EhuEditor";
	specification.CommandLineArgs = args;
	specification.EnableMainWindowSceneRendering = false;
	return new Ehu::EditorApp(specification);
}
