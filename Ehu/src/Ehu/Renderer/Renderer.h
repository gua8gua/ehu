#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	/// 渲染器层统一入口：初始化/关闭与调度
	class EHU_API Renderer {
	public:
		/// 初始化整个渲染器层（Renderer2D、Renderer3D 等），应在 Application 构造中、窗口与 RendererAPI 就绪后调用
		static void Init();
		/// 关闭渲染器层，释放资源
		static void Shutdown();
	};

} // namespace Ehu
