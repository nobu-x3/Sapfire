#pragma once

namespace Sapfire
{
	class Camera
	{
	public:
		virtual ~Camera() {};
		virtual const glm::mat4& get_projection_matrix() const = 0;
		virtual const glm::mat4& get_view_matrix() const = 0;
		virtual const glm::mat4& get_view_projection_matrix() const = 0;

	protected:
		virtual void recalculate_view_matrix() = 0;

	protected:
	};

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fov, float width, float height, float near, float farPlane);
		const glm::vec3& get_position() const { return mPosition; }

		void set_position(const glm::vec3& position)
		{
			mPosition = position;
			recalculate_view_matrix();
		}

		float get_rotation() const { return mRotation; }

		void set_rotation(float rotation)
		{
			mRotation = rotation;
			recalculate_view_matrix();
		}

		virtual const glm::mat4& get_projection_matrix() const override { return mProjectionMatrix; }
		virtual const glm::mat4& get_view_matrix() const override { return mViewMatrix; }
		virtual const glm::mat4& get_view_projection_matrix() const override { return mViewProjectionMatrix; }

	protected:
		virtual void recalculate_view_matrix() override;

	private:
		glm::vec3 mPosition{ 0.f, 0.f, 0.f };
		float mRotation{ 0.f };
		glm::mat4 mProjectionMatrix;
		glm::mat4 mViewMatrix;
		glm::mat4 mViewProjectionMatrix;
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		const glm::vec3& get_position() const { return mPosition; }

		void set_position(const glm::vec3& position)
		{
			mPosition = position;
			recalculate_view_matrix();
		}

		float get_rotation() const { return mRotation; }

		void set_rotation(float rotation)
		{
			mRotation = rotation;
			recalculate_view_matrix();
		}

		virtual const glm::mat4& get_projection_matrix() const override { return mProjectionMatrix; }
		virtual const glm::mat4& get_view_matrix() const override { return mViewMatrix; }
		virtual const glm::mat4& get_view_projection_matrix() const override { return mViewProjectionMatrix; }

	protected:
		virtual void recalculate_view_matrix() override;

	private:
		glm::vec3 mPosition{ 0.f, 0.f, 0.f };
		float mRotation;
		glm::mat4 mProjectionMatrix;
		glm::mat4 mViewMatrix;
		glm::mat4 mViewProjectionMatrix;
	};
}