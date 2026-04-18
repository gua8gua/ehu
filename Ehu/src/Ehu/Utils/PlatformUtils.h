#pragma once

#include "Core/Core.h"
#include "Platform/IO/FileDialog.h"
#include <string>

namespace Ehu {

	/// 与 FileDialog 对齐的薄封装，便于与 Hazel PlatformUtils 命名习惯对齐
	namespace PlatformUtils {

		inline bool OpenFile(const std::string& title, const std::string& filterName, const std::string& filterSpec, std::string& outPath) {
			return FileDialog::OpenFile(title, filterName, filterSpec, outPath);
		}

		inline bool OpenFolder(const std::string& title, std::string& outPath) {
			return FileDialog::OpenFolder(title, outPath);
		}

		inline bool SaveFile(const std::string& title, const std::string& filterName, const std::string& filterSpec, std::string& outPath) {
			return FileDialog::SaveFile(title, filterName, filterSpec, outPath);
		}

	} // namespace PlatformUtils

} // namespace Ehu
