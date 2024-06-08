#include "engpch.h"

#include "core/file_system.h"
#include "core/logger.h"

namespace Sapfire::fs {
	void FileSystem::locate_root_directory() {
		auto currentDirectory = std::filesystem::current_path();
		// The asset directory is one folder within the root directory.
		while (!std::filesystem::exists(currentDirectory / "Assets")) {
			if (currentDirectory.has_parent_path()) {
				currentDirectory = currentDirectory.parent_path();
			} else {
				CORE_CRITICAL("Assets Directory not found!");
			}
		}
		auto assetsDirectory = currentDirectory / "Assets";
		if (!std::filesystem::is_directory(assetsDirectory)) {
			CORE_CRITICAL("Assets Directory that was located is not a directory!");
		}
		s_RootDirectoryPath = currentDirectory.string() + "/";
		CORE_INFO("Detected root directory at path : {}.", s_RootDirectoryPath);
	}
} // namespace Sapfire::fs
