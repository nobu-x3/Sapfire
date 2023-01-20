#pragma once
#include "engine/Math.h"
class Shader
{
	public:
	virtual ~Shader() {}
	virtual bool Load(const std::string &vertName, const std::string &fragName) = 0;
	virtual void Unload() = 0;

	// sets the active shader to this
	virtual void SetActive() = 0;
	virtual void SetMatrixUniform(const std::string &name, const Matrix4 &matrix) = 0;
	virtual void SetVectorUniform(const std::string &name, const Vector3 &vec) = 0;
	virtual void SetFloatUniform(const std::string &name, float val) = 0;
};
