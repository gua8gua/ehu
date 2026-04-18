#include "ehupch.h"
#include "ProjectSerializer.h"
#include "Core/FileSystem.h"
#include <sstream>
#include <iomanip>

namespace Ehu {

	namespace {
		static const char* PROJECT_MAGIC = "EhuProject";
		static const int PROJECT_VERSION = 3;
	}

	bool ProjectSerializer::Serialize(const ProjectConfig& config, const std::string& path) {
		std::ostringstream out;
		out << PROJECT_MAGIC << ' ' << PROJECT_VERSION << '\n';
		out << "Name " << config.Name << '\n';
		if (!config.ProjectDirectory.empty())
			out << "ProjectDirectory " << config.ProjectDirectory << '\n';
		out << "AssetDirectory " << (config.AssetDirectory.empty() ? "Assets" : config.AssetDirectory) << '\n';
		out << "CacheDirectory " << (config.CacheDirectory.empty() ? "Cache" : config.CacheDirectory) << '\n';
		out << "AssetRegistry " << (config.AssetRegistryPath.empty() ? "AssetRegistry.ehuassets" : config.AssetRegistryPath) << '\n';
		out << "ScriptModuleDirectory " << (config.ScriptModuleDirectory.empty() ? "Scripts" : config.ScriptModuleDirectory) << '\n';
		if (!config.ScriptCoreAssemblyPath.empty())
			out << "ScriptCoreAssembly " << config.ScriptCoreAssemblyPath << '\n';
		if (!config.ScriptAppAssemblyPath.empty())
			out << "ScriptAppAssembly " << config.ScriptAppAssemblyPath << '\n';
		if (!config.StartupScene.empty())
			out << "StartupScene " << config.StartupScene << '\n';

		for (const ProjectSceneEntry& entry : config.Scenes)
			out << "Scene " << entry.RelativePath << ' ' << (entry.Active ? 1 : 0) << '\n';

		for (const RenderChannelConfigEntry& entry : config.RenderChannels)
			out << "RenderChannel " << std::quoted(entry.Name) << ' ' << entry.Id << '\n';

		for (const CollisionLayerConfigEntry& entry : config.CollisionLayers)
			out << "CollisionLayer " << std::quoted(entry.Name) << ' ' << entry.Bit << '\n';

		for (const auto& [layerBit, mask] : config.CollisionDefaultMasks)
			out << "CollisionDefaultMask " << layerBit << ' ' << mask << '\n';

		return FileSystem::WriteTextFile(path, out.str());
	}

	bool ProjectSerializer::Deserialize(ProjectConfig& config, const std::string& path) {
		std::string content = FileSystem::ReadTextFile(path);
		if (content.empty())
			return false;
		std::istringstream in(content);
		std::string line;
		if (!std::getline(in, line) || line.find(PROJECT_MAGIC) != 0)
			return false;

		config = ProjectConfig{};

		while (std::getline(in, line)) {
			if (line.rfind("Name ", 0) == 0) {
				config.Name = line.substr(5);
			} else if (line.rfind("ProjectDirectory ", 0) == 0) {
				config.ProjectDirectory = line.substr(17);
			} else if (line.rfind("AssetDirectory ", 0) == 0) {
				config.AssetDirectory = line.substr(15);
			} else if (line.rfind("CacheDirectory ", 0) == 0) {
				config.CacheDirectory = line.substr(15);
			} else if (line.rfind("AssetRegistry ", 0) == 0) {
				config.AssetRegistryPath = line.substr(14);
			} else if (line.rfind("ScriptModuleDirectory ", 0) == 0) {
				config.ScriptModuleDirectory = line.substr(22);
			} else if (line.rfind("ScriptCoreAssembly ", 0) == 0) {
				config.ScriptCoreAssemblyPath = line.substr(19);
			} else if (line.rfind("ScriptAppAssembly ", 0) == 0) {
				config.ScriptAppAssemblyPath = line.substr(18);
			} else if (line.rfind("StartupScene ", 0) == 0) {
				config.StartupScene = line.substr(13);
			} else if (line.rfind("Scene ", 0) == 0) {
				std::string rest = line.substr(6);
				size_t lastSpace = rest.rfind(' ');
				bool active = true;
				std::string path = rest;
				if (lastSpace != std::string::npos) {
					std::string tail = rest.substr(lastSpace + 1);
					if (tail == "0" || tail == "1") {
						active = (tail == "1");
						path = rest.substr(0, lastSpace);
					}
				}
				config.Scenes.push_back({ path, active });
			} else if (line.rfind("ActiveScene ", 0) == 0) {
				// 兼容旧格式：当作启用场景追加
				config.Scenes.push_back({ line.substr(12), true });
			} else if (line.rfind("RenderChannel ", 0) == 0) {
				std::istringstream iss(line.substr(14));
				RenderChannelConfigEntry entry;
				if (iss >> std::quoted(entry.Name) >> entry.Id)
					config.RenderChannels.push_back(std::move(entry));
			} else if (line.rfind("CollisionLayer ", 0) == 0) {
				std::istringstream iss(line.substr(15));
				CollisionLayerConfigEntry entry;
				if (iss >> std::quoted(entry.Name) >> entry.Bit)
					config.CollisionLayers.push_back(std::move(entry));
			} else if (line.rfind("CollisionDefaultMask ", 0) == 0) {
				std::istringstream iss(line.substr(21));
				uint32_t layerBit = 0;
				uint32_t mask = 0;
				if (iss >> layerBit >> mask)
					config.CollisionDefaultMasks[layerBit] = mask;
			}
		}

		if (config.AssetDirectory.empty())
			config.AssetDirectory = "Assets";
		if (config.CacheDirectory.empty())
			config.CacheDirectory = "Cache";
		if (config.AssetRegistryPath.empty())
			config.AssetRegistryPath = "AssetRegistry.ehuassets";
		if (config.ScriptModuleDirectory.empty())
			config.ScriptModuleDirectory = "Scripts";
		return true;
	}

} // namespace Ehu

