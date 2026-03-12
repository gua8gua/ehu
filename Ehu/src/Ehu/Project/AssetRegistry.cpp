#include "AssetRegistry.h"
#include "Project.h"
#include "Core/FileSystem.h"
#include <filesystem>

namespace Ehu {

	AssetType AssetRegistry::GetTypeFromExtension(const std::string& extension) {
		if (extension.empty()) return AssetType::Unknown;
		// 小写比较
		std::string ext = extension;
		for (char& c : ext) { if (c >= 'A' && c <= 'Z') c += 32; }
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp")
			return AssetType::Texture;
		if (ext == ".ehuscene")
			return AssetType::Scene;
		if (ext == ".cpp" || ext == ".h" || ext == ".cs")
			return AssetType::Script;
		return AssetType::Unknown;
	}

	void AssetRegistry::Refresh(const Project& project) {
		m_Entries.clear();
		std::string root = project.GetAssetDirectory();
		if (root.empty() || !FileSystem::IsDirectory(root))
			return;
		CollectRecursive(root, "");
	}

	void AssetRegistry::CollectRecursive(const std::string& assetRootAbs, const std::string& relativePrefix) {
		namespace fs = std::filesystem;
		std::error_code ec;
		fs::path root(assetRootAbs);
		if (!fs::is_directory(root, ec))
			return;

		for (const auto& entry : fs::directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
			if (ec) continue;
			std::string name = entry.path().filename().string();
			std::string relPath = relativePrefix.empty() ? name : FileSystem::Join(relativePrefix, name);

			AssetEntry e;
			e.RelativePath = relPath;

			if (entry.is_directory(ec)) {
				e.Type = AssetType::Folder;
				e.IsDirectory = true;
				m_Entries.push_back(e);
				CollectRecursive(entry.path().string(), relPath);
			} else if (entry.is_regular_file(ec)) {
				e.IsDirectory = false;
				e.Type = GetTypeFromExtension(entry.path().extension().string());
				m_Entries.push_back(e);
			}
		}
	}

} // namespace Ehu
