#pragma once

#include "Core/Core.h"
#include <string>
#include <vector>

namespace Ehu {

	class Project;

	/// 资产类型（按扩展名映射，供前端展示与筛选）
	enum class AssetType : uint8_t {
		Unknown = 0,
		Texture,
		Scene,
		Script,
		Folder,
	};

	/// 资产条目：相对路径 + 类型
	struct EHU_API AssetEntry {
		std::string RelativePath;
		AssetType Type = AssetType::Unknown;
		bool IsDirectory = false;
	};

	/// 资产发现：扫描项目资产目录，得到相对路径与类型列表，供资源预览面板只读使用
	class EHU_API AssetRegistry {
	public:
		/// 刷新：遍历 Project::GetAssetDirectory()，填充 m_Entries；相对路径相对于资产根
		void Refresh(const Project& project);

		/// 当前缓存的条目（Refresh 后有效）
		const std::vector<AssetEntry>& GetEntries() const { return m_Entries; }

		/// 根据扩展名推断资产类型
		static AssetType GetTypeFromExtension(const std::string& extension);

	private:
		void CollectRecursive(const std::string& assetRootAbs, const std::string& relativePrefix);

		std::vector<AssetEntry> m_Entries;
	};

} // namespace Ehu
