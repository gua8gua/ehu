#pragma once

#include "Core/Layer.h"
#include "Core/TimeStep.h"
#include "Drawable.h"

namespace Ehu {

	class Scene;

	/// 引擎提供的层：不持有 Scene；Phase1 驱动所有已激活 Scene::OnUpdate，Phase2 从激活场景中提取归属本层的实体提交
	/// 可渲染实体须设置 SetRenderLayer(this)；子类可重写 OnUpdateScene(TimeStep) 做额外逻辑
	class EHU_API SceneLayer : public Layer, public IDrawable {
	public:
		/// 不持有 Scene/Camera；Scene 通过 Application::RegisterScene 注册，相机由 Scene 管理并选择主相机
		explicit SceneLayer(const std::string& name = "SceneLayer");
		virtual ~SceneLayer() override;

		void OnAttach() override;
		void OnUpdate(const TimeStep& timestep) override;
		void SubmitTo(RenderQueue& queue) const override;
		void SubmitTo(RenderQueue& queue, class Camera* viewCameraOverride) const override;

	protected:
		/// 子类重写以每帧做额外逻辑（场景逻辑以 Scene::OnUpdate 为主）
		virtual void OnUpdateScene(const TimeStep& timestep) { (void)timestep; }
	};

} // namespace Ehu
