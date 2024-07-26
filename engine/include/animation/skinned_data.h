#pragma once

#include <DirectXMath.h>

namespace Sapfire::anim {
	struct Keyframe {
		f32 time_pos;
		DirectX::XMFLOAT3 position{0};
		DirectX::XMFLOAT3 scale{0};
		DirectX::XMFLOAT4 rotation{0};
	};

	// At least 2 keyframes
	struct BoneAnimation {
		f32 start_time() const;
		f32 end_time() const;

		void interp(f32 t, DirectX::XMFLOAT4X4& M) const;

		stl::vector<Keyframe> keyframes;
	};

	struct AnimationClip {
		f32 start_time() const;
		f32 end_time() const;

		void interp(f32 t, stl::vector<DirectX::XMFLOAT4X4>& bone_transform) const;

		stl::vector<BoneAnimation> bone_animations;
		UUID uuid;
	};

	using FinalTransformCache = stl::unordered_map<UUID, stl::unordered_map<f32, stl::vector<DirectX::XMFLOAT4X4>>>;

	class SkinnedData {
	public:
		SkinnedData(stl::vector<int>& bone_hierarchy, stl::vector<DirectX::XMFLOAT4X4>& bone_offsets,
					stl::unordered_map<UUID, AnimationClip>& animations);
		inline u32 bone_count() const { return m_BoneHierarchy.size(); }
		inline f32 clip_start_time(UUID clip_uuid) const { m_Animations.at(clip_uuid).start_time(); }
		inline f32 clip_end_time(UUID clip_uuid) const { m_Animations.at(clip_uuid).end_time(); }
		void final_transform(UUID clip_uuid, f32 time_pos, stl::vector<DirectX::XMFLOAT4X4>& final_transforms);

	private:
		// Gives parentIndex of ith bone.
		stl::vector<int> m_BoneHierarchy;
		stl::vector<DirectX::XMFLOAT4X4> m_BoneOffsets;
		stl::unordered_map<UUID, AnimationClip> m_Animations;
		FinalTransformCache m_FinalTransformsCache{};
	};
} // namespace Sapfire::anim