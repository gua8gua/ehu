#pragma once

#include "Core/Core.h"
#include "Core/UUID.h"
#include "ECS/Entity.h"
#include <cstdint>

struct _MonoDomain;
struct _MonoClass;
struct _MonoObject;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;

namespace Ehu {

	class Scene;

	// 脚本运行时实例
	struct ScriptRuntimeInstance {
		uint32_t GCHandle = 0; // 托管对象句柄
		bool OnCreateInvoked = false; // 是否已调用 OnCreate 方法
		MonoClass* BoundClass = nullptr; // 绑定的类
	};

	// 脚本注册表
	// 管理实例的创建、复用、释放
	class EHU_API ScriptRegistry {
	public:
		static ScriptRuntimeInstance* GetOrCreate(Scene* scene, Entity entity, UUID instanceId, MonoDomain* domain, MonoClass* klass);
		static ScriptRuntimeInstance* Find(Scene* scene, Entity entity, UUID instanceId);
		static MonoObject* GetManagedObject(const ScriptRuntimeInstance& instance);

		static void RemoveEntityInstance(Scene* scene, Entity entity, UUID instanceId);
		static void RemoveEntity(Scene* scene, Entity entity);
		static void RemoveScene(Scene* scene);
		static void ResetOnCreateFlags(Scene* scene);
		static void ResetOnCreateFlagsAll();
		static void Clear();
	};

} // namespace Ehu
