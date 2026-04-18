#include "ScriptEngine.h"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/FileSystem.h"
#include "Core/Timer.h"
#include "Core/RuntimeStats.h"
#include "Renderer/Camera/Camera.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ScriptGlue.h"
#include "ScriptRegistry.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <unordered_map>

#ifdef EHU_ENABLE_MONO
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#endif

namespace Ehu {
	MonoDomain* ScriptEngine::s_Domain = nullptr;
	MonoAssembly* ScriptEngine::s_CoreAssembly = nullptr;
	MonoImage* ScriptEngine::s_CoreImage = nullptr;
	MonoAssembly* ScriptEngine::s_AppAssembly = nullptr;
	MonoImage* ScriptEngine::s_AppImage = nullptr;

#ifdef EHU_ENABLE_MONO
	namespace {
		// 托管字段
		struct ManagedField {
			MonoClassField* Field = nullptr;
			ScriptFieldType Type = ScriptFieldType::None;
		};

		// 托管类
		struct ManagedClass {
			std::string FullName;
			std::string NamespaceName;
			std::string ClassName;
			MonoClass* Class = nullptr;
			MonoMethod* OnCreateMethod = nullptr;
			// 0/1 后缀表示 方法参数个数
			MonoMethod* OnUpdateMethod1 = nullptr;
			MonoMethod* OnUpdateMethod0 = nullptr;
			MonoMethod* OnPhysicsUpdateMethod1 = nullptr;
			MonoMethod* OnPhysicsUpdateMethod0 = nullptr;
			MonoMethod* OnDestroyMethod = nullptr;
			std::unordered_map<std::string, ManagedField> Fields;
		};

		// 调用上下文
		struct InvocationContext {
			Scene* ScenePtr = nullptr;
			Entity EntityHandle{};
		};

		std::unordered_map<std::string, ManagedClass> s_ClassCache;// 按NamespaceName缓存托管类
		std::string s_CoreAssemblyPath;// 核心程序集路径
		std::string s_AppAssemblyPath;// 应用程序集路径
		std::filesystem::file_time_type s_AppAssemblyWriteTime{};// 应用程序集写入时间
		std::chrono::steady_clock::time_point s_LastWatchTick{};// 最后一次检查时间
		bool s_HasPendingReloadRequest = false;// 是否有挂起重载请求
		bool s_InternalCallsRegistered = false;// 内部调用注册标志
		thread_local InvocationContext s_InvocationContext{};// 调用上下文

		//从Mono侧类型转换为引擎侧类型
		ScriptFieldType MonoTypeToFieldType(int monoType) {
			switch (monoType) {
			case MONO_TYPE_BOOLEAN: return ScriptFieldType::Bool;
			case MONO_TYPE_I4: return ScriptFieldType::Int;
			case MONO_TYPE_R4: return ScriptFieldType::Float;
			case MONO_TYPE_STRING: return ScriptFieldType::String;
			case MONO_TYPE_VALUETYPE: return ScriptFieldType::Vec3;
			default: return ScriptFieldType::None;
			}
		}

		/**
		* 分割全类名
		* @param fullName 全类名
		* @param outNamespace 命名空间
		* @param outClassName 类名
		* @return 是否成功
		*/
		bool SplitFullClassName(const std::string& fullName, std::string& outNamespace, std::string& outClassName) {
			size_t lastDot = fullName.rfind('.');
			if (lastDot == std::string::npos || lastDot == 0 || lastDot == fullName.size() - 1)
				return false;
			outNamespace = fullName.substr(0, lastDot);
			outClassName = fullName.substr(lastDot + 1);
			return true;
		}

		/**
		* 将托管字符串转换为UTF-8字符串
		* @param managedString 托管字符串
		* @return UTF-8字符串
		*/
		std::string MonoStringToUtf8(MonoString* managedString) {
			if (!managedString)
				return {};
			char* utf8 = mono_string_to_utf8(managedString);
			if (!utf8)
				return {};
			std::string result = utf8;
			mono_free(utf8);
			return result;
		}

