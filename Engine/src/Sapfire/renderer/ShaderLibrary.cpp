#include "engpch.h"
#include "ShaderLibrary.h"


namespace Sapfire
{
	void ShaderLibrary::add(const Ref<Shader>& shader)
	{
		PROFILE_FUNCTION();
		if (mShaders.find(shader->get_name()) == mShaders.end())
		{
			mShaders[shader->get_name()] = shader;
		}
	}

	Ref<Shader> ShaderLibrary::load(const std::string& filePath)
	{
		PROFILE_FUNCTION();
		auto shader = Shader::create(filePath);
		add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::get(const std::string& name)
	{
		PROFILE_FUNCTION();
		auto iter = mShaders.find(name);
		if (iter != mShaders.end())
		{
			return iter->second;
		}
		ENGINE_ERROR("Shader {0} does not exist in memory!", name);
		return nullptr;
	}
}