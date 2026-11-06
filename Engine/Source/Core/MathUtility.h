#pragma once

#include <numbers>

namespace Math
{
	// 직접 자릿수를 적지 않고 표준 상수를 쓴다. 3.141592653589793을 float로 바꾼 값(약 3.1415927)과 같다.
	constexpr float Pi = std::numbers::pi_v<float>;

	/** 도를 라디안으로 바꾼다. */
	constexpr float DegreesToRadians(float degrees)
	{
		return degrees * (Pi / 180.0f);
	}

	/** 라디안을 도로 바꾼다. */
	constexpr float RadiansToDegrees(float radians)
	{
		return radians * (180.0f / Pi);
	}
}
