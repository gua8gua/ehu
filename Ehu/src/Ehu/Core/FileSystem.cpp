#include "ehupch.h"
#include "FileSystem.h"
#include <fstream>
#include <filesystem>

namespace Ehu {

	namespace fs = std::filesystem;

	bool FileSystem::Exists(const std::string& path) {
		std::error_code ec;
		return fs::exists(path, ec);
	}

	bool FileSystem::IsDirectory(const std::string& path) {
		std::error_code ec;
		return fs::is_directory(path, ec);
	}

	bool FileSystem::IsFile(const std::string& path) {
		std::error_code ec;
		return fs::is_regular_file(path, ec);
	}

	bool FileSystem::CreateDirectory(const std::string& path) {
		std::error_code ec;
		return fs::create_directories(path, ec);
	}

	std::string FileSystem::ReadTextFile(const std::string& path) {
		std::ifstream f(path, std::ios::in | std::ios::binary);
		if (!f)
			return {};
		std::string out;
		f.seekg(0, std::ios::end);
		out.resize(static_cast<size_t>(f.tellg()));
		f.seekg(0);
		f.read(&out[0], static_cast<std::streamsize>(out.size()));
		return out;
	}

	bool FileSystem::WriteTextFile(const std::string& path, const std::string& content) {
		std::ofstream f(path, std::ios::out | std::ios::trunc);
		if (!f)
			return false;
		f.write(content.data(), static_cast<std::streamsize>(content.size()));
		return !!f;
	}

	bool FileSystem::AppendTextFile(const std::string& path, const std::string& content) {
		std::ofstream f(path, std::ios::out | std::ios::app);
		if (!f)
			return false;
		f.write(content.data(), static_cast<std::streamsize>(content.size()));
		return !!f;
	}

	std::string FileSystem::GetCurrentDirectory() {
		std::error_code ec;
		auto p = fs::current_path(ec);
		return ec ? std::string() : p.string();
	}

	std::string FileSystem::Join(const std::string& a, const std::string& b) {
		return (fs::path(a) / b).string();
	}

	std::string FileSystem::GetExtension(const std::string& path) {
		return fs::path(path).extension().string();
	}

	std::string FileSystem::GetFileName(const std::string& path) {
		return fs::path(path).filename().string();
	}

	std::string FileSystem::GetParentPath(const std::string& path) {
		return fs::path(path).parent_path().string();
	}

} // namespace Ehu
