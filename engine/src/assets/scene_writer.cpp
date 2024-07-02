#include "engpch.h"

#include <DirectXMath.h>
#include "../../include/core/game_context.h"
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
		auto full_path = fs::full_path(scene_path);
		auto relative_path = fs::relative_path(scene_path);
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
				{"parent", transform.parent()},
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
			nlohmann::json entity_json{{"UUID", static_cast<u64>(uuid)}, {"components", j_eng_comps}};
			j["entities"].push_back(entity_json);
		}
		j["assets"] = nlohmann::json::parse(m_AssetManager.to_string());
		std::ofstream file{relative_path};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void SceneWriter::deserealize(
		const stl::string& scene_path,
		stl::function<void(Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths)> render_component_setter) {
		auto full_path = fs::full_path(scene_path);
		auto relative_path = fs::relative_path(scene_path);
		std::ifstream file{relative_path};
		if (!file.is_open()) {
			CORE_CRITICAL("Scene at path {} could not be open.", full_path);
			file.open(relative_path);
		}
		nlohmann::json j;
		file >> j;
		file.close();
		if (!j.contains("assets")) {
			CORE_CRITICAL("Broken scene at path {}. Missing asset registries.", full_path);
			return;
		}
		auto& registries = j["assets"];
		if (!registries.contains("mesh_registry")) {
			CORE_CRITICAL("Broken scene at path {}. Missing mesh registry.", full_path);
			return;
		}
		m_AssetManager.deserialize(j.dump());
		for (auto&& entity : j["entities"]) {
			if (!entity.is_object()) {
				CORE_CRITICAL("Broken scene at path {}. Serialization formatting error. No entity was created.", full_path);
				break;
			}
			if (!entity.contains("UUID")) {
				CORE_CRITICAL("Broken scene at path {}. An entity does not contain a UUID. No entity was created. Dump:\n{}", full_path,
							  entity.dump());
				break;
			}
			UUID uuid{entity["UUID"]};
			if (!entity.contains("components")) {
				CORE_CRITICAL("Broken scene at path {}. Entity with UUID {} does not contain any components. No entity was created.",
							  full_path, static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"].contains("transform")) {
				CORE_CRITICAL("Broken scene at path {}. Entity with UUID {} does not contain any transform. No entity was created.",
							  full_path, static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"].contains("name")) {
				CORE_CRITICAL("Broken scene at path {}. Entity with UUID {} does not contain any name. No entity was created.", full_path,
							  static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"]["transform"].contains("position")) {
				CORE_CRITICAL(
					"Broken scene at path {}. Entity with UUID {}'s transform components does not contain position. No entity was created.",
					full_path, static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"]["transform"].contains("scale")) {
				CORE_CRITICAL(
					"Broken scene at path {}. Entity with UUID {}'s transform components does not contain scale. No entity was created.",
					full_path, static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"]["transform"].contains("rotation")) {
				CORE_CRITICAL(
					"Broken scene at path {}. Entity with UUID {}'s transform components does not contain rotation. No entity was created.",
					full_path, static_cast<u64>(uuid));
				break;
			}
			if (!entity["components"]["transform"].contains("parent")) {
				CORE_CRITICAL("Broken scene at path {}. Entity with UUID {}'s transform components does not contain parent index. No "
							  "entity was created.",
							  full_path, static_cast<u64>(uuid));
				break;
			}
			auto created_entity = m_ECManager.create_entity(uuid);
			auto& transform = m_ECManager.engine_component<components::Transform>(created_entity);
			stl::array<f32, 3> position = entity["components"]["transform"]["position"];
			stl::array<f32, 3> rotation = entity["components"]["transform"]["rotation"];
			stl::array<f32, 3> scale = entity["components"]["transform"]["scale"];
			transform.position(DirectX::XMVECTOR{position[0], position[1], position[2]});
			transform.scale(DirectX::XMVECTOR{scale[0], scale[1], scale[2]});
			transform.euler_rotation(DirectX::XMVECTOR{rotation[0], rotation[1], rotation[2]});
			transform.parent(entity["components"]["transform"]["parent"]);
			auto& name = m_ECManager.engine_component<components::NameComponent>(created_entity);
			name.name(entity["components"]["name"]);
			if (entity["components"].contains("movement")) {
				auto movement_json = entity["components"]["movement"];
				bool should_add = true;
				if (!movement_json.contains("velocity")) {
					CORE_ERROR("Broken scene at path {}. Entity with UUID {}'s movement component does not contain velocity.", full_path,
							   static_cast<u64>(uuid));
					should_add = false;
				}
				if (!movement_json.contains("acceleration")) {
					CORE_ERROR("Broken scene at path {}. Entity with UUID {}'s movement component does not contain acceleration.",
							   full_path, static_cast<u64>(uuid));
					should_add = false;
				}
				if (should_add) {
					stl::array<f32, 3> velocity = movement_json["velocity"];
					stl::array<f32, 3> acceleration = movement_json["acceleration"];
					components::MovementComponent movement_comp{};
					movement_comp.acceleration(DirectX::XMVECTOR{acceleration[0], acceleration[1], acceleration[2]});
					movement_comp.velocity(DirectX::XMVECTOR{velocity[0], velocity[1], velocity[2]});
					m_ECManager.add_engine_component<components::MovementComponent>(created_entity, movement_comp);
				}
			}
			if (entity["components"].contains("render")) {
				UUID mesh_uuid{entity["components"]["render"]["mesh"]};
				UUID texture_uuid{entity["components"]["render"]["texture"]};
				stl::string mesh_path = m_AssetManager.get_mesh_path(mesh_uuid);
				stl::string texture_path = m_AssetManager.get_texture_path(texture_uuid);
				render_component_setter(created_entity, {.mesh_path = mesh_path, .texture_path = texture_path});
				//auto& render_comp = m_ECManager.engine_component<components::RenderComponent>(created_entity);
				//auto* gpu_data = render_comp.per_draw_constants();
				//auto resource = m_AssetManager.get_texture_resource(texture_path);
				//gpu_data->texture_cbuffer_idx = resource.gpu_idx;
			}
		}
	}
} // namespace Sapfire::assets
