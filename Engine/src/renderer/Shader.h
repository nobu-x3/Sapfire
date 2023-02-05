#pragma once
#include "Core.h"

class Shader
{
	public:
	virtual ~Shader() {}
	// sets the active shader to this
	virtual void Bind() = 0;
	virtual void SetMatrixUniform(const std::string &name, const glm::mat4 &matrix) = 0;
	virtual void SetVectorUniform(const std::string &name, const glm::vec3 &vec) = 0;
	virtual void SetFloatUniform(const std::string &name, float val) = 0;
	virtual void SetIntUniform(const std::string &name, int val) = 0;
	virtual const std::string &GetName() const = 0;

	static Ref<Shader> Create(const std::string &path);
};
