#pragma once
#include <glad/glad.h>
#include "Sapfire/renderer/Shader.h"

namespace Sapfire
{
	class OpenGLShader : public Shader
	{
	public:
		const std::unordered_map<std::string, GLenum> shaderTypes{ {"vertex", GL_VERTEX_SHADER},
									  {"fragment", GL_FRAGMENT_SHADER} };

	public:
		OpenGLShader() = default;
		OpenGLShader(const std::string& path);
		~OpenGLShader();

		// sets the active shader to this
		virtual void bind() override;
		virtual void set_matrix_uniform(const std::string& name, const glm::mat4& matrix) override;
		virtual void set_vector_uniform(const std::string& name, const glm::vec3& vec) override;
		virtual void set_float_uniform(const std::string& name, float val) override;
		virtual void set_int_uniform(const std::string& name, int val) override;
		virtual const std::string& get_name() const override { return mName; }
		virtual inline const RendererID get_program_id() const override { return mShaderProgram; }

	private:
		std::string ParseFile(const std::string& path);
		std::unordered_map<GLenum, std::string> Process(const std::string& source) const;
		// tries to compile specified shader
		bool CompileShader(const std::unordered_map<GLenum, std::string>& sources);
		// tests if compiled
		bool IsCompiled(GLuint shader);
		// tests if linked
		bool IsValidProgram() const;
	private:
		RendererID mShaderProgram;
		std::string mName;
	};
}