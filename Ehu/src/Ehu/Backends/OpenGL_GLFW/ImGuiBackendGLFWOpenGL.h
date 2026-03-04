#pragma once

#include "ImGui/ImGuiBackend.h"

namespace Ehu {

	class ImGuiBackendGLFWOpenGL : public ImGuiBackend {
	public:
		void Init(Window* window) override;
		void BeginFrame() override;
		void EndFrame(Window* window) override;
		void Shutdown() override;
	};

} // namespace Ehu
