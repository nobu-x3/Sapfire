#pragma once
#include "GL/glew.h"
#include "engine/Math.h"
#include <string>
#include <unordered_map>

class Shader
{
	public:
	Shader() = default;
	~Shader() = default;
	// tries to load v/f shaders with speicifed names
	bool Load(const std::string &vertName, const std::string &fragName);
	void Unload();

	void SetMatrixUniform(const char *name, const Matrix4 &matrix);
	// sets the active shader to this
	void SetActive();

	private:
	// tries to compile specified shader
	bool CompileShader(const std::string &filePath, GLenum shaderType, GLuint &outShader);
	// tests if compiled
	bool IsCompiled(GLuint shader) const;
	// tests if linked
	bool IsValidProgram() const;
	GLuint mVertexShader;
	GLuint mFragShader;
	GLuint mShaderProgram;
	std::unordered_map<const char *, GLuint> mNameIdMap;
};
