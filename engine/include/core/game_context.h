#pragma once

#include "assets/mesh_manager.h"
#include "components/ec_manager.h"
#include "components/transform.h"
#include "core/core.h"
#include "core/stl/unique_ptr.h"
#include "math/math_helper.h"
#include "physics/physics_engine.h"
#include "render/camera.h"
#include "render/graphics_device.h"
#include "render/material.h"

namespace Sapfire {

	namespace physics {
		class PhysicsEngine;
	}

	namespace d3d {
		class GraphicsDevice;
	}

	class Application;
	class Window;
	class ClientExtent;

	struct GameContextCreationDesc {
		ClientExtent* client_extent;
		HWND window_handle;
		stl::string mesh_registry_path{"mesh_registry.db"};
	};

	constexpr f32 CAMERA_FOV = TO_RADIANS(90);
	class SFAPI GameContext {
	public:
		GameContext(const GameContextCreationDesc& desc);
		virtual ~GameContext() {}
		void init();
        void create_render_component(Entity entity, stl::string& mesh_path);
		virtual void on_window_resize();
		virtual void load_contents() = 0;
		virtual void update(f32 delta_time);
		virtual void render() = 0;

	protected:
		stl::unique_ptr<d3d::GraphicsDevice> m_GraphicsDevice;
		stl::unique_ptr<physics::PhysicsEngine> m_PhysicsEngine;
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_RTIndexBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexPosBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexNormalBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexTangentBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexUVBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_TransformBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Material> m_Materials{};
        Sapfire::d3d::Buffer m_MainPassCB{};
		assets::MeshRegistry m_MeshRegistry;
		ECManager m_ECManager{};
		ClientExtent* m_ClientExtent;
		Camera m_MainCamera{};
	};
} // namespace Sapfire
