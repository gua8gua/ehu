#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera/Camera.h"
#include <glm/glm.hpp>

namespace Ehu {

	/// 3D 渲染器（平台无关）：占位，后续实现前向/延迟等
	class EHU_API Renderer3D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const PerspectiveCamera& camera);
		static void EndScene();

		/// 占位：提交 3D 物体
		static void Submit(const glm::mat4& transform, const glm::vec4& color);
	};

} // namespace Ehu
