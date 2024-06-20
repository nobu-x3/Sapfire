#pragma once

namespace Sapfire {
	class ECManager;
	class RenderComponentResourcePaths;
	namespace assets {
		class AssetManager;
	} // namespace assets
} // namespace Sapfire

namespace Sapfire::assets {
	class SFAPI SceneWriter {
	public:
		explicit SceneWriter(ECManager* ec, AssetManager* am);
		void serialize(const stl::string& scene_path);
		void deserealize(const stl::string& scene_path,
						 stl::function<void(Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths)>
							 render_component_setter);

	private:
		ECManager& m_ECManager;
		AssetManager& m_AssetManager;
	};
} // namespace Sapfire::assets
