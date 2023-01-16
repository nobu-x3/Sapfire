#pragma once
#include "GL/glew.h"
#include "engine/Math.h"

class Shader
{
	public:
	Shader() = default;
	~Shader() = default;
	// tries to load v/f shaders with speicifed names
	bool Load(const std::string &vertName, const std::string &fragName);
	void Unload();

	// sets the active shader to this
	void SetActive();
	void SetMatrixUniform(const std::string &name, const Matrix4 &matrix);
	void SetVectorUniform(const std::string &name, const Vector3 &vec);
	void SetFloatUniform(const std::string &name, float val);

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
