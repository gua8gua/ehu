#include "ehupch.h"
#include "Project.h"
#include "Core/FileSystem.h"
#include "ProjectSerializer.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "ECS/Components.h"
#include "ECS/LayerRegistry.h"
#include "Renderer/Camera/Camera.h"  // Scene 内部持有 Camera unique_ptr，需完整类型
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <unordered_set>

namespace Ehu {

	Ref<Project> Project::s_ActiveProject;

	static std::string NormalizeDirectory(const std::string& path) {
		if (path.empty()) return path;
		std::string result = path;
		if (result.back() == '/' || result.back() == '\\')
			result.pop_back();
		return result;
	}

	static bool EnsureDirectoryExists(const std::string& path) {
		if (FileSystem::Exists(path))
			return FileSystem::IsDirectory(path);
		return FileSystem::CreateDirectory(path);
	}

	static void SyncLayerConfigFromRegistry(ProjectConfig& cfg) {
		cfg.RenderChannels.clear();
		for (const auto& [id, name] : LayerRegistry::GetRenderChannels())
			cfg.RenderChannels.push_back({ name, id });

		cfg.CollisionLayers.clear();
		for (const auto& [bit, name] : LayerRegistry::GetCollisionLayers())
			cfg.CollisionLayers.push_back({ name, bit });

		cfg.CollisionDefaultMasks.clear();
		for (const auto& [bit, mask] : LayerRegistry::GetCollisionDefaultMasks())
			cfg.CollisionDefaultMasks[bit] = mask;
	}

	Ref<Project> Project::CreateInternal(const std::string& projectFilePath, const ProjectConfig& config) {
		Ref<Project> proj = CreateRef<Project>();
		proj->m_ProjectFilePath = projectFilePath;
		proj->m_Config = config;
		s_ActiveProject = proj;
		return proj;
	}

	Ref<Project> Project::New(const std::string& directory, const std::string& name) {
		std::string dir = NormalizeDirectory(directory);
		if (dir.empty())
			dir = FileSystem::GetCurrentDirectory();

		ProjectConfig cfg;
		cfg.Name = name;
		cfg.ProjectDirectory = dir;
		cfg.AssetDirectory = "Assets";
		cfg.CacheDirectory = "Cache";
		cfg.AssetRegistryPath = "AssetRegistry.ehuassets";
		cfg.ScriptModuleDirectory = "Scripts";
		cfg.ScriptCoreAssemblyPath = "Scripts/Ehu-ScriptCore.dll";
		cfg.ScriptAppAssemblyPath = "Scripts/Game.dll";
		SyncLayerConfigFromRegistry(cfg);

		std::string projectFile = FileSystem::Join(dir, name + ".ehuproject");
		ProjectSerializer serializer;
		if (!serializer.Serialize(cfg, projectFile))
			return nullptr;
		Ref<Project> proj = CreateInternal(projectFile, cfg);

		// 默认模板：创建 Assets、Assets/Scenes 及默认场景 Main.ehuscene
		std::string assetDir = proj->GetAssetDirectory();
		if (!EnsureDirectoryExists(assetDir)) {
			Project::CloseActive();
			return nullptr;
		}
		if (!EnsureDirectoryExists(proj->GetCacheDirectory())) {
			Project::CloseActive();
			return nullptr;
		}
		if (!EnsureDirectoryExists(proj->ResolvePath(cfg.ScriptModuleDirectory.empty() ? "Scripts" : cfg.ScriptModuleDirectory))) {
			Project::CloseActive();
			return nullptr;
		}
		std::string scenesDir = FileSystem::Join(assetDir, "Scenes");
		if (!EnsureDirectoryExists(scenesDir)) {
			Project::CloseActive();
			return nullptr;
		}

		Scene defaultScene;
		Entity root = defaultScene.CreateEntity();
		defaultScene.GetWorld().AddComponent(root, TransformComponent{});
		if (TagComponent* tag = defaultScene.GetWorld().GetComponent<TagComponent>(root))
			tag->Name = "Root";

		SceneSerializer sceneSer;
		std::string mainScenePath = proj->GetAssetFileSystemPath("Scenes/Main.ehuscene");
		if (!sceneSer.Serialize(&defaultScene, mainScenePath)) {
			Project::CloseActive();
			return nullptr;
		}
		proj->m_Config.Scenes.push_back({ "Scenes/Main.ehuscene", true });
		proj->m_Config.StartupScene = "Scenes/Main.ehuscene";
		if (!SaveActive()) {
			Project::CloseActive();
			return nullptr;
		}

		return proj;
	}

	Ref<Project> Project::Load(const std::string& projectFilePath) {
		ProjectConfig cfg;
		ProjectSerializer serializer;
		if (!serializer.Deserialize(cfg, projectFilePath))
			return nullptr;
		// 若配置中 ProjectDirectory 为空，则用项目文件父目录填充
		if (cfg.ProjectDirectory.empty())
			cfg.ProjectDirectory = FileSystem::GetParentPath(projectFilePath);
		return CreateInternal(projectFilePath, cfg);
	}

	bool Project::SaveActive() {
		if (!s_ActiveProject)
			return false;
		SyncLayerConfigFromRegistry(s_ActiveProject->m_Config);
		ProjectSerializer serializer;
		return serializer.Serialize(s_ActiveProject->m_Config, s_ActiveProject->m_ProjectFilePath);
	}

