#pragma once
#include "GL/glew.h"
#include "engine/renderer/Shader.h"

class OpenGLShader : public Shader
{
	public:
	OpenGLShader() = default;
	~OpenGLShader();
	// tries to load v/f shaders with speicifed names
	virtual bool Load(const std::string &vertName, const std::string &fragName) override;
	virtual void Unload() override;

	// sets the active shader to this
	virtual void Bind() override;
	virtual void SetMatrixUniform(const std::string &name, const Matrix4 &matrix) override;
	virtual void SetVectorUniform(const std::string &name, const Vector3 &vec) override;
	virtual void SetFloatUniform(const std::string &name, float val) override;
	virtual void SetIntUniform(const std::string &name, int val) override;

	private:
	// tries to compile specified shader
	bool CompileShader(const std::string &filePath, GLenum shaderType, GLuint &outShader);
	// tests if compiled
	bool IsCompiled(GLuint shader);
	// tests if linked
	bool IsValidProgram();
	GLuint mVertexShader;
	GLuint mFragShader;
	GLuint mShaderProgram;
};
