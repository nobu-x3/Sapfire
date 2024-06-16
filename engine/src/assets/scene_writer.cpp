#include "engpch.h"

#include <DirectXMath.h>
#include "assets/asset_manager.h"
#include "assets/scene_writer.h"
#include "components/ec_manager.h"
#include "components/movement_component.h"
#include "components/name_component.h"
#include "components/render_component.h"
#include "components/transform.h"
#include "core/memory.h"
#include "nlohmann/json.hpp"

using namespace DirectX;

namespace Sapfire::assets {

	SceneWriter::SceneWriter(ECManager* ec, AssetManager* am) : m_ECManager(*ec), m_AssetManager(*am) {}

	void SceneWriter::serialize(const stl::string& scene_path) {
		auto full_path = fs::FileSystem::get_full_path(fs::FileSystem::root_directory() + scene_path);
		auto relative_path = fs::FileSystem::root_directory() + scene_path;
		nlohmann::json j;
		for (auto&& entry : m_ECManager.entities()) {
			if (!entry.has_value() || !m_ECManager.is_valid(entry->value))
				continue;
			Entity entity = entry->value;
			auto uuid = entry->value.uuid();
			assert(m_ECManager.has_engine_component<components::Transform>(entity));
			assert(m_ECManager.has_engine_component<components::NameComponent>(entity));
			nlohmann::json j_eng_comps{};
			auto& transform = m_ECManager.engine_component<components::Transform>(entity);
			j_eng_comps["transform"] = {
				{"position",
				 stl::array<f32, 3>{XMVectorGetX(transform.position()), XMVectorGetY(transform.position()),
									XMVectorGetZ(transform.position())}},
				{"rotation",
				 stl::array<f32, 3>{XMVectorGetX(transform.euler_rotation()), XMVectorGetY(transform.euler_rotation()),
									XMVectorGetZ(transform.euler_rotation())}},
				{"scale",
				 stl::array<f32, 3>{XMVectorGetX(transform.scale()), XMVectorGetY(transform.scale()), XMVectorGetZ(transform.scale())}},
			};
			j_eng_comps["name"] = m_ECManager.engine_component<components::NameComponent>(entity).name();
			if (m_ECManager.has_engine_component<components::MovementComponent>(entity)) {
				auto& movement_comp = m_ECManager.engine_component<components::MovementComponent>(entity);
				j_eng_comps["movement"] = {
					{"acceleration",
					 stl::array<f32, 3>{XMVectorGetX(movement_comp.acceleration()), XMVectorGetY(movement_comp.acceleration()),
										XMVectorGetZ(movement_comp.acceleration())}},
					{"velocity",
					 stl::array<f32, 3>{XMVectorGetX(movement_comp.velocity()), XMVectorGetY(movement_comp.velocity()),
										XMVectorGetZ(movement_comp.velocity())}},
				};
			}
			if (m_ECManager.has_engine_component<components::RenderComponent>(entity)) {
				auto& render_component = m_ECManager.engine_component<components::RenderComponent>(entity);
				j_eng_comps["render"] = {
					{"mesh", static_cast<u64>(render_component.mesh_uuid())},
					{"texture", static_cast<u64>(render_component.texture_uuid())},
					{"material", static_cast<u64>(render_component.material_uuid())},
				};
			}
			nlohmann::json entity_json{{"uuid", static_cast<u64>(uuid)}, {"components", j_eng_comps}};
			j["entities"].push_back(entity_json);
		}
		std::ofstream file{relative_path};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void SceneWriter::deserealize(const stl::string& scene_path) {
		auto full_path = fs::FileSystem::get_full_path(fs::FileSystem::root_directory() + scene_path);
		auto relative_path = fs::FileSystem::root_directory() + scene_path;
		/* std::ifstream file{relative_path}; */
		/* if (!file.is_open()) { */
		/* 	CORE_CRITICAL("Scene at path {} could not be open.", full_path); */
		/* 	file.open(relative_path); */
		/* } */
		/* nlohmann::json j; */
		/* file >> j; */
		/* file.close(); */
		/* for(auto&& entity : j["entities"]) { */
		/* 	if (!entity.is_object()) { */
		/* 		CORE_CRITICAL("Broken scene at path {}. Array of entities is not objects.", full_path); */
		/* 		break; */
		/* 	} */
		/* } */
	}
} // namespace Sapfire::assets
