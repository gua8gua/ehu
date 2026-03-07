#pragma once

#include "Core/Core.h"

namespace Ehu {

	class RenderQueue;  // 前向声明，SubmitTo 实现处包含 RenderQueue.h
	class Camera;       // 前向声明，IProvidesCamera::GetCamera 实现处包含 Camera/Camera.h

	/// 可绘制对象接口：将自身绘制命令提交到渲染队列，由统一流程排序后绘制
	class EHU_API IDrawable {
	public:
		virtual ~IDrawable() = default;
		virtual void SubmitTo(RenderQueue& queue) const = 0;
	};

	/// 提供当前帧相机（用于统一 Flush 时的 ViewProjection）；Layer/Scene 可实现
	class EHU_API IProvidesCamera {
	public:
		virtual ~IProvidesCamera() = default;
		virtual Camera* GetCamera() const = 0;
	};

} // namespace Ehu