	void Project::CloseActive() {
		s_ActiveProject.reset();
	}

	namespace {
		static const char* RECENT_FILE = "ehu_recent_projects.txt";
		static const size_t RECENT_MAX = 10;

		std::string NormalizeRecentProjectPath(const std::string& path) {
			if (path.empty())
				return {};
			std::error_code ec;
			std::filesystem::path p(path);
			p = std::filesystem::absolute(p, ec);
			if (ec)
				return path;
			const std::string normalized = p.lexically_normal().string();
#ifdef EHU_PLATFORM_WINDOWS
			std::string lowered = normalized;
			for (char& c : lowered) {
				if (c == '/')
					c = '\\';
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}
			return lowered;
#else
			return normalized;
#endif
		}

		std::string GetRecentFilePath() {
			return FileSystem::Join(FileSystem::GetCurrentDirectory(), RECENT_FILE);
		}

		void LoadRecent(std::vector<std::string>& out) {
			out.clear();
			std::string path = GetRecentFilePath();
			std::string content = FileSystem::ReadTextFile(path);
			if (content.empty()) return;
			std::istringstream in(content);
			std::string line;
			while (std::getline(in, line)) {
				if (!line.empty())
					out.push_back(line);
			}
		}

		void SaveRecent(const std::vector<std::string>& list) {
			std::ostringstream out;
			for (const auto& p : list)
				out << p << '\n';
			FileSystem::WriteTextFile(GetRecentFilePath(), out.str());
		}
	} // namespace

	bool Project::IsValidProjectFilePath(const std::string& projectFilePath) {
		if (projectFilePath.empty() || !FileSystem::IsFile(projectFilePath))
			return false;
		ProjectConfig cfg;
		ProjectSerializer serializer;
		return serializer.Deserialize(cfg, projectFilePath);
	}

	void Project::RemoveFromRecent(const std::string& projectFilePath) {
		const std::string key = NormalizeRecentProjectPath(projectFilePath);
		if (key.empty())
			return;
		std::vector<std::string> list;
		LoadRecent(list);
		list.erase(std::remove_if(list.begin(), list.end(), [&](const std::string& s) {
			return NormalizeRecentProjectPath(s) == key;
		}), list.end());
		SaveRecent(list);
	}

	void Project::AddToRecent(const std::string& projectFilePath) {
		const std::string normalizedPath = NormalizeRecentProjectPath(projectFilePath);
		if (normalizedPath.empty() || !FileSystem::IsFile(normalizedPath))
			return;

		std::vector<std::string> list;
		LoadRecent(list);
		list.erase(std::remove_if(list.begin(), list.end(), [&](const std::string& s) {
			return NormalizeRecentProjectPath(s) == normalizedPath;
			}), list.end());
		list.insert(list.begin(), normalizedPath);
		if (list.size() > RECENT_MAX)
			list.resize(RECENT_MAX);
		SaveRecent(list);
	}

	std::vector<std::string> Project::GetRecentProjects() {
		std::vector<std::string> raw;
		LoadRecent(raw);
		std::unordered_set<std::string> seenKeys;
		std::vector<std::string> deduped;
		deduped.reserve(raw.size());
		for (const std::string& line : raw) {
			const std::string n = NormalizeRecentProjectPath(line);
			if (n.empty())
				continue;
			if (seenKeys.count(n))
				continue;
			seenKeys.insert(n);
			deduped.push_back(n);
		}
		std::vector<std::string> valid;
		valid.reserve(deduped.size());
		for (const std::string& p : deduped) {
			if (IsValidProjectFilePath(p))
				valid.push_back(p);
		}
		const bool dirty = (deduped.size() != raw.size()) || (valid.size() != deduped.size());
		if (dirty)
			SaveRecent(valid);
		return valid;
	}

	std::string Project::GetProjectDirectory() const {
		if (!m_Config.ProjectDirectory.empty())
			return NormalizeDirectory(m_Config.ProjectDirectory);
		return NormalizeDirectory(FileSystem::GetParentPath(m_ProjectFilePath));
	}

	std::string Project::GetAssetDirectory() const {
		return ResolvePath(m_Config.AssetDirectory.empty() ? "Assets" : m_Config.AssetDirectory);
	}

	std::string Project::GetCacheDirectory() const {
		return ResolvePath(m_Config.CacheDirectory.empty() ? "Cache" : m_Config.CacheDirectory);
	}

	std::string Project::GetAssetRegistryFilePath() const {
		return ResolvePath(m_Config.AssetRegistryPath.empty() ? "AssetRegistry.ehuassets" : m_Config.AssetRegistryPath);
	}

	std::string Project::ResolvePath(const std::string& projectRelativePath) const {
		if (projectRelativePath.empty())
			return GetProjectDirectory();
		std::filesystem::path path(projectRelativePath);
		if (path.is_absolute())
			return path.lexically_normal().string();
		return FileSystem::Join(GetProjectDirectory(), projectRelativePath);
	}

	std::string Project::GetAssetFileSystemPath(const std::string& assetRelativePath) const {
		return FileSystem::Join(GetAssetDirectory(), assetRelativePath);
	}

} // namespace Ehu

