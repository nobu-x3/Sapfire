#pragma once

namespace Sapfire
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection);
		Camera(float degFov, float width, float height, float nearP, float farP);
		virtual ~Camera() = default;
		const glm::mat4& get_projection_matrix() const { return mProjectionMatrix; }

		void set_projection_matrix(const glm::mat4 projection)
		{
			mProjectionMatrix = projection;
		}

		void set_perspective_projection_matrix(const float radFov, const float width, const float height,
		                                       const float nearP, const float farP)
		{
			mProjectionMatrix = glm::perspectiveFov(radFov, width, height, nearP, farP);
		}

		void set_ortho_projection_matrix(const float width, const float height, const float nearP, const float farP)
		{
			mProjectionMatrix = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, nearP, farP);
		}

	private:
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
	};
}
