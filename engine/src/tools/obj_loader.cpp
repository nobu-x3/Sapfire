#include "engpch.h"

#include "core/logger.h"
#include "tiny_obj_loader.h"
#include "tools/obj_loader.h"

namespace Sapfire::tools {

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
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				// hardcode loading to triangles
				int fv = 3;
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					// vertex position
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					// vertex normal
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					// vertex uv
					tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];
					// copy it into our vertex
					mesh_data.positions.emplace_back(vx, vy, vz);
					mesh_data.normals.emplace_back(nx, ny, nz);
					mesh_data.texcs.emplace_back(ux, uy);
                    mesh_data.indices32.emplace_back(index_offset + v);
				}
				index_offset += fv;
			}
		}
		return mesh_data;
	}
} // namespace Sapfire::tools
