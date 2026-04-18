#include "ScriptRegistry.h"
#include <unordered_map>

#ifdef EHU_ENABLE_MONO
#include <mono/metadata/object.h>
#endif

namespace Ehu {

#ifdef EHU_ENABLE_MONO
	namespace {
		// 场景实体键
		struct SceneEntityKey {
			Scene* ScenePtr = nullptr;
			Entity Handle{};
			UUID InstanceId = UUID(0);
			bool operator==(const SceneEntityKey& other) const {
				return ScenePtr == other.ScenePtr && Handle == other.Handle && InstanceId == other.InstanceId;
			}
		};

		// 场景实体键哈希
		struct SceneEntityKeyHash {
			size_t operator()(const SceneEntityKey& key) const {
				size_t h1 = std::hash<void*>()(key.ScenePtr);
				size_t h2 = std::hash<Entity>()(key.Handle);
				size_t h3 = std::hash<UUID>()(key.InstanceId);
				size_t combined = h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
				return combined ^ (h3 + 0x9e3779b9 + (combined << 6) + (combined >> 2));
			}
		};

		// 场景实体键到脚本运行时实例的映射
		std::unordered_map<SceneEntityKey, ScriptRuntimeInstance, SceneEntityKeyHash> s_RuntimeInstances;

		// 释放脚本运行时实例
		void ReleaseInstance(ScriptRuntimeInstance& instance) {
			// 如果句柄不为0，则释放句柄
			if (instance.GCHandle != 0) {
				mono_gchandle_free(instance.GCHandle);
				instance.GCHandle = 0;
			}
			instance.OnCreateInvoked = false;
			instance.BoundClass = nullptr;
		}
	}
#endif

	/**
	 * 获取或创建脚本运行时实例
	 * @param scene 场景
	 * @param entity 实体
	 * @param domain 域
	 * @param klass 类
	 * @return 脚本运行时实例
	 */
	ScriptRuntimeInstance* ScriptRegistry::GetOrCreate(Scene* scene, Entity entity, UUID instanceId, MonoDomain* domain, MonoClass* klass) {
#ifdef EHU_ENABLE_MONO
		if (!scene || !domain || !klass || instanceId.Raw() == 0)
			return nullptr;

		SceneEntityKey key{ scene, entity, instanceId };
		ScriptRuntimeInstance& instance = s_RuntimeInstances[key];// 获取或创建脚本运行时实例
		const bool needRecreate = (instance.GCHandle == 0 || instance.BoundClass != klass);
		if (needRecreate) {
			ReleaseInstance(instance);// 释放脚本运行时实例
			MonoObject* managedObject = mono_object_new(domain, klass);// 创建托管对象
			if (!managedObject)
				return nullptr;
			mono_runtime_object_init(managedObject);
			instance.GCHandle = mono_gchandle_new(managedObject, false);
			instance.BoundClass = klass;
			instance.OnCreateInvoked = false;
		}
		return &instance;
#else
		(void)scene;
		(void)entity;
		(void)instanceId;
		(void)domain;
		(void)klass;
		return nullptr;
#endif
	}

	ScriptRuntimeInstance* ScriptRegistry::Find(Scene* scene, Entity entity, UUID instanceId) {
#ifdef EHU_ENABLE_MONO
		if (!scene || instanceId.Raw() == 0)
			return nullptr;
		SceneEntityKey key{ scene, entity, instanceId };
		auto it = s_RuntimeInstances.find(key);
		return it != s_RuntimeInstances.end() ? &it->second : nullptr;
#else
		(void)scene;
		(void)entity;
		(void)instanceId;
		return nullptr;
#endif
	}

	MonoObject* ScriptRegistry::GetManagedObject(const ScriptRuntimeInstance& instance) {
#ifdef EHU_ENABLE_MONO
		if (instance.GCHandle == 0)
			return nullptr;
		return mono_gchandle_get_target(instance.GCHandle);
#else
		(void)instance;
		return nullptr;
#endif
	}

	void ScriptRegistry::RemoveEntityInstance(Scene* scene, Entity entity, UUID instanceId) {
#ifdef EHU_ENABLE_MONO
		if (!scene || instanceId.Raw() == 0)
			return;
		SceneEntityKey key{ scene, entity, instanceId };
		auto it = s_RuntimeInstances.find(key);
		if (it == s_RuntimeInstances.end())
			return;
		ReleaseInstance(it->second);
		s_RuntimeInstances.erase(it);
#else
		(void)scene;
		(void)entity;
		(void)instanceId;
#endif
	}

	void ScriptRegistry::RemoveEntity(Scene* scene, Entity entity) {
#ifdef EHU_ENABLE_MONO
		if (!scene)
			return;
		for (auto it = s_RuntimeInstances.begin(); it != s_RuntimeInstances.end(); ) {
			if (it->first.ScenePtr == scene && it->first.Handle == entity) {
				ReleaseInstance(it->second);
				it = s_RuntimeInstances.erase(it);
			} else {
				++it;
			}
		}
#else
		(void)scene;
		(void)entity;
#endif
	}

	void ScriptRegistry::RemoveScene(Scene* scene) {
#ifdef EHU_ENABLE_MONO
		if (!scene)
			return;
		for (auto it = s_RuntimeInstances.begin(); it != s_RuntimeInstances.end(); ) {
			if (it->first.ScenePtr == scene) {
				ReleaseInstance(it->second);
				it = s_RuntimeInstances.erase(it);
			} else {
				++it;
			}
		}
#else
		(void)scene;
#endif
	}

	void ScriptRegistry::ResetOnCreateFlags(Scene* scene) {
#ifdef EHU_ENABLE_MONO
		if (!scene)
			return;
		for (auto& [key, instance] : s_RuntimeInstances) {
			if (key.ScenePtr == scene)
				instance.OnCreateInvoked = false;
		}
#else
		(void)scene;
#endif
	}

	void ScriptRegistry::ResetOnCreateFlagsAll() {
#ifdef EHU_ENABLE_MONO
		for (auto& [key, instance] : s_RuntimeInstances)
			instance.OnCreateInvoked = false;
#endif
	}

	void ScriptRegistry::Clear() {
#ifdef EHU_ENABLE_MONO
		for (auto& [key, instance] : s_RuntimeInstances)
			ReleaseInstance(instance);
		s_RuntimeInstances.clear();
#endif
	}

} // namespace Ehu
