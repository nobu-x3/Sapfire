#pragma once

#include "Sapfire.h"

namespace assets {
	class ProjectReader {
	public:
		explicit ProjectReader(Sapfire::assets::AssetManager* asset_manager, Sapfire::stl::string& project_name);
		void serialize(const Sapfire::stl::string& project_path);
		void deserealize(const Sapfire::stl::string& project_path);

	private:
		Sapfire::assets::AssetManager& m_AssetManager;
		Sapfire::stl::string& m_ProjectName;
	};
} // namespace assets