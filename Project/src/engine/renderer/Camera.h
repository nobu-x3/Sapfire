#pragma once

#include "engine/Math.h"

class Camera
{
	public:
	virtual ~Camera() {}
	virtual const Matrix4 &GetProjectionMatrix() const = 0;
	virtual const Matrix4 &GetViewMatrix() const = 0;
	virtual const Matrix4 &GetViewProjectionMatrix() const = 0;

	protected:
	virtual void RecalculateViewMatrix() = 0;
};

class FirstPersonCamera
{
	private:
	Matrix4 mProjectionMatrix;
	Matrix4 mViewMatrix;
};

class OrthographicCamera : public Camera
{
	public:
	OrthographicCamera(float left, float right, float bottom, float top);
	inline const Vector3 &GetPosition() const { return mPosition; }
	inline void SetPosition(const Vector3 &position)
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
	inline virtual const Matrix4 &GetProjectionMatrix() const override { return mProjectionMatrix; }
	inline virtual const Matrix4 &GetViewMatrix() const override { return mViewMatrix; }
	inline virtual const Matrix4 &GetViewProjectionMatrix() const override { return mViewProjectionMatrix; }

	protected:
	virtual void RecalculateViewMatrix() override;

	private:
	Matrix4 mProjectionMatrix;
	Matrix4 mViewMatrix;
	Matrix4 mViewProjectionMatrix;
	Vector3 mPosition;
	float mRotation;
};
