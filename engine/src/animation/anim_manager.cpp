#include "engpch.h"

#include "animation/anim_manager.h"
#include "components/anim_component.h"
#include "components/ec_manager.h"
#include "render/graphics_device.h"

namespace Sapfire::anim {
	AnimationManager::AnimationManager(Sapfire::ECManager* ec, Sapfire::d3d::GraphicsDevice* device) :
		m_ECManager(*ec), m_Device(*device) {}

	void AnimationManager::update(f32 delta_time) {
		auto& anim_components = m_ECManager.engine_components<components::AnimComponent>();
		for (auto& comp : anim_components) {
			auto skinned_data_uuid = comp.skinned_data_uuid();
			auto time_pos = comp.time_pos() + delta_time;
			// @TODO: Right now after one clip ends it loops. Eventually I want it to reset the state depending on the state machine.
			if (time_pos > m_UuidSkinnedDataMap[skinned_data_uuid].clip_end_time(comp.current_clip()))
				time_pos = 0.0f;
			comp.time_pos(time_pos);
			Entity entity = m_ECManager.engine_component_owner<components::AnimComponent>(comp);
			if (auto inner_cache_it = m_FinalTransformsCache.find(entity.uuid()); inner_cache_it != m_FinalTransformsCache.end()) {
				if (auto clip_it = inner_cache_it->second.find(comp.current_clip()); clip_it != inner_cache_it->second.end()) {
					if (auto transform_it = clip_it->second.find(time_pos); transform_it != clip_it->second.end()) {
						m_EntityTransformsMap[entity.uuid()] = m_FinalTransformsCache[entity.uuid()][comp.current_clip()][time_pos];
						return;
					} else {
						m_FinalTransformsCache[entity.uuid()][comp.current_clip()][time_pos] = {};
					}
				} else {
					m_FinalTransformsCache[entity.uuid()][comp.current_clip()] = {{}};
				}
			} else {
				stl::unordered_map<f32, stl::vector<DirectX::XMFLOAT4X4>> time_pos_transform_map{{comp.current_clip(), {}}};
				stl::unordered_map<UUID, stl::unordered_map<f32, stl::vector<DirectX::XMFLOAT4X4>>> clip_time_map{
					{time_pos, std::move(time_pos_transform_map)}};
				m_FinalTransformsCache[entity.uuid()] = std::move(clip_time_map);
			}
			m_UuidSkinnedDataMap[skinned_data_uuid].final_transform(comp.current_clip(), time_pos, m_EntityTransformsMap[entity.uuid()]);
			m_FinalTransformsCache[entity.uuid()][comp.current_clip()][time_pos] = m_EntityTransformsMap[entity.uuid()];
		}
	}

	const Sapfire::stl::vector<DirectX::XMFLOAT4X4>& AnimationManager::final_transforms_for_current_clip(Entity entity) const {
		return m_EntityTransformsMap[entity.uuid()];
	}

} // namespace Sapfire::anim
