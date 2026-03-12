#include "ehupch.h"
#include "ProjectSerializer.h"
#include "Core/FileSystem.h"
#include <sstream>

namespace Ehu {

	namespace {
		static const char* PROJECT_MAGIC = "EhuProject";
		static const int PROJECT_VERSION = 1;
	}

	bool ProjectSerializer::Serialize(const ProjectConfig& config, const std::string& path) {
		std::ostringstream out;
		out << PROJECT_MAGIC << ' ' << PROJECT_VERSION << '\n';
		out << "Name " << config.Name << '\n';
		if (!config.ProjectDirectory.empty())
			out << "ProjectDirectory " << config.ProjectDirectory << '\n';
		out << "AssetDirectory " << (config.AssetDirectory.empty() ? "Assets" : config.AssetDirectory) << '\n';
		if (!config.StartupScene.empty())
			out << "StartupScene " << config.StartupScene << '\n';

		for (const ProjectSceneEntry& entry : config.Scenes)
			out << "Scene " << entry.RelativePath << ' ' << (entry.Active ? 1 : 0) << '\n';

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
			}
		}

		if (config.AssetDirectory.empty())
			config.AssetDirectory = "Assets";
		return true;
	}

} // namespace Ehu

