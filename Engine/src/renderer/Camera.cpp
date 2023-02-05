#include "Camera.h"
#include "engine/engpch.h"

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
    : mProjectionMatrix(glm::ortho(left, right, bottom, top, -1.f, 1.f)), mViewMatrix(1.0f)
{
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

void OrthographicCamera::RecalculateViewMatrix()
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), mPosition) *
			      glm::rotate(glm::mat4(1.0f), glm::radians(mRotation), glm::vec3(0, 0, 1));
	mViewMatrix = glm::inverse(transform);
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

PerspectiveCamera::PerspectiveCamera(float fov, float width, float height, float near, float far)
    : mProjectionMatrix(glm::perspectiveFov(glm::radians(fov), width, height, near, far)), mViewMatrix(1.f)
{
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

void PerspectiveCamera::RecalculateViewMatrix()
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), mPosition) *
			      glm::rotate(glm::mat4(1.0f), glm::radians(mRotation), glm::vec3(0, 0, 1));
	mViewMatrix = glm::inverse(transform);
	/* mViewMatrix = transform; */
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}
