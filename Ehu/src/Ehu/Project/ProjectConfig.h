#pragma once

#include "Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace Ehu {

	/// 项目内一条场景记录：相对路径 + 是否启用（启用则打开项目时加载并参与渲染）
	struct EHU_API ProjectSceneEntry {
		std::string RelativePath;  /// 如 "Scenes/Level1.ehuscene"
		bool Active = true;        /// 是否启用，与 Application 侧“已激活”对应
	};

	/// 项目配置：描述一个游戏项目的元数据与场景/资源布局
	struct EHU_API RenderChannelConfigEntry {
		std::string Name;
		uint32_t Id = 0;
	};

	struct EHU_API CollisionLayerConfigEntry {
		std::string Name;
		uint32_t Bit = 0;
	};

	struct EHU_API ProjectConfig {
		std::string Name;
		/// 项目根目录（可由项目文件路径推导，此字段主要用于序列化可读性）
		std::string ProjectDirectory;
		/// 资产目录相对路径（相对于 ProjectDirectory，如 "Assets"）
		std::string AssetDirectory = "Assets";
		/// 缓存目录相对路径（编译中间产物、导入缓存等）
		std::string CacheDirectory = "Cache";
		/// 资产注册表文件相对路径
		std::string AssetRegistryPath = "AssetRegistry.ehuassets";
		/// 脚本模块目录相对路径
		std::string ScriptModuleDirectory = "Scripts";
		/// C# ScriptCore 程序集路径（可为绝对路径，或相对于 ProjectDirectory）
		std::string ScriptCoreAssemblyPath;
		/// C# 游戏脚本程序集路径（可为绝对路径，或相对于 ProjectDirectory）
		std::string ScriptAppAssemblyPath;

		/// 场景列表：顺序即加载顺序；Active 为 true 的会在打开项目时加载并参与渲染
		std::vector<ProjectSceneEntry> Scenes;
		/// 默认起始场景文件（相对路径），可与 Scenes 中某条一致
		std::string StartupScene;

		/// 项目级渲染通道注册表（名称 <-> ID）
		std::vector<RenderChannelConfigEntry> RenderChannels;
		/// 项目级物理层注册表（名称 <-> bit）
		std::vector<CollisionLayerConfigEntry> CollisionLayers;
		/// 物理层默认掩码模板（layerBit -> defaultMask）
		std::unordered_map<uint32_t, uint32_t> CollisionDefaultMasks;
	};

} // namespace Ehu

