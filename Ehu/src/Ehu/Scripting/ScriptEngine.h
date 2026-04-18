#pragma once

#include "Core/Core.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "IScriptRuntime.h"
#include <string>
#include <vector>

// 前向声明 Mono 类型，避免强依赖具体头文件路径；真正的定义来自 Mono embedding API 头
struct _MonoDomain;
struct _MonoAssembly;
struct _MonoImage;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;

namespace Ehu {

	class Scene;

	struct ScriptFieldInfo {
		std::string Name;
		ScriptFieldType Type = ScriptFieldType::None;
	};

	/// C# 脚本运行时封装（基于 Mono embedding API）
	class EHU_API ScriptEngine {
	public:
		/// 初始化脚本运行时（若未启用 EHU_ENABLE_MONO 可为空实现）
		static void Init(const std::string& coreAssemblyPath);
		/// 关闭脚本运行时并释放资源
		static void Shutdown();

		/// 加载游戏脚本程序集（例如 GameScript.dll）
		static bool LoadCoreAssembly(const std::string& assemblyPath);
		static void LoadAppAssembly(const std::string& assemblyPath);
		static bool ReloadAssemblies(const std::string& coreAssemblyPath, const std::string& appAssemblyPath);
		static bool ConfigureAssemblies(const std::string& coreAssemblyPath, const std::string& appAssemblyPath);

		/// 是否已成功初始化并有有效脚本域
		static bool IsInitialized();
		static bool HasAppAssembly();

		// 场景更新、销毁、实体销毁、播放模式改变
		static void OnSceneUpdate(Scene* scene, float deltaTime);
		static void OnSceneFixedUpdate(Scene* scene, float fixedDeltaTime);
		static void OnSceneDestroyed(Scene* scene);
		static void OnEntityDestroyed(Scene* scene, Entity entity);
		static void OnPlayModeChanged(bool isPlayMode);

		static IScriptRuntime* GetRuntimeBridge();

		static bool InitializeScriptComponentFields(ScriptComponent& scriptComponent);
		static bool GetScriptFieldInfos(const std::string& fullClassName, std::vector<ScriptFieldInfo>& outFields);

		/// 热重载：由外部触发请求，实际重载在安全点执行
		static void RequestAssemblyReload();

		/// 提供给脚本胶水层与内部实现查询当前运行时对象
		static MonoDomain* GetDomain();
		static MonoImage* GetAppImage();
		static Scene* GetInvocationScene();
		static Entity GetInvocationEntity();

	private:
		static void RegisterInternalCallsIfNeeded();

		// 执行环境：域、程序集、镜像
		static MonoDomain* s_Domain;
		static MonoAssembly* s_CoreAssembly;
		static MonoImage* s_CoreImage;
		static MonoAssembly* s_AppAssembly;
		static MonoImage* s_AppImage;
	};

} // namespace Ehu

