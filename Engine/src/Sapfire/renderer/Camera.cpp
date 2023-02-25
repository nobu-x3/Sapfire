#include "engpch.h"
#include "Camera.h"

namespace Sapfire
{
	Camera::Camera(const glm::mat4& projection) : mProjectionMatrix(projection)
	{
	}

	Camera::Camera(float degFov, float width, float height, float nearP, float farP) : mProjectionMatrix(
		glm::perspectiveFov(glm::radians(degFov), width, height, nearP, farP))
	{
	}
}
