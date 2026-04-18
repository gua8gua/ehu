#pragma once

#include "Core/Core.h"
#include <string>

namespace Ehu {

	/// 原生系统对话框：选择文件夹 / 选择文件（由各平台实现，非 Windows 返回 false）
	namespace FileDialog {

		/// 打开“选择文件夹”对话框，选中路径写入 outPath（UTF-8），成功返回 true
		EHU_API bool OpenFolder(const std::string& title, std::string& outPath);
		/// 打开“打开文件”对话框；filterName 为显示名（如 "Ehu Project"），filterSpec 为通配符（如 "*.ehuproject"）；选中路径写入 outPath，成功返回 true
		EHU_API bool OpenFile(const std::string& title, const std::string& filterName, const std::string& filterSpec, std::string& outPath);
		/// 打开“保存文件”对话框；若用户输入的文件名没有匹配扩展名，由调用方自行补全
		EHU_API bool SaveFile(const std::string& title, const std::string& filterName, const std::string& filterSpec, std::string& outPath);
	}

} // namespace Ehu
