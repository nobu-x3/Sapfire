#include "engine/shader/OpenGLShader.h"
#include "GL/glew.h"
#include "engine/Log.h"

bool OpenGLShader::Load(const std::string &vertName, const std::string &fragName)
{
	// Compile vertex and pixel shaders
	if (!CompileShader(vertName, GL_VERTEX_SHADER, mVertexShader) ||
	    !CompileShader(fragName, GL_FRAGMENT_SHADER, mFragShader))
	{
		return false;
	}

	// Now create a shader program that
	// links together the vertex/frag shaders
	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragShader);
	glLinkProgram(mShaderProgram);

	// Verify that the program linked successfully
	if (!IsValidProgram())
	{
		return false;
	}

	return true;
}

void OpenGLShader::Unload()
{
	// Delete the program/shaders
	glDeleteProgram(mShaderProgram);
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragShader);
}

void OpenGLShader::SetActive()
{
	// Set this program as the active one
	glUseProgram(mShaderProgram);
}

void OpenGLShader::SetMatrixUniform(const std::string &name, const Matrix4 &matrix)
{
	// Find the uniform by this name
	GLuint loc = glGetUniformLocation(mShaderProgram, name.c_str());
	// Send the matrix data to the uniform
	glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.GetAsConstFloatPtr());
}

void OpenGLShader::SetVectorUniform(const std::string &name, const Vector3 &vec)
{
	GLuint loc = glGetUniformLocation(mShaderProgram, name.c_str());
	glUniform3fv(loc, 1, vec.GetAsConstFloatPtr());
}
void OpenGLShader::SetFloatUniform(const std::string &name, float val)
{
	GLuint loc = glGetUniformLocation(mShaderProgram, name.c_str());
	glUniform1f(loc, val);
}

bool OpenGLShader::CompileShader(const std::string &fileName, GLenum shaderType, GLuint &outShader)
{
	// Open file
	std::ifstream shaderFile(fileName);
	if (shaderFile.is_open())
	{
		// Read all the text into a string
		std::stringstream sstream;
		sstream << shaderFile.rdbuf();
		std::string contents = sstream.str();
		const char *contentsChar = contents.c_str();

		// Create a shader of the specified type
		outShader = glCreateShader(shaderType);
		// Set the source characters and try to compile
		glShaderSource(outShader, 1, &(contentsChar), nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			ENGINE_ERROR("Failed to compile shader: {0}", fileName);
			return false;
		}
	}
	else
	{
		ENGINE_ERROR("Shader file not found: {0}", fileName);
		return false;
	}

	return true;
}

bool OpenGLShader::IsCompiled(GLuint shader)
{
	GLint status;
	// Query the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		ENGINE_ERROR("GLSL Compile failed:\n{0}", buffer);
		return false;
	}

	return true;
}

bool OpenGLShader::IsValidProgram()
{

	GLint status;
	// Query the link status
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
		ENGINE_ERROR("GLSL Link status:\n{0}", buffer);
		return false;
	}

	return true;
}
