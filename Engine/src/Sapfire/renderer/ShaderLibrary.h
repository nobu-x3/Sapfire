#pragma once

#include "Sapfire/renderer/Shader.h"
#include <unordered_map>

namespace Sapfire
{
	class ShaderLibrary
	{
	public:
		void add(const Ref<Shader>& shader);
		Ref<Shader> load(const std::string& filePath);
		Ref<Shader> get(const std::string& name);

	private:
		std::unordered_map<std::string, Ref<Shader>> mShaders;
	};
}