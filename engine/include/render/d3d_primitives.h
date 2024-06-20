#pragma once
#include "DirectXMath.h"
#include "core/core.h"

namespace Sapfire::d3d::primitives {

	struct SFAPI Vertex {
		Vertex() {}
		Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv) :
			position(p), normal(n), tangentu(t), texc(uv) {}
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v) :
			position(px, py, pz), normal(nx, ny, nz), tangentu(tx, ty, tz), texc(u, v) {}

		DirectX::XMFLOAT3 position{};
		DirectX::XMFLOAT3 normal{};
		DirectX::XMFLOAT3 tangentu{};
		DirectX::XMFLOAT2 texc{};
	};

	struct SFAPI MeshData {
		stl::vector<DirectX::XMFLOAT3> positions;
		stl::vector<DirectX::XMFLOAT3> normals;
		stl::vector<DirectX::XMFLOAT3> tangentus;
		stl::vector<DirectX::XMFLOAT2> texcs;
		std::vector<u32> indices32;

		std::vector<u16>& indices16() {
			if (m_Indices16.empty()) {
				m_Indices16.resize(indices32.size());
				for (size_t i = 0; i < indices32.size(); ++i)
					m_Indices16[i] = static_cast<u16>(indices32[i]);
			}

			return m_Indices16;
		}

	private:
		std::vector<u16> m_Indices16;
	};

	///< summary>
	/// Creates a box centered at the origin with the given dimensions,
	/// where each face has m rows and n columns of vertices.
	///</summary>
	MeshData SFAPI create_box(float width, float height, float depth, u32 num_subdivisions);

	///< summary>
	/// Creates a quad aligned with the screen.  This is useful for
	/// postprocessing and screen effects.
	///</summary>
	MeshData SFAPI create_quad(float x, float y, float w, float h, float depth);

} // namespace Sapfire::d3d::primitives
