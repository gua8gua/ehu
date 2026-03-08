#include "ehupch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Core/FileSystem.h"
#include <chrono>

namespace Ehu {

	void Renderer::Init() {
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer.cpp:pre_2d_init\",\"message\":\"before Renderer2D::Init\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		Renderer2D::Init();
		{ std::string _line = "{\"sessionId\":\"8e1d5b\",\"location\":\"Renderer.cpp:post_2d_init\",\"message\":\"Renderer2D::Init done\",\"data\":{\"ok\":1},\"timestamp\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) + ",\"hypothesisId\":\"C\"}\n"; FileSystem::AppendTextFile("debug-8e1d5b.log", _line); }
		Renderer3D::Init();
	}

	void Renderer::Shutdown() {
		Renderer3D::Shutdown();
		Renderer2D::Shutdown();
	}

} // namespace Ehu
