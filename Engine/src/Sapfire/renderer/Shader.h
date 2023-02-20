#pragma once
#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class Shader
	{
	public:
		virtual ~Shader() {}
		// sets the active shader to this
		virtual void bind() = 0;
		virtual void set_matrix_uniform(const std::string& name, const glm::mat4& matrix) = 0;
		virtual void set_vector_uniform(const std::string& name, const glm::vec3& vec) = 0;
		virtual void set_float_uniform(const std::string& name, float val) = 0;
		virtual void set_int_uniform(const std::string& name, int val) = 0;
		virtual const RendererID get_program_id() const = 0;
		virtual const std::string& get_name() const = 0;
		static Ref<Shader> create(const std::string& path);
	};
}