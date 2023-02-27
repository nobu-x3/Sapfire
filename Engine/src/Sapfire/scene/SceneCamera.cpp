#include "engpch.h"
#include "SceneCamera.h"

namespace Sapfire
{

	SceneCamera::SceneCamera()
	{
	}

	SceneCamera::~SceneCamera()
	{
	}

	void SceneCamera::set_perspective(float fov, float nearClip, float farClip)
	{
		mProjectionType = ProjectionType::Perspective;
		mPerspectiveFOV = fov;
		mPerspectiveNear = nearClip;
		mPerspectiveFar = farClip;
	}

	void SceneCamera::set_viewport_size(uint32_t width, uint32_t height)
	{
		switch (mProjectionType)
		{
		case ProjectionType::Perspective:
			set_perspective_projection_matrix(glm::radians(mPerspectiveFOV), width, height, mPerspectiveNear,
			                                  mPerspectiveFar);
			break;
		case ProjectionType::Orthographic:
			break;
		}
	}
}
