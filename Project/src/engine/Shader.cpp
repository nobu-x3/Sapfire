#include "Shader.h"
#include "SDL.h"
#include <SDL_log.h>
#include <fstream>
#include <sstream>

bool Shader::Load(const std::string &vertName, const std::string &fragName)
{
	if (!CompileShader(vertName, GL_VERTEX_SHADER, mVertexShader) ||
	    !CompileShader(fragName, GL_FRAGMENT_SHADER, mFragShader))
	{
		return false;
	}

	// link vertex and frag shaders
	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragShader);
	glLinkProgram(mShaderProgram);

	if (!IsValidProgram())
	{
		return false;
	}
	return true;
}

void Shader::Unload()
{
	glDeleteProgram(mShaderProgram);
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragShader);
}

void Shader::SetActive()
{
	glUseProgram(mShaderProgram);
}

void Shader::SetMatrixUniform(const char *name, const Matrix4 &matrix)
{

	// Find the uniform by this name
	GLuint loc = glGetUniformLocation(mShaderProgram, name);
	// Send the matrix data to the uniform
	glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.GetAsFloatPtr());
	/* // TODO: implement via uniform buffer objects (UBOs) */
	/* auto iter = mNameIdMap.find(name); */
	/* if (iter != mNameIdMap.end()) */
	/* { */
	/* 	glUniformMatrix4fv(iter->second,	  // Uniform ID */
	/* 			   1,			  // Number of matrices */
	/* 			   GL_TRUE,		  // TRUE if row vectors */
	/* 			   matrix.GetAsFloatPtr() // Pointer to matrix data */
	/* 	); */
	/* } */
	/* else */
	/* { */
	/* 	GLuint loc = glGetUniformLocation(mShaderProgram, name); */
	/* 	mNameIdMap.emplace(name, loc); */
	/* 	glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.GetAsFloatPtr()); */
	/* } */
}

bool Shader::CompileShader(const std::string &filePath, GLenum shaderType, GLuint &outShader)
{
	std::ifstream shaderFile(filePath);
	if (shaderFile.is_open())
	{
		// read file contents
		std::stringstream sstream;
		sstream << shaderFile.rdbuf();
		std::string contents = sstream.str();
		const char *contentsChar = contents.c_str();

		// Create shaderType
		outShader = glCreateShader(shaderType);
		glShaderSource(outShader, 1, &(contentsChar), nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			SDL_Log("Failed to compile shader %s", filePath.c_str());
			return false;
		}
	}
	else
	{
		SDL_Log("Shader file not found: %s", filePath.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader) const
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		SDL_Log("GLSL compile failed:\n%s", buffer);
		return false;
	}
	return true;
}

bool Shader::IsValidProgram() const
{
	GLint status;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
		SDL_Log("GLSL compile failed:\n%s", buffer);
		return false;
	}
	return true;
}
