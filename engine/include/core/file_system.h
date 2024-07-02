#pragma once

#include "core.h"
#include "render/d3d_util.h"

namespace Sapfire::fs {

	class FileSystem {
	public:
		static void locate_root_directory();

		static const stl::string& root_directory() { return s_RootDirectoryPath; }

	private:
		static inline stl::string s_RootDirectoryPath{};
	};

	inline stl::string extension(const std::string_view asset_path) {
		std::filesystem::path path(asset_path);
		return path.extension().string();
	}

	inline stl::string relative_path(const std::string_view asset_path) {
		std::filesystem::path path = std::filesystem::relative(asset_path);
		if (path.is_relative()) {
			return path.relative_path().string();
		}
		std::filesystem::path corrected_path =
			std::filesystem::relative(std::filesystem::path(asset_path), std::filesystem::absolute(FileSystem::root_directory()));
		return corrected_path.relative_path().string();
	}

	inline stl::string full_path(const std::string_view asset_path) {
		std::filesystem::path path = std::filesystem::absolute(asset_path);
		if (path.is_absolute()) {
			return std::filesystem::absolute(asset_path).string();
		}
		std::filesystem::path corrected_path{std::filesystem::current_path().string() + "/" + stl::string{asset_path}};
		return std::filesystem::absolute(corrected_path).string();
	}

	inline stl::wstring full_path(const std::wstring_view asset_path) {
		std::filesystem::path path = std::filesystem::absolute(asset_path);
		if (path.is_absolute()) {
			return std::filesystem::absolute(asset_path).wstring();
		}
		std::filesystem::path corrected_path{std::filesystem::current_path().wstring() + L"/" + stl::wstring{asset_path}};
		return std::filesystem::absolute(corrected_path).wstring();
	}

	inline stl::string file_name(const std::string_view asset_path) {
		std::filesystem::path path(asset_path);
		return path.filename().string();
	}

} // namespace Sapfire::fs
