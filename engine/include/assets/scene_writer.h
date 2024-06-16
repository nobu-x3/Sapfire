#pragma once

namespace Sapfire {
	class ECManager;
	namespace assets {
        class AssetManager;
	} // namespace assets
} // namespace Sapfire

namespace Sapfire::assets {
	class SFAPI SceneWriter {
	public:
		explicit SceneWriter(ECManager* ec, AssetManager* am);
		void serialize(const stl::string& scene_path);
		void deserealize(const stl::string& scene_path);

	private:
		ECManager& m_ECManager;
        AssetManager& m_AssetManager;
	};
} // namespace Sapfire::assets
