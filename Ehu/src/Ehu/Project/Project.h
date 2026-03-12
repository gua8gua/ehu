#pragma once

#include "Core/Core.h"
#include "Core/Ref.h"
#include "ProjectConfig.h"
#include <string>
#include <vector>

namespace Ehu {

	/// 项目单例：管理当前游戏项目的配置与路径解析
	class EHU_API Project {
	public:
		Project() = default;

		/// 创建新项目：在 directory 下以 name 命名，生成默认配置与项目文件；并设为 Active
		static Ref<Project> New(const std::string& directory, const std::string& name);
		/// 从项目文件路径加载项目配置，并设为 Active
		static Ref<Project> Load(const std::string& projectFilePath);
		/// 将当前激活项目保存回其项目文件
		static bool SaveActive();
		/// 关闭当前激活项目（清空单例，不卸载已加载场景，由调用方 DeactivateAllScenes）
		static void CloseActive();

		static Ref<Project> GetActive() { return s_ActiveProject; }

		/// 最近打开项目：加入列表并持久化
		static void AddToRecent(const std::string& projectFilePath);
		/// 最近项目路径列表（最多 10 条，从持久化读取）
		static std::vector<std::string> GetRecentProjects();

		const ProjectConfig& GetConfig() const { return m_Config; }
		ProjectConfig& GetConfig() { return m_Config; }

		/// 项目文件的绝对路径（例如 D:/Game/MyGame/MyGame.ehuproject）
		const std::string& GetProjectFilePath() const { return m_ProjectFilePath; }

		/// 项目根目录绝对路径（若配置中为空，则由项目文件路径推导）
		std::string GetProjectDirectory() const;
		/// 资产目录绝对路径（ProjectDirectory + AssetDirectory）
		std::string GetAssetDirectory() const;
		/// 将资产相对路径（如 "Textures/foo.png"）转换为绝对路径
		std::string GetAssetFileSystemPath(const std::string& assetRelativePath) const;

	private:
		static Ref<Project> s_ActiveProject;

		ProjectConfig m_Config;
		std::string m_ProjectFilePath;

		static Ref<Project> CreateInternal(const std::string& projectFilePath, const ProjectConfig& config);
	};

} // namespace Ehu

