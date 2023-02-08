#include "engpch.h"
#include "ShaderLibrary.h"
#include "Sapfire/core/Log.h"

namespace Sapfire
{
	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		if (mShaders.find(shader->GetName()) == mShaders.end())
		{
			mShaders[shader->GetName()] = shader;
		}
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		auto iter = mShaders.find(name);
		if (iter != mShaders.end())
		{
			return iter->second;
		}
		else
		{
			ENGINE_ERROR("Shader {0} does not exist in memory!", name);
			return nullptr;
		}
	}
}