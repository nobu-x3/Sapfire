#include "assets/project_reader.h"
#include "nlohmann/json.hpp"

namespace assets {
	ProjectReader::ProjectReader(Sapfire::assets::AssetManager* asset_manager, Sapfire::stl::string& project_name) :
		m_AssetManager(*asset_manager), m_ProjectName(project_name) {}

	void ProjectReader::serialize(const Sapfire::stl::string& project_path) {
		nlohmann::json j;
		j["name"] = m_ProjectName;
		j["assets"] = nlohmann::json::parse(m_AssetManager.to_string());
		std::ofstream file{project_path};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void ProjectReader::deserealize(const Sapfire::stl::string& project_path) {
		std::ifstream file{project_path};
		if (!file.is_open()) {
			CLIENT_CRITICAL("Scene at path {} could not be open.", project_path);
			file.open(project_path);
		}
		try {
			int size = file.peek();
			if (size == std::ifstream::traits_type::eof()) {
				CLIENT_ERROR("Scene at path {} is empty. Will attemt to create a new project.", project_path);
				return;
			}
		} catch (std::exception&) {
			CLIENT_ERROR(
				"Scene at path {} is probably empty, otherwise an internal STD exception was thrown. Will attemt to create a new project.",
				project_path);
			return;
		}
		nlohmann::json j;
		file >> j;
		file.close();
		if (!j.contains("name")) {
			CLIENT_CRITICAL("Broken scene at path {}. Missing project name.", project_path);
			return;
		}
		if (!j.contains("assets")) {
			CLIENT_CRITICAL("Broken scene at path {}. Missing asset registries.", project_path);
			return;
		}
		m_ProjectName = j["name"];
		auto& registries = j["assets"];
		if (!registries.contains("mesh_registry")) {
			CLIENT_CRITICAL("Broken scene at path {}. Missing mesh registry.", project_path);
			return;
		}
		m_AssetManager.deserialize(j.dump());
	}

} // namespace assets
