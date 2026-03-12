#pragma once

#include "Core/Core.h"
#include "ProjectConfig.h"
#include <string>

namespace Ehu {

	/// 项目序列化：将 ProjectConfig 保存/加载为简单文本（.ehuproject）
	class EHU_API ProjectSerializer {
	public:
		ProjectSerializer() = default;

		bool Serialize(const ProjectConfig& config, const std::string& path);
		bool Deserialize(ProjectConfig& config, const std::string& path);
	};

} // namespace Ehu

