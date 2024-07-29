#include "engpch.h"

#include "animation/skinned_data.h"

using namespace DirectX;

namespace Sapfire::anim {
	f32 BoneAnimation::start_time() const {
		// keyframes are sorted by time, so first keyframe gives start time.
		return keyframes.front().time_pos;
	}

	f32 BoneAnimation::end_time() const { return keyframes.back().time_pos; }

	void BoneAnimation::interp(f32 t, DirectX::XMFLOAT4X4& M) const {
		if (t <= keyframes.front().time_pos) {
			XMVECTOR S = XMLoadFloat3(&keyframes.front().scale);
			XMVECTOR P = XMLoadFloat3(&keyframes.front().position);
			XMVECTOR Q = XMLoadFloat4(&keyframes.front().rotation);
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
		} else if (t >= keyframes.back().time_pos) {
			XMVECTOR S = XMLoadFloat3(&keyframes.back().scale);
			XMVECTOR P = XMLoadFloat3(&keyframes.back().position);
			XMVECTOR Q = XMLoadFloat4(&keyframes.back().rotation);
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
		} else {
			for (UINT i = 0; i < keyframes.size() - 1; ++i) {
				if (t >= keyframes[i].time_pos && t <= keyframes[i + 1].time_pos) {
					float lerpPercent = (t - keyframes[i].time_pos) / (keyframes[i + 1].time_pos - keyframes[i].time_pos);
					XMVECTOR s0 = XMLoadFloat3(&keyframes[i].scale);
					XMVECTOR s1 = XMLoadFloat3(&keyframes[i + 1].scale);
					XMVECTOR p0 = XMLoadFloat3(&keyframes[i].position);
					XMVECTOR p1 = XMLoadFloat3(&keyframes[i + 1].position);
					XMVECTOR q0 = XMLoadFloat4(&keyframes[i].rotation);
					XMVECTOR q1 = XMLoadFloat4(&keyframes[i + 1].rotation);
					XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
					XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
					XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);
					XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
					XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
					break;
				}
			}
		}
	}

	f32 AnimationClip::start_time() const {
		// Find smallest start time over all bones in this clip.
		f32 t = FLT_MAX;
		for (UINT i = 0; i < bone_animations.size(); ++i) {
			t = t < bone_animations[i].start_time() ? t : bone_animations[i].start_time();
		}
		return t;
	}

	f32 AnimationClip::end_time() const {
		f32 t = 0.f;
		for (UINT i = 0; i < bone_animations.size(); ++i) {
			t = t > bone_animations[i].end_time() ? t : bone_animations[i].end_time();
		}
		return t;
	}

	void AnimationClip::interp(f32 t, stl::vector<DirectX::XMFLOAT4X4>& bone_transform) const {
		for (int i = 0; i < bone_animations.size(); ++i) {
			bone_animations[i].interp(t, bone_transform[i]);
		}
	}

	SkinnedData::SkinnedData(stl::vector<int>& bone_hierarchy, stl::vector<DirectX::XMFLOAT4X4>& bone_offsets,
							 stl::unordered_map<UUID, AnimationClip>& animations) :
		m_BoneHierarchy(bone_hierarchy),
		m_BoneOffsets(bone_offsets), m_Animations(animations) {}

	void SkinnedData::final_transform(UUID clip_uuid, f32 time_pos, stl::vector<DirectX::XMFLOAT4X4>& final_transforms) {
		
		UINT numBones = m_BoneOffsets.size();
		std::vector<XMFLOAT4X4> toParentTransforms(numBones);
		// Interpolate all the bones of this clip at the given time instance.
		auto clip = m_Animations.find(clip_uuid);
		clip->second.interp(time_pos, toParentTransforms);
		//
		// Traverse the hierarchy and transform all the bones to the root space.
		//
		std::vector<XMFLOAT4X4> toRootTransforms(numBones);
		// The root bone has index 0.  The root bone has no parent, so its toRootTransform
		// is just its local bone transform.
		toRootTransforms[0] = toParentTransforms[0];
		// Now find the toRootTransform of the children.
		for (UINT i = 1; i < numBones; ++i) {
			XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
			int parentIndex = m_BoneHierarchy[i];
			XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
			XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
			XMStoreFloat4x4(&toRootTransforms[i], toRoot);
		}
		// Premultiply by the bone offset transform to get the final transform.
		for (UINT i = 0; i < numBones; ++i) {
			XMMATRIX offset = XMLoadFloat4x4(&m_BoneOffsets[i]);
			XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
			XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
			XMStoreFloat4x4(&final_transforms[i], XMMatrixTranspose(finalTransform));
		}
	}

} // namespace Sapfire::anim