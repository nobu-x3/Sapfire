#pragma once

#include "Sapfire/renderer/Shader.h"
#include <unordered_map>

namespace Sapfire
{
	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filePath);
		Ref<Shader> Get(const std::string& name);

	private:
		std::unordered_map<std::string, Ref<Shader>> mShaders;
	};
}