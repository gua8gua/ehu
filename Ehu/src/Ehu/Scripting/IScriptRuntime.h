#pragma once

#include "Core/Core.h"
#include "ECS/Entity.h"

namespace Ehu {

	class Scene;

	/// Scene 与脚本系统之间的桥接接口，避免 Scene 直接依赖 ScriptEngine 静态实现
	class EHU_API IScriptRuntime {
	public:
		virtual ~IScriptRuntime() = default;
		virtual void OnSceneUpdate(Scene* scene, float deltaTime) = 0;
		/// 固定时间步（与 Application 内累加器一致），用于 OnPhysicsUpdate 等
		virtual void OnSceneFixedUpdate(Scene* scene, float fixedDeltaTime) = 0;
		virtual void OnSceneDestroyed(Scene* scene) = 0;
		virtual void OnEntityDestroyed(Scene* scene, Entity entity) = 0;
		virtual void OnPlayModeChanged(bool isPlayMode) = 0;
	};

} // namespace Ehu
