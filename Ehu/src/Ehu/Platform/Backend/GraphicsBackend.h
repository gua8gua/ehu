#pragma once

#include "ehupch.h"

namespace Ehu {

	/// 图形/窗口后端类型，用于环境选择与工厂分发
	enum class GraphicsBackend {
		None = 0,
		OpenGL_GLFW  // 当前唯一实现：OpenGL + GLFW
	};

	/// 根据当前环境选择后端（可读环境变量 EHU_GRAPHICS_BACKEND，暂仅支持 OpenGL_GLFW）
	GraphicsBackend GetGraphicsBackend();

} // namespace Ehu
