#include "ehupch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Ehu {

	void Renderer::Init() {
		Renderer2D::Init();
		Renderer3D::Init();
	}

	void Renderer::Shutdown() {
		Renderer3D::Shutdown();
		Renderer2D::Shutdown();
	}

} // namespace Ehu
