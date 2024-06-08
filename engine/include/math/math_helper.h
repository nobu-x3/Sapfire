#pragma once
#include <DirectXMath.h>

namespace Sapfire::math {

#define TO_RADIANS(degrees) degrees* DirectX::XM_PI / 180.f
#define TO_DEGREES(radians) radians * 180.f / DirectX::XM_PI

	inline DirectX::XMFLOAT4X4 Identity4x4() {
		static DirectX::XMFLOAT4X4 I(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

} // namespace Sapfire::math
