#include "engpch.h"

#include <DirectXMath.h>
#include <cfloat>
#include "core/logger.h"
#include "tiny_obj_loader.h"
#include "tools/obj_loader.h"

namespace Sapfire::tools {

    using namespace DirectX;

	stl::optional<d3d::primitives::MeshData> OBJLoader::load_mesh(const stl::string& full_path) {
		tinyobj::attrib_t attrib;
		stl::vector<tinyobj::shape_t> shapes;
		stl::string warning, error;
		tinyobj::LoadObj(&attrib, &shapes, nullptr, &warning, &error, full_path.c_str());
		if (!warning.empty()) {
			CORE_WARN("Warning while loading obj file: {}", warning);
		}
		if (!error.empty()) {
			CORE_ERROR("Error while reading obj file: {}", error);
			return {};
		}
		d3d::primitives::MeshData mesh_data{};
		const DirectX::XMFLOAT3 v_min_f3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
		const DirectX::XMFLOAT3 v_max_f3{-FLT_MAX, -FLT_MAX, -FLT_MAX};
		DirectX::XMVECTOR v_min = DirectX::XMLoadFloat3(&v_min_f3);
		DirectX::XMVECTOR v_max = DirectX::XMLoadFloat3(&v_max_f3);
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				// hardcode loading to triangles
				const int fv = 3;
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					const tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					// vertex position
					const tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					const tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					const tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					// vertex normal
					const tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					const tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					const tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					// vertex uv
					const tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
					const tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];
					// copy it into our vertex
					mesh_data.positions.emplace_back(vx, vy, vz);
					mesh_data.normals.emplace_back(nx, ny, nz);
					mesh_data.texcs.emplace_back(ux, uy);
					mesh_data.indices32.emplace_back(index_offset + v);
					const DirectX::XMVECTOR positions{vx, vy, vz};
					v_min = DirectX::XMVectorMin(v_min, positions);
					v_max = DirectX::XMVectorMax(v_max, positions);
				}
				index_offset += fv;
			}
		}
		//DirectX::XMStoreFloat3(&mesh_data.aabb.Center, 0.5f * (v_min + v_max));
		//DirectX::XMStoreFloat3(&mesh_data.aabb.Extents, 0.5f * (v_max - v_min));
		DirectX::BoundingBox::CreateFromPoints(mesh_data.aabb, mesh_data.positions.size(), mesh_data.positions.data(),
											   sizeof(DirectX::XMFLOAT3));
		return mesh_data;
	}
} // namespace Sapfire::tools
