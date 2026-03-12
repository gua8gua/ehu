#include "Ehu.h"
#include "EditorLayer.h"

namespace Ehu {

	class EditorApp : public Application {
	public:
		EditorApp() {
			if (ImGuiLayer* imgui = GetImGuiLayer())
				imgui->SetDrawMainMenuBar(false);
			PushOverlay(new EditorLayer());
		}
	};

} // namespace Ehu

Ehu::Application* Ehu::CreateApplication() {
	return new Ehu::EditorApp();
}
