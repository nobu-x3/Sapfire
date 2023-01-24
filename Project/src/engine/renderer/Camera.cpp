#include "Camera.h"
#include "engine/engpch.h"

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
    : mProjectionMatrix(Matrix4::CreateOrtho(right - left, top - bottom, -1.f, 1.f)), mViewMatrix(Matrix4::Identity)
{
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

void OrthographicCamera::RecalculateViewMatrix()
{
	Matrix4 transform =
	    Matrix4::CreateTranslation(mPosition) * Matrix4::CreateRotationZ(Math::ToRadians(mRotation));
	transform.Invert();
	mViewMatrix = transform;
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}
