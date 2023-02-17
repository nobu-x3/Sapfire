#pragma once
#include "Sapfire/core/Core.h"

namespace Sapfire
{
	class Camera;
	class Shader;
	class CubeMap;
	class Mesh;

	class Skybox
	{
	public:
		Skybox(const std::string& meshPath, const std::string& shaderPath, std::array<std::string, 6> textureFaces);
		void draw(const Camera& camera);
	private:
		Ref<Mesh> mMesh;
		Ref<CubeMap> mCubeMap;
		Ref<Shader> mShader;
	};
	
}
