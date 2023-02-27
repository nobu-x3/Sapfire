#pragma once
#include "Sapfire/renderer/Camera.h"

namespace Sapfire
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		SceneCamera();
		virtual ~SceneCamera();
		void set_perspective(float fov, float nearClip = 0.01f, float farClip = 10000.f);
		void set_viewport_size(uint32_t width, uint32_t height);
		void set_perspective_fov_deg(float deg) { mPerspectiveFOV = deg; }
		[[nodiscard]] float get_perspective_fov_deg() const { return mPerspectiveFOV; }
		void set_perspective_fov_rad(float rad) { mPerspectiveFOV = glm::degrees(rad); }
		[[nodiscard]] float get_perspective_fov_rad() const { return glm::radians(mPerspectiveFOV); }
		void set_perspective_near_clip(float nearClip) { mPerspectiveNear = nearClip; }
		[[nodiscard]] float get_perspective_near_clip() const { return mPerspectiveNear; }
		void set_perspective_far_clip(float farClip) { mPerspectiveFar = farClip; }
		[[nodiscard]] float get_perspective_far_clip() const { return mPerspectiveFar; }
		void set_projection_type(ProjectionType type) { mProjectionType = type; }
		[[nodiscard]] ProjectionType get_projection_type() const { return mProjectionType;}

	private:
		ProjectionType mProjectionType = ProjectionType::Perspective;
		float mPerspectiveFOV = 45.f;
		float mPerspectiveNear = 0.01f;
		float mPerspectiveFar = 10000.f;
	};
}
