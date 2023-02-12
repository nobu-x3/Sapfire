#include "engpch.h"
#include "Camera.h"

namespace Sapfire
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: mProjectionMatrix(glm::ortho(left, right, bottom, top, -1.f, 1.f)), mViewMatrix(1.0f)
	{
		PROFILE_FUNCTION();
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	void OrthographicCamera::recalculate_view_matrix()
	{
		PROFILE_FUNCTION();
		glm::mat4 transform = translate(glm::mat4(1.0f), mPosition) *
			rotate(glm::mat4(1.0f), glm::radians(mRotation), glm::vec3(0, 0, 1));
		mViewMatrix = inverse(transform);
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	PerspectiveCamera::PerspectiveCamera(float fov, float width, float height, float nearPlane, float farPlane)
		: mProjectionMatrix(glm::perspectiveFov(glm::radians(fov), width, height, nearPlane, farPlane)), mViewMatrix(1.f)
	{
		PROFILE_FUNCTION();
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	void PerspectiveCamera::recalculate_view_matrix()
	{
		PROFILE_FUNCTION();
		glm::mat4 transform = translate(glm::mat4(1.0f), mPosition) *
			rotate(glm::mat4(1.0f), glm::radians(mRotation), glm::vec3(0, 0, 1));
		mViewMatrix = inverse(transform);
		/* mViewMatrix = transform; */
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}
}