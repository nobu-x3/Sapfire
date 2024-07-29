#pragma once
#include "skinned_data.h"

namespace Sapfire {
	class ECManager;
	namespace d3d {
		class GraphicsDevice;
	}
} // namespace Sapfire

namespace Sapfire::anim {

	struct SFAPI AnimationResource {
		u32 cpu_idx{0};
		u32 gpu_idx{0};
	};

	using FinalTransformCache =
		stl::unordered_map<UUID, stl::unordered_map<UUID, stl::unordered_map<f32, stl::vector<DirectX::XMFLOAT4X4>>>>;

	class SFAPI AnimationManager {
	public:
		explicit AnimationManager(Sapfire::ECManager* ec, Sapfire::d3d::GraphicsDevice* device);
		void update(f32 delta_time);
		const stl::vector<DirectX::XMFLOAT4X4>& final_transforms_for_current_clip(Entity entity) const;

	private:
		// UUID of Skinned Data is the key
		stl::unordered_map<UUID, SkinnedData> m_UuidSkinnedDataMap{};
		stl::unordered_map<UUID, AnimationResource> m_SkinnedDataResourceMap{};
		// Entity UUID is the key
		stl::unordered_map<UUID, stl::vector<DirectX::XMFLOAT4X4>> m_EntityTransformsMap{};
		FinalTransformCache m_FinalTransformsCache{};
		Sapfire::ECManager& m_ECManager;
		Sapfire::d3d::GraphicsDevice& m_Device;
	};
} // namespace Sapfire::anim