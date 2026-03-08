#pragma once

#include "Core.h"
#include <string>

namespace Ehu {

	/// 文件系统路径与基础操作（跨平台抽象，内部使用 std::filesystem）
	class EHU_API FileSystem {
	public:
		/// 路径是否存在
		static bool Exists(const std::string& path);
		/// 是否为目录
		static bool IsDirectory(const std::string& path);
		/// 是否为常规文件
		static bool IsFile(const std::string& path);

		/// 创建目录（含父级）；成功返回 true
		static bool CreateDirectory(const std::string& path);
		/// 读取整个文件为文本；失败返回空字符串
		static std::string ReadTextFile(const std::string& path);
		/// 写入文本到文件；成功返回 true
		static bool WriteTextFile(const std::string& path, const std::string& content);
		/// 追加文本到文件末尾；成功返回 true
		static bool AppendTextFile(const std::string& path, const std::string& content);

		/// 当前工作目录
		static std::string GetCurrentDirectory();
		/// 路径拼接（自动处理分隔符）
		static std::string Join(const std::string& a, const std::string& b);
		/// 取扩展名（含点，如 ".txt"）
		static std::string GetExtension(const std::string& path);
		/// 取文件名（含扩展名）
		static std::string GetFileName(const std::string& path);
		/// 取父目录路径
		static std::string GetParentPath(const std::string& path);
	};

} // namespace Ehu
