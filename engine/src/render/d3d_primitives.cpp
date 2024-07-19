#include "engpch.h"

#include "render/d3d_primitives.h"

namespace Sapfire::d3d::primitives {

	using namespace DirectX;
	void subdivide(MeshData&);
	Vertex mid_point(const Vertex&, const Vertex&);

	MeshData create_box(float width, float height, float depth, u32 num_subdivisions) {
		MeshData meshData;
		//
		// Create the vertices.
		//
		Vertex v[24];
		const float w2 = 0.5f * width;
		const float h2 = 0.5f * height;
		const float d2 = 0.5f * depth;
		// Fill in the front face vertex data.
		v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		// Fill in the back face vertex data.
		v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		// Fill in the top face vertex data.
		v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		// Fill in the bottom face vertex data.
		v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		// Fill in the left face vertex data.
		v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
		v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
		v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
		// Fill in the right face vertex data.
		v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
		v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		meshData.positions.reserve(24);
		meshData.normals.reserve(24);
		meshData.tangentus.reserve(24);
		meshData.texcs.reserve(24);
		for (const auto& vertex : v) {
			meshData.positions.push_back(vertex.position);
			meshData.normals.push_back(vertex.normal);
			meshData.tangentus.push_back(vertex.tangentu);
			meshData.texcs.push_back(vertex.texc);
		}
		//
		// Create the indices.
		//
		u32 i[36];
		// Fill in the front face index data
		i[0] = 0;
		i[1] = 1;
		i[2] = 2;
		i[3] = 0;
		i[4] = 2;
		i[5] = 3;
		// Fill in the back face index data
		i[6] = 4;
		i[7] = 5;
		i[8] = 6;
		i[9] = 4;
		i[10] = 6;
		i[11] = 7;
		// Fill in the top face index data
		i[12] = 8;
		i[13] = 9;
		i[14] = 10;
		i[15] = 8;
		i[16] = 10;
		i[17] = 11;
		// Fill in the bottom face index data
		i[18] = 12;
		i[19] = 13;
		i[20] = 14;
		i[21] = 12;
		i[22] = 14;
		i[23] = 15;
		// Fill in the left face index data
		i[24] = 16;
		i[25] = 17;
		i[26] = 18;
		i[27] = 16;
		i[28] = 18;
		i[29] = 19;
		// Fill in the right face index data
		i[30] = 20;
		i[31] = 21;
		i[32] = 22;
		i[33] = 20;
		i[34] = 22;
		i[35] = 23;
		meshData.indices32.assign(&i[0], &i[36]);
		// Put a cap on the number of subdivisions.
		num_subdivisions = std::min<u32>(num_subdivisions, 6u);
		for (u32 i = 0; i < num_subdivisions; ++i)
			subdivide(meshData);
		return meshData;
	}

	MeshData create_quad(float x, float y, float w, float h, float depth) {
		MeshData meshData;
		stl::vector<Vertex> vertices{4};
		meshData.indices32.resize(6);
		// Position coordinates specified in NDC space.
		vertices[0] = Vertex(x, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		vertices[1] = Vertex(x, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		vertices[2] = Vertex(x + w, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		vertices[3] = Vertex(x + w, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		for (auto& vertex : vertices) {
			meshData.positions.push_back(vertex.position);
			meshData.normals.push_back(vertex.normal);
			meshData.tangentus.push_back(vertex.tangentu);
			meshData.texcs.push_back(vertex.texc);
		}
		meshData.indices32[0] = 0;
		meshData.indices32[1] = 1;
		meshData.indices32[2] = 2;
		meshData.indices32[3] = 0;
		meshData.indices32[4] = 2;
		meshData.indices32[5] = 3;
		return meshData;
	}

	void subdivide(MeshData& data) {
		// Save a copy of the input geometry.
		const MeshData inputCopy = data;
		data.positions.resize(0);
		data.tangentus.resize(0);
		data.normals.resize(0);
		data.texcs.resize(0);
		data.indices32.resize(0);
		//       v1
		//       *
		//      / \
        //     /   \
        //  m0*-----*m1
		//   / \   / \
        //  /   \ /   \
        // *-----*-----*
		// v0    m2     v2
		/* u32 numTris = (u32)inputCopy.indices32.size() / 3; */
		/* for (u32 i = 0; i < numTris; ++i) { */
		/* 	Vertex v0 = inputCopy.vertices[inputCopy.indices32[i * 3 + 0]]; */
		/* 	Vertex v1 = inputCopy.vertices[inputCopy.indices32[i * 3 + 1]]; */
		/* 	Vertex v2 = inputCopy.vertices[inputCopy.indices32[i * 3 + 2]]; */
		/* 	// */
		/* 	// Generate the midpoints. */
		/* 	// */
		/* 	Vertex m0 = mid_point(v0, v1); */
		/* 	Vertex m1 = mid_point(v1, v2); */
		/* 	Vertex m2 = mid_point(v0, v2); */
		/* 	// */
		/* 	// Add new geometry. */
		/* 	// */
		/* 	data.vertices.push_back(v0); // 0 */
		/* 	data.vertices.push_back(v1); // 1 */
		/* 	data.vertices.push_back(v2); // 2 */
		/* 	data.vertices.push_back(m0); // 3 */
		/* 	data.vertices.push_back(m1); // 4 */
		/* 	data.vertices.push_back(m2); // 5 */
		/* 	data.indices32.push_back(i * 6 + 0); */
		/* 	data.indices32.push_back(i * 6 + 3); */
		/* 	data.indices32.push_back(i * 6 + 5); */
		/* 	data.indices32.push_back(i * 6 + 3); */
		/* 	data.indices32.push_back(i * 6 + 4); */
		/* 	data.indices32.push_back(i * 6 + 5); */
		/* 	data.indices32.push_back(i * 6 + 5); */
		/* 	data.indices32.push_back(i * 6 + 4); */
		/* 	data.indices32.push_back(i * 6 + 2); */
		/* 	data.indices32.push_back(i * 6 + 3); */
		/* 	data.indices32.push_back(i * 6 + 1); */
		/* 	data.indices32.push_back(i * 6 + 4); */
		/* } */
	}

	Vertex mid_point(const Vertex& v0, const Vertex& v1) {
		const XMVECTOR p0 = XMLoadFloat3(&v0.position);
		const XMVECTOR p1 = XMLoadFloat3(&v1.position);
		const XMVECTOR n0 = XMLoadFloat3(&v0.normal);
		const XMVECTOR n1 = XMLoadFloat3(&v1.normal);
		const XMVECTOR tan0 = XMLoadFloat3(&v0.tangentu);
		const XMVECTOR tan1 = XMLoadFloat3(&v1.tangentu);
		const XMVECTOR tex0 = XMLoadFloat2(&v0.texc);
		const XMVECTOR tex1 = XMLoadFloat2(&v1.texc);
		// Compute the midpoints of all the attributes.  Vectors need to be
		// normalized since linear interpolating can make them not unit length.
		const XMVECTOR pos = 0.5f * (p0 + p1);
		const XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
		const XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
		const XMVECTOR tex = 0.5f * (tex0 + tex1);
		Vertex v;
		XMStoreFloat3(&v.position, pos);
		XMStoreFloat3(&v.normal, normal);
		XMStoreFloat3(&v.tangentu, tangent);
		XMStoreFloat2(&v.texc, tex);
		return v;
	}
} // namespace Sapfire::d3d::primitives
