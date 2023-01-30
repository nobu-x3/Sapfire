#include "engine/renderer/opengl/OpenGLShader.h"
#include "engine/Log.h"
#include "engine/renderer/RendererNew.h"
#include <fstream>

Ref<Shader> Shader::Create(const std::string &path)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::OpenGL: {
		return std::make_shared<OpenGLShader>(path);
	}
	default:
		ENGINE_ERROR("Unknown RenderAPI!");
		return nullptr;
	}
	return nullptr;
}

OpenGLShader::OpenGLShader(const std::string &path)
{
	std::string source = ParseFile(path);
	auto shaderSources = Process(source);
	CompileShader(shaderSources);
}

OpenGLShader::~OpenGLShader()
{
	glDeleteProgram(mShaderProgram);
}

std::string OpenGLShader::ParseFile(const std::string &path)
{
	std::string result;
	std::ifstream in(path, std::ios::in);
	if (in)
	{
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());
		in.close();
	}
	else
	{
		ENGINE_ERROR("Could not open file at {0}", path);
	}
	return result;
}

std::unordered_map<GLenum, std::string> OpenGLShader::Process(const std::string &source)
{
	std::unordered_map<GLenum, std::string> shaderSources;

	const char *typeToken = "#type";
	size_t typeTokenLen = strlen(typeToken);
	size_t pos = source.find(typeToken, 0);
	while (pos != std::string::npos)
	{
		size_t endOfLine = source.find_first_of("\r\n", pos);
		if (endOfLine == std::string::npos)
		{
			ENGINE_ERROR("Shader syntax error!");
			return shaderSources;
		}
		size_t begin = pos + typeTokenLen + 1; // +1 is space between token and the type
		std::string type = source.substr(begin, endOfLine - begin);
		if (type != "vertex" && type != "fragment")
		{
			ENGINE_ERROR("Invalid shader type specifier '{0}'", type);
			return shaderSources;
		}
		size_t nextLinePos = source.find_first_not_of("\r\n", endOfLine);
		pos = source.find(typeToken, nextLinePos);
		auto iter = shaderTypes.find(type);
		if (iter == shaderTypes.end())
		{
			ENGINE_ERROR("Invalid shader type specifier '{0}'", type);
			return shaderSources;
		}
		shaderSources[iter->second] = source.substr(
		    nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
	}

	return shaderSources;
}

bool OpenGLShader::CompileShader(const std::unordered_map<GLenum, std::string> &sources)
{
	// TODO: in case of failures, most likely leaks memory due to not keeping track of specific shader ids
	RendererID program = glCreateProgram();
	std::vector<GLuint> shaderIds;
	shaderIds.reserve(sources.size());
	for (auto &src : sources)
	{
		GLenum shaderType = src.first;
		const std::string &contents = src.second;
		GLuint outShader = glCreateShader(shaderType);
		const GLchar *contensCstr = contents.c_str();
		// Set the source characters and try to compile
		glShaderSource(outShader, 1, &(contensCstr), nullptr);
		glCompileShader(outShader);
		if (!IsCompiled(outShader))
		{
			ENGINE_ERROR("Shader failed to compile:\n{0}", contents);
			for (auto &shader : shaderIds)
			{
				glDeleteShader(shader);
			}
			glDeleteProgram(mShaderProgram);
			return false;
		}
		glAttachShader(program, outShader);
		shaderIds.push_back(outShader);
	}
	mShaderProgram = program;
	glLinkProgram(mShaderProgram);
	// Verify that the program linked successfully
	if (!IsValidProgram())
	{
		ENGINE_ERROR("Shader failed to link!");
		for (auto &shader : shaderIds)
		{
			glDeleteShader(shader);
		}
		glDeleteProgram(mShaderProgram);
		return false;
	}
	for (auto &id : shaderIds)
	{
		glDetachShader(mShaderProgram, id);
	}
	return true;
}

void OpenGLShader::Bind()
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

void OpenGLShader::SetIntUniform(const std::string &name, int val)
{
	GLuint loc = glGetUniformLocation(mShaderProgram, name.c_str());
	glUniform1i(loc, val);
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
