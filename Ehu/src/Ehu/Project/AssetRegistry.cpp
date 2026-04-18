#include "AssetRegistry.h"
#include "Project.h"
#include "Core/FileSystem.h"
#include <algorithm>
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
		if (ext == ".ttf" || ext == ".otf")
			return AssetType::Font;
		return AssetType::Unknown;
	}

	void AssetRegistry::Refresh(const Project& project) {
		m_Entries.clear();
		m_EntryIndexByHandle.clear();
		m_HandleByPath.clear();
		std::string root = project.GetAssetDirectory();
		if (root.empty() || !FileSystem::IsDirectory(root))
			return;
		CollectRecursive(root, "");
		std::sort(m_Entries.begin(), m_Entries.end(), [](const AssetEntry& a, const AssetEntry& b) {
			return a.RelativePath < b.RelativePath;
		});
		for (size_t index = 0; index < m_Entries.size(); ++index) {
			m_EntryIndexByHandle[m_Entries[index].Handle] = index;
			m_HandleByPath[m_Entries[index].RelativePath] = m_Entries[index].Handle;
		}
	}

	const AssetEntry* AssetRegistry::GetEntry(AssetHandle handle) const {
		auto it = m_EntryIndexByHandle.find(handle);
		if (it == m_EntryIndexByHandle.end())
			return nullptr;
		return &m_Entries[it->second];
	}

	const AssetEntry* AssetRegistry::GetEntry(const std::string& relativePath) const {
		auto handleIt = m_HandleByPath.find(relativePath);
		return handleIt != m_HandleByPath.end() ? GetEntry(handleIt->second) : nullptr;
	}

	bool AssetRegistry::Contains(AssetHandle handle) const {
		return m_EntryIndexByHandle.find(handle) != m_EntryIndexByHandle.end();
	}

	AssetHandle AssetRegistry::GetHandle(const std::string& relativePath) const {
		auto it = m_HandleByPath.find(relativePath);
		return it != m_HandleByPath.end() ? it->second : 0;
	}

	AssetHandle AssetRegistry::HashPath(const std::string& relativePath) {
		const uint64_t offset = 1469598103934665603ull;
		const uint64_t prime = 1099511628211ull;
		uint64_t hash = offset;
		for (char c : relativePath) {
			hash ^= static_cast<uint8_t>(c);
			hash *= prime;
		}
		return hash;
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
			e.Handle = HashPath(relPath);
			e.RelativePath = relPath;
			e.FileSystemPath = entry.path().lexically_normal().string();

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