		/**
		* 记录托管异常
		* @param exceptionObject 托管异常对象
		*/
		void LogManagedException(MonoObject* exceptionObject) {
			if (!exceptionObject)
				return;
			MonoString* excString = mono_object_to_string(exceptionObject, nullptr);
			std::string text = MonoStringToUtf8(excString);
			if (text.empty())
				text = "Unknown managed exception";
			EHU_CORE_ERROR("[Script] Managed exception: {}", text);
		}

		/**
		* 调用托管方法
		* @param instance 实例对象
		* @param method 方法
		* @param params 参数
		* @return 结果
		*/
		MonoObject* InvokeManagedMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr) {
			if (!method)
				return nullptr;
			MonoObject* exception = nullptr;
			MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);
			if (exception)
				LogManagedException(exception);
			return result;
		}

		/**
		* 查找或创建托管类
		* @param fullClassName 全类名
		* @return 托管类
		*/
		ManagedClass* FindOrCreateManagedClass(const std::string& fullClassName) {
			// 先从缓存中查找
			auto it = s_ClassCache.find(fullClassName);
			if (it != s_ClassCache.end())
				return &it->second;
			if (!ScriptEngine::HasAppAssembly())
				return nullptr;

			// 分割全类名
			std::string namespaceName;
			std::string className;
			if (!SplitFullClassName(fullClassName, namespaceName, className)) {
				EHU_CORE_WARN("[Script] Invalid class name '{}', expected Namespace.Class", fullClassName);
				return nullptr;
			}

			// 从应用程序集获取类
			MonoClass* klass = mono_class_from_name(ScriptEngine::GetAppImage(), namespaceName.c_str(), className.c_str());
			if (!klass) {
				EHU_CORE_WARN("[Script] Class '{}' not found in app assembly", fullClassName);
				return nullptr;
			}

			// 创建托管类
			ManagedClass& managedClass = s_ClassCache[fullClassName];
			managedClass.FullName = fullClassName;
			managedClass.NamespaceName = namespaceName;
			managedClass.ClassName = className;
			managedClass.Class = klass;
			managedClass.OnCreateMethod = mono_class_get_method_from_name(klass, "OnCreate", 0);
			managedClass.OnUpdateMethod1 = mono_class_get_method_from_name(klass, "OnUpdate", 1);
			managedClass.OnUpdateMethod0 = mono_class_get_method_from_name(klass, "OnUpdate", 0);
			managedClass.OnPhysicsUpdateMethod1 = mono_class_get_method_from_name(klass, "OnPhysicsUpdate", 1);
			managedClass.OnPhysicsUpdateMethod0 = mono_class_get_method_from_name(klass, "OnPhysicsUpdate", 0);
			managedClass.OnDestroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);

			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(klass, &iterator)) {
				const uint32_t flags = mono_field_get_flags(field);
				if ((flags & FIELD_ATTRIBUTE_PUBLIC) == 0)// 只处理公共字段
					continue;
				MonoType* monoType = mono_field_get_type(field);
				if (!monoType)
					continue;
				ScriptFieldType fieldType = MonoTypeToFieldType(mono_type_get_type(monoType));// 转换为引擎侧类型
				if (fieldType == ScriptFieldType::None)// 无效类型
					continue;
				const char* fieldName = mono_field_get_name(field);
				if (!fieldName || !fieldName[0])// 无效字段名
					continue;
				managedClass.Fields[fieldName] = ManagedField{ field, fieldType };// 添加到托管类
			}

			return &managedClass;
		}

		/**
		* 确保组件字段默认值
		* @param managedClass 托管类
		* @param scriptComponent 脚本组件
		*/
		void EnsureComponentFieldDefaults(const ManagedClass& managedClass, ScriptInstanceData& scriptInstance) {
			for (const auto& [name, field] : managedClass.Fields) {// 遍历托管类字段
				auto it = scriptInstance.Fields.find(name);
				if (it != scriptInstance.Fields.end()) {// 如果组件字段已存在
					it->second.Type = field.Type;// 更新类型
					continue;
				}
				ScriptFieldValue value{};// 创建新字段值
				value.Type = field.Type;
				switch (field.Type) {
				case ScriptFieldType::Bool: value.BoolValue = false; break;
				case ScriptFieldType::Int: value.IntValue = 0; break;
				case ScriptFieldType::Float: value.FloatValue = 0.0f; break;
				case ScriptFieldType::Vec2: value.Vec2Value = glm::vec2(0.0f); break;
				case ScriptFieldType::Vec3: value.Vec3Value = glm::vec3(0.0f); break;
				case ScriptFieldType::Vec4: value.Vec4Value = glm::vec4(0.0f); break;
				case ScriptFieldType::String: value.StringValue.clear(); break;
				case ScriptFieldType::Entity: value.EntityUUIDValue = 0; break;
				default: break;
				}
				scriptInstance.Fields[name] = std::move(value);
			}
		}

		/**
		* 将组件字段应用到托管对象
		* @param managedClass 托管类
		* @param instance 托管对象
		* @param scriptComponent 脚本组件
		*/
		void ApplyComponentFieldsToManagedObject(const ManagedClass& managedClass, MonoObject* instance, const ScriptInstanceData& scriptInstance) {
			if (!instance)
				return;

			for (const auto& [fieldName, managedField] : managedClass.Fields) {// 遍历托管类字段
				auto valueIt = scriptInstance.Fields.find(fieldName);// 查找组件字段
				if (valueIt == scriptInstance.Fields.end())// 如果组件字段不存在
					continue;

				const ScriptFieldValue& value = valueIt->second;
				switch (managedField.Type) {
				case ScriptFieldType::Bool: {
					uint8_t raw = value.BoolValue ? 1u : 0u;
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::Int: {
					int32_t raw = value.IntValue;
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::Float: {
					float raw = value.FloatValue;
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::Vec2: {
					struct Vec2Raw { float x, y; } raw{ value.Vec2Value.x, value.Vec2Value.y };
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::Vec3: {
					struct Vec3Raw { float x, y, z; } raw{
						value.Vec3Value.x, value.Vec3Value.y, value.Vec3Value.z
					};
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::Vec4: {
					struct Vec4Raw { float x, y, z, w; } raw{
						value.Vec4Value.x, value.Vec4Value.y, value.Vec4Value.z, value.Vec4Value.w
					};
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				case ScriptFieldType::String: {
					MonoString* raw = mono_string_new(ScriptEngine::GetDomain(), value.StringValue.c_str());
					mono_field_set_value(instance, managedField.Field, raw);
					break;
				}
				case ScriptFieldType::Entity: {
					uint64_t raw = value.EntityUUIDValue;
					mono_field_set_value(instance, managedField.Field, &raw);
					break;
				}
				default:
					break;
				}
			}
		}

		/**
		* 将托管对象字段拉取到组件
		* @param managedClass 托管类
		* @param instance 托管对象
		* @param scriptComponent 脚本组件
		*/
		void PullManagedObjectFieldsToComponent(const ManagedClass& managedClass, MonoObject* instance, ScriptInstanceData& scriptInstance) {
			if (!instance)
				return;

			for (const auto& [fieldName, managedField] : managedClass.Fields) {
				ScriptFieldValue& value = scriptInstance.Fields[fieldName];
				value.Type = managedField.Type;

				switch (managedField.Type) {
				case ScriptFieldType::Bool: {
					uint8_t raw = 0;
					mono_field_get_value(instance, managedField.Field, &raw);
					value.BoolValue = (raw != 0);
					break;
				}
				case ScriptFieldType::Int: {
					int32_t raw = 0;
					mono_field_get_value(instance, managedField.Field, &raw);
					value.IntValue = raw;
					break;
				}
				case ScriptFieldType::Float: {
					float raw = 0.0f;
					mono_field_get_value(instance, managedField.Field, &raw);
					value.FloatValue = raw;
					break;
				}
				case ScriptFieldType::Vec2: {
					struct Vec2Raw { float x, y; } raw{};
					mono_field_get_value(instance, managedField.Field, &raw);
					value.Vec2Value = glm::vec2(raw.x, raw.y);
					break;
				}
				case ScriptFieldType::Vec3: {
					struct Vec3Raw { float x, y, z; } raw{};
					mono_field_get_value(instance, managedField.Field, &raw);
					value.Vec3Value = glm::vec3(raw.x, raw.y, raw.z);
					break;
				}
				case ScriptFieldType::Vec4: {
					struct Vec4Raw { float x, y, z, w; } raw{};
					mono_field_get_value(instance, managedField.Field, &raw);
					value.Vec4Value = glm::vec4(raw.x, raw.y, raw.z, raw.w);
					break;
				}
				case ScriptFieldType::String: {
					MonoString* raw = nullptr;
					mono_field_get_value(instance, managedField.Field, &raw);
					value.StringValue = MonoStringToUtf8(raw);
					break;
				}
				case ScriptFieldType::Entity: {
					uint64_t raw = 0;
					mono_field_get_value(instance, managedField.Field, &raw);
					value.EntityUUIDValue = raw;
					break;
				}
				default:
					break;
				}
			}
		}

		/**
		* 作用域调用上下文
		* @param scene 场景
		* @param entity 实体
		*/
		class ScopedInvocationContext {
		public:
			ScopedInvocationContext(Scene* scene, Entity entity)
				: m_Previous(s_InvocationContext) {
				s_InvocationContext.ScenePtr = scene;
				s_InvocationContext.EntityHandle = entity;
			}
			~ScopedInvocationContext() { s_InvocationContext = m_Previous; }
		private:
			InvocationContext m_Previous{};
		};

		/**
		* 刷新程序集监视器
		*/
		void RefreshAssemblyWatcher() {
			// 如果应用程序集路径为空或不存在，则返回
			if (s_AppAssemblyPath.empty() || !std::filesystem::exists(s_AppAssemblyPath))
				return;
			s_AppAssemblyWriteTime = std::filesystem::last_write_time(s_AppAssemblyPath);// 更新应用程序集写入时间
			s_LastWatchTick = std::chrono::steady_clock::now();// 更新最后一次检查时间
		}

		/**
		* 检查程序集更新
		*/
		void TickAssemblyWatcher() {
			// 如果应用程序集路径为空或写入时间为空，则返回
			if (s_AppAssemblyPath.empty() || s_AppAssemblyWriteTime == std::filesystem::file_time_type{})
				return;
			const auto now = std::chrono::steady_clock::now();// 当前时间
			if (s_LastWatchTick.time_since_epoch().count() != 0) {
				auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_LastWatchTick).count();
				if (elapsedMs < 500)// 如果时间差小于500毫秒，则返回
					return;
			}
			s_LastWatchTick = now;// 更新最后一次检查时间			
			if (!std::filesystem::exists(s_AppAssemblyPath))// 如果应用程序集不存在，则返回
				return;
			const auto latestWrite = std::filesystem::last_write_time(s_AppAssemblyPath);// 最新写入时间
			if (latestWrite != s_AppAssemblyWriteTime) {
				s_AppAssemblyWriteTime = latestWrite;// 更新应用程序集写入时间
				ScriptEngine::RequestAssemblyReload();
				EHU_CORE_INFO("[Script] Detected script assembly update, reload requested");// 记录日志
			}
		}

		/**
		* 处理挂起重新加载
		*/
		void HandlePendingReload() {
			if (!s_HasPendingReloadRequest)// 如果没有挂起重新加载请求，则返回
				return;
			s_HasPendingReloadRequest = false;// 设置挂起重新加载请求为false
			if (!s_AppAssemblyPath.empty())
				ScriptEngine::ReloadAssemblies(s_CoreAssemblyPath, s_AppAssemblyPath);// 重新加载程序集
		}

		/**
		* 调用实体销毁
		* @param scene 场景
		* @param entity 实体
		* @param scriptComponent 脚本组件
		*/
		void InvokeDestroyForEntity(Scene* scene, Entity entity, ScriptInstanceData* scriptInstance) {
			if (!scene || !scriptInstance || scriptInstance->ClassName.empty())
				return;

			if (scriptInstance->InstanceId.Raw() == 0)
				scriptInstance->InstanceId = UUID();

			ScriptRuntimeInstance* instance = ScriptRegistry::Find(scene, entity, scriptInstance->InstanceId);
			if (!instance)
				return;

			MonoObject* managedObject = ScriptRegistry::GetManagedObject(*instance);
			if (!managedObject)
				return;

			ManagedClass* managedClass = FindOrCreateManagedClass(scriptInstance->ClassName);
			if (!managedClass || !managedClass->Class)
				return;

			PullManagedObjectFieldsToComponent(*managedClass, managedObject, *scriptInstance);
			if (managedClass->OnDestroyMethod) {
				ScopedInvocationContext context(scene, entity);
				InvokeManagedMethod(managedObject, managedClass->OnDestroyMethod, nullptr);
			}
		}
	}
#endif

	/**
	 * 初始化脚本引擎
	 * @param coreAssemblyPath 核心程序集路径
	 */
	void ScriptEngine::Init(const std::string& coreAssemblyPath) {
#ifdef EHU_ENABLE_MONO
		if (s_Domain)// 如果域已经初始化，则返回
			return;

		mono_set_assemblies_path(nullptr);// 设置程序集路径为空
		s_Domain = mono_jit_init_version("EhuScriptDomain", "v4.0.30319");
		if (!s_Domain) {
			EHU_CORE_ERROR("[Script] Failed to initialize Mono domain");// 记录日志
			return;
		}

		s_InternalCallsRegistered = false;// 设置内部调用注册为false
		RegisterInternalCallsIfNeeded();
		if (!coreAssemblyPath.empty())
			LoadCoreAssembly(coreAssemblyPath);// 加载核心程序集
#else
		(void)coreAssemblyPath;
#endif
	}

	/**
	 * 关闭脚本引擎
	 */
	void ScriptEngine::Shutdown() {
#ifdef EHU_ENABLE_MONO
		if (!s_Domain)
			return;

		ScriptRegistry::Clear();// 清除脚本注册表
		s_ClassCache.clear();// 清除类缓存
		s_AppAssembly = nullptr;// 设置应用程序集为空
		s_AppImage = nullptr;// 设置应用程序图像为空
		s_CoreAssembly = nullptr;// 设置核心程序集为空
		s_CoreImage = nullptr;// 设置核心图像为空
		s_InternalCallsRegistered = false;// 设置内部调用注册为false
		mono_jit_cleanup(s_Domain);// 清理域
		s_Domain = nullptr;// 设置域为空
#endif
	}

	/**
	 * 加载核心程序集
	 * @param assemblyPath 程序集路径
	 */
	bool ScriptEngine::LoadCoreAssembly(const std::string& assemblyPath) {
#ifdef EHU_ENABLE_MONO
		if (!s_Domain || assemblyPath.empty())
			return false;
		// 打开核心程序集
		s_CoreAssembly = mono_domain_assembly_open(s_Domain, assemblyPath.c_str());
		if (!s_CoreAssembly) {
			EHU_CORE_WARN("[Script] Failed to load core assembly '{}'", assemblyPath);
			return false;
		}
		// 获取核心映像
		s_CoreImage = mono_assembly_get_image(s_CoreAssembly);
		s_CoreAssemblyPath = assemblyPath;// 设置核心程序集路径
		return s_CoreImage != nullptr;
#else
		(void)assemblyPath;
		return false;
#endif
	}

	/**
	 * 加载应用程序集
	 * @param assemblyPath 程序集路径
	 */
	void ScriptEngine::LoadAppAssembly(const std::string& assemblyPath) {
#ifdef EHU_ENABLE_MONO
		// 如果域不存在或程序集路径为空，则清除注册表并返回
		if (!s_Domain || assemblyPath.empty()) {
			ScriptRegistry::Clear();// 清除脚本注册表
			s_ClassCache.clear();// 清除类缓存
			s_AppAssembly = nullptr;// 设置应用程序集为空
			s_AppImage = nullptr;// 设置应用程序图像为空
			s_AppAssemblyPath.clear();// 设置应用程序集路径为空
			return;
		}
		// 打开应用程序集
		s_AppAssembly = mono_domain_assembly_open(s_Domain, assemblyPath.c_str());
		if (!s_AppAssembly) {
			EHU_CORE_WARN("[Script] Failed to load app assembly '{}'", assemblyPath);
			s_AppImage = nullptr;
			s_AppAssemblyPath.clear();
			return;
		}
		s_AppImage = mono_assembly_get_image(s_AppAssembly);
		s_AppAssemblyPath = assemblyPath;
		RefreshAssemblyWatcher();
		EHU_CORE_INFO("[Script] Loaded app assembly '{}'", assemblyPath);
#else
		(void)assemblyPath;
#endif
	}

	/**
	 * 重新加载程序集
	 * @param coreAssemblyPath 核心程序集路径
	 * @param appAssemblyPath 应用程序集路径
	 */
	bool ScriptEngine::ReloadAssemblies(const std::string& coreAssemblyPath, const std::string& appAssemblyPath) {
#ifdef EHU_ENABLE_MONO
		const std::string nextCore = coreAssemblyPath.empty() ? s_CoreAssemblyPath : coreAssemblyPath;
		const std::string nextApp = appAssemblyPath.empty() ? s_AppAssemblyPath : appAssemblyPath;
		if (nextApp.empty())
			return false;

		Shutdown();
		Init(nextCore);
		if (!nextCore.empty() && !LoadCoreAssembly(nextCore))
			EHU_CORE_WARN("[Script] Core assembly reload failed: '{}'", nextCore);
		LoadAppAssembly(nextApp);
		return HasAppAssembly();
#else
		(void)coreAssemblyPath;
		(void)appAssemblyPath;
		return false;
#endif
	}

	bool ScriptEngine::ConfigureAssemblies(const std::string& coreAssemblyPath, const std::string& appAssemblyPath) {
#ifdef EHU_ENABLE_MONO
		if (appAssemblyPath.empty()) {
			LoadAppAssembly("");
			return false;
		}

		const bool needReload = !HasAppAssembly() || coreAssemblyPath != s_CoreAssemblyPath || appAssemblyPath != s_AppAssemblyPath;
		if (needReload)
			return ReloadAssemblies(coreAssemblyPath, appAssemblyPath);

		return HasAppAssembly();
#else
		(void)coreAssemblyPath;
		(void)appAssemblyPath;
		return false;
#endif
	}

	bool ScriptEngine::IsInitialized() {
#ifdef EHU_ENABLE_MONO
		return s_Domain != nullptr;
#else
		return false;
#endif
	}

	bool ScriptEngine::HasAppAssembly() {
#ifdef EHU_ENABLE_MONO
		return s_AppImage != nullptr;
#else
		return false;
#endif
	}

	/**
	 * 场景更新
	 * @param scene 场景
	 * @param deltaTime 时间增量
	 */
	void ScriptEngine::OnSceneUpdate(Scene* scene, float deltaTime) {
#ifdef EHU_ENABLE_MONO
		if (!scene || !IsInitialized() || !HasAppAssembly())
			return;

		// 检查程序集更新
		TickAssemblyWatcher();
		// 处理挂起重新加载
		HandlePendingReload();
		if (!HasAppAssembly())
			return;

		Timer scriptTimer;
		World& world = scene->GetWorld();
		world.Each<ScriptComponent>([&](Entity entity, ScriptComponent& script) {// 遍历场景中的脚本组件
			if (!InitializeScriptComponentFields(script))
				return;

			for (ScriptInstanceData& scriptInstanceData : script.Instances) {
				if (scriptInstanceData.ClassName.empty())
					continue;
				if (scriptInstanceData.InstanceId.Raw() == 0)
					scriptInstanceData.InstanceId = UUID();

				ManagedClass* managedClass = FindOrCreateManagedClass(scriptInstanceData.ClassName);
				if (!managedClass || !managedClass->Class)
					continue;

				ScriptRuntimeInstance* scriptInstance = ScriptRegistry::GetOrCreate(scene, entity, scriptInstanceData.InstanceId, s_Domain, managedClass->Class);
				if (!scriptInstance || scriptInstance->GCHandle == 0)
					continue;

				MonoObject* managedObject = ScriptRegistry::GetManagedObject(*scriptInstance);
				if (!managedObject)
					continue;

				{
					ScopedInvocationContext context(scene, entity);// 设置调用上下文
					ApplyComponentFieldsToManagedObject(*managedClass, managedObject, scriptInstanceData);

					if (!scriptInstance->OnCreateInvoked && managedClass->OnCreateMethod) {
						InvokeManagedMethod(managedObject, managedClass->OnCreateMethod, nullptr);// 调用创建方法
						scriptInstance->OnCreateInvoked = true;
					}

					// 调用更新方法
					if (managedClass->OnUpdateMethod1) {
						float dt = deltaTime;
						void* params[1] = { &dt };
						InvokeManagedMethod(managedObject, managedClass->OnUpdateMethod1, params);
					} else if (managedClass->OnUpdateMethod0) {
						InvokeManagedMethod(managedObject, managedClass->OnUpdateMethod0, nullptr);
					}

					PullManagedObjectFieldsToComponent(*managedClass, managedObject, scriptInstanceData);
				}
			}
		});
		RuntimeStats::Get().ScriptingMs += scriptTimer.ElapsedMs();
#else
		(void)scene;
		(void)deltaTime;
#endif
	}

	void ScriptEngine::OnSceneFixedUpdate(Scene* scene, float fixedDeltaTime) {
#ifdef EHU_ENABLE_MONO
		if (!scene || !IsInitialized() || !HasAppAssembly())
			return;

		Timer scriptTimer;
		World& world = scene->GetWorld();
		world.Each<ScriptComponent>([&](Entity entity, ScriptComponent& script) {
			if (!InitializeScriptComponentFields(script))
				return;

			for (ScriptInstanceData& scriptInstanceData : script.Instances) {
				if (scriptInstanceData.ClassName.empty())
					continue;
				if (scriptInstanceData.InstanceId.Raw() == 0)
					scriptInstanceData.InstanceId = UUID();

				ManagedClass* managedClass = FindOrCreateManagedClass(scriptInstanceData.ClassName);
				if (!managedClass || !managedClass->Class)
					continue;

				ScriptRuntimeInstance* scriptInstance = ScriptRegistry::GetOrCreate(scene, entity, scriptInstanceData.InstanceId, s_Domain, managedClass->Class);
				if (!scriptInstance || scriptInstance->GCHandle == 0)
					continue;

				MonoObject* managedObject = ScriptRegistry::GetManagedObject(*scriptInstance);
				if (!managedObject)
					continue;

				{
					ScopedInvocationContext context(scene, entity);
					ApplyComponentFieldsToManagedObject(*managedClass, managedObject, scriptInstanceData);

					if (managedClass->OnPhysicsUpdateMethod1) {
						float dt = fixedDeltaTime;
						void* params[1] = { &dt };
						InvokeManagedMethod(managedObject, managedClass->OnPhysicsUpdateMethod1, params);
					} else if (managedClass->OnPhysicsUpdateMethod0) {
						InvokeManagedMethod(managedObject, managedClass->OnPhysicsUpdateMethod0, nullptr);
					}

					PullManagedObjectFieldsToComponent(*managedClass, managedObject, scriptInstanceData);
				}
			}
		});
		RuntimeStats::Get().ScriptingMs += scriptTimer.ElapsedMs();
#else
		(void)scene;
		(void)fixedDeltaTime;
#endif
	}

	void ScriptEngine::OnSceneDestroyed(Scene* scene) {
#ifdef EHU_ENABLE_MONO
		if (scene) {
			World& world = scene->GetWorld();
			world.Each<ScriptComponent>([&](Entity entity, ScriptComponent& script) {
				for (ScriptInstanceData& scriptInstanceData : script.Instances)
					InvokeDestroyForEntity(scene, entity, &scriptInstanceData);
			});
		}
		ScriptRegistry::RemoveScene(scene);
#else
		(void)scene;
#endif
	}

	void ScriptEngine::OnEntityDestroyed(Scene* scene, Entity entity) {
#ifdef EHU_ENABLE_MONO
		if (scene) {
			World& world = scene->GetWorld();
			ScriptComponent* script = world.GetComponent<ScriptComponent>(entity);
			if (script) {
				for (ScriptInstanceData& scriptInstanceData : script->Instances)
					InvokeDestroyForEntity(scene, entity, &scriptInstanceData);
			}
		}
		ScriptRegistry::RemoveEntity(scene, entity);
#else
		(void)scene;
		(void)entity;
#endif
	}

	bool ScriptEngine::InitializeScriptComponentFields(ScriptComponent& scriptComponent) {
#ifdef EHU_ENABLE_MONO
		if (scriptComponent.Instances.empty())
			return false;
		bool initializedAny = false;
		for (ScriptInstanceData& scriptInstanceData : scriptComponent.Instances) {
			if (scriptInstanceData.ClassName.empty())
				continue;
			if (scriptInstanceData.InstanceId.Raw() == 0)
				scriptInstanceData.InstanceId = UUID();
			ManagedClass* managedClass = FindOrCreateManagedClass(scriptInstanceData.ClassName);
			if (!managedClass)
				continue;
			EnsureComponentFieldDefaults(*managedClass, scriptInstanceData);
			initializedAny = true;
		}
		return initializedAny;
#else
		(void)scriptComponent;
		return false;
#endif
	}

	bool ScriptEngine::GetScriptFieldInfos(const std::string& fullClassName, std::vector<ScriptFieldInfo>& outFields) {
		outFields.clear();
#ifdef EHU_ENABLE_MONO
		ManagedClass* managedClass = FindOrCreateManagedClass(fullClassName);
		if (!managedClass)
			return false;
		outFields.reserve(managedClass->Fields.size());
		for (const auto& [name, field] : managedClass->Fields)
			outFields.push_back({ name, field.Type });
		std::sort(outFields.begin(), outFields.end(), [](const ScriptFieldInfo& a, const ScriptFieldInfo& b) {
			return a.Name < b.Name;
		});
		return true;
#else
		(void)fullClassName;
		return false;
#endif
	}

	void ScriptEngine::RequestAssemblyReload() {
#ifdef EHU_ENABLE_MONO
		s_HasPendingReloadRequest = true;
#endif
	}

	MonoDomain* ScriptEngine::GetDomain() {
		return s_Domain;
	}

	MonoImage* ScriptEngine::GetAppImage() {
		return s_AppImage;
	}

	Scene* ScriptEngine::GetInvocationScene() {
#ifdef EHU_ENABLE_MONO
		return s_InvocationContext.ScenePtr;
#else
		return nullptr;
#endif
	}

	Entity ScriptEngine::GetInvocationEntity() {
#ifdef EHU_ENABLE_MONO
		return s_InvocationContext.EntityHandle;
#else
		return Entity{};
#endif
	}

	void ScriptEngine::OnPlayModeChanged(bool isPlayMode) {
#ifdef EHU_ENABLE_MONO
		if (!isPlayMode) {
			for (Scene* scene : Application::Get().GetActivatedScenes()) {
				if (!scene)
					continue;
				World& world = scene->GetWorld();
				world.Each<ScriptComponent>([&](Entity entity, ScriptComponent& script) {
					for (ScriptInstanceData& scriptInstanceData : script.Instances)
						InvokeDestroyForEntity(scene, entity, &scriptInstanceData);
				});
				ScriptRegistry::RemoveScene(scene);
			}
			return;
		}
		ScriptRegistry::ResetOnCreateFlagsAll();
#else
		(void)isPlayMode;
#endif
	}

	namespace {
		class ScriptRuntimeBridge final : public IScriptRuntime {
		public:
			void OnSceneUpdate(Scene* scene, float deltaTime) override { ScriptEngine::OnSceneUpdate(scene, deltaTime); }
			void OnSceneFixedUpdate(Scene* scene, float fixedDeltaTime) override { ScriptEngine::OnSceneFixedUpdate(scene, fixedDeltaTime); }
			void OnSceneDestroyed(Scene* scene) override { ScriptEngine::OnSceneDestroyed(scene); }
			void OnEntityDestroyed(Scene* scene, Entity entity) override { ScriptEngine::OnEntityDestroyed(scene, entity); }
			void OnPlayModeChanged(bool isPlayMode) override { ScriptEngine::OnPlayModeChanged(isPlayMode); }
		};
	}

	IScriptRuntime* ScriptEngine::GetRuntimeBridge() {
		static ScriptRuntimeBridge s_Bridge;
		return &s_Bridge;
	}

	void ScriptEngine::RegisterInternalCallsIfNeeded() {
#ifdef EHU_ENABLE_MONO
		if (s_InternalCallsRegistered || !s_Domain)
			return;
		ScriptGlue::RegisterInternalCalls();
		s_InternalCallsRegistered = true;
#endif
	}

} // namespace Ehu

