#pragma once
#include <cmath>
#include <limits>

namespace Math
{
const float Pi = 3.1415926535f;
const float TwoPi = Pi * 2.0f;
const float PiOver2 = Pi / 2.0f;
const float Infinity = std::numeric_limits<float>::infinity();
const float NegInfinity = -std::numeric_limits<float>::infinity();
inline float ToDegrees(float radians)
{
	return radians * 180.0f / Pi;
}
} // namespace Math
