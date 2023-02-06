#pragma once

class Camera
{
	public:
	virtual ~Camera(){};
	virtual const glm::mat4 &GetProjectionMatrix() const = 0;
	virtual const glm::mat4 &GetViewMatrix() const = 0;
	virtual const glm::mat4 &GetViewProjectionMatrix() const = 0;

	protected:
	virtual void RecalculateViewMatrix() = 0;

	protected:
};

class PerspectiveCamera : public Camera
{
	public:
	PerspectiveCamera(float fov, float width, float height, float near, float far);
	inline const glm::vec3 &GetPosition() const { return mPosition; }
	inline void SetPosition(const glm::vec3 &position)
	{
		mPosition = position;
		RecalculateViewMatrix();
	}
	inline float GetRotation() const { return mRotation; }
	inline void SetRotation(float rotation)
	{
		mRotation = rotation;
		RecalculateViewMatrix();
	}

	inline virtual const glm::mat4 &GetProjectionMatrix() const override { return mProjectionMatrix; }
	inline virtual const glm::mat4 &GetViewMatrix() const override { return mViewMatrix; }
	inline virtual const glm::mat4 &GetViewProjectionMatrix() const override { return mViewProjectionMatrix; }

	protected:
	virtual void RecalculateViewMatrix() override;

	private:
	glm::vec3 mPosition{0.f, 0.f, 0.f};
	float mRotation{ 0.f };
	glm::mat4 mProjectionMatrix;
	glm::mat4 mViewMatrix;
	glm::mat4 mViewProjectionMatrix;
};

class OrthographicCamera : public Camera
{
	public:
	OrthographicCamera(float left, float right, float bottom, float top);
	inline const glm::vec3 &GetPosition() const { return mPosition; }
	inline void SetPosition(const glm::vec3 &position)
	{
		mPosition = position;
		RecalculateViewMatrix();
	}
	inline float GetRotation() const { return mRotation; }
	inline void SetRotation(float rotation)
	{
		mRotation = rotation;
		RecalculateViewMatrix();
	}
	inline virtual const glm::mat4 &GetProjectionMatrix() const override { return mProjectionMatrix; }
	inline virtual const glm::mat4 &GetViewMatrix() const override { return mViewMatrix; }
	inline virtual const glm::mat4 &GetViewProjectionMatrix() const override { return mViewProjectionMatrix; }

	protected:
	virtual void RecalculateViewMatrix() override;

	private:
	glm::vec3 mPosition{0.f, 0.f, 0.f};
	float mRotation;
	glm::mat4 mProjectionMatrix;
	glm::mat4 mViewMatrix;
	glm::mat4 mViewProjectionMatrix;
};
