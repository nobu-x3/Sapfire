#pragma once

#include "core.h"
#include "render/d3d_util.h"

namespace Sapfire::fs {

	class FileSystem {
	public:
		static inline std::string get_full_path(const std::string_view assetPath) { return s_RootDirectoryPath + assetPath.data(); }

		static inline std::wstring get_full_path(const std::wstring_view assetPath) {
			return d3d::AnsiToWString(s_RootDirectoryPath) + assetPath.data();
		}

		static void locate_root_directory();

		static const stl::string& root_directory() { return s_RootDirectoryPath; }

	private:
		static inline stl::string s_RootDirectoryPath{};
	};
} // namespace Sapfire::fs
