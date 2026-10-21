#include "Quat.h"

#include "Core/Math/Rotator.h"

#include <cmath>
#include <numbers>

bool FQuat::IsNormalized() const
{
	constexpr float Tolerance = 0.01f;
	const float lengthSquared = x * x + y * y + z * z + w * w;

	return std::abs(1.0f - lengthSquared) <= Tolerance;
}

FRotator FQuat::ToRotator() const
{
	const double qx = x;
	const double qy = y;
	const double qz = z;
	const double qw = w;

	// 고정 축 Z, X, Y 순서로 적용하는 행 벡터 회전의 성분이다. 대각 성분도 제곱합으로 계산해 상쇄 오차를 줄인다.
	const double m11 = qw * qw + qx * qx - qy * qy - qz * qz;
	const double m12 = 2.0 * (qx * qy + qw * qz);
	const double m13 = 2.0 * (qx * qz - qw * qy);
	const double m22 = qw * qw - qx * qx + qy * qy - qz * qz;
	const double m31 = 2.0 * (qx * qz + qw * qy);
	const double m32 = 2.0 * (qy * qz - qw * qx);
	const double m33 = qw * qw - qx * qx - qy * qy + qz * qz;
	const double cosPitch = std::hypot(m31, m33);
	constexpr double SingularTolerance = 1.0e-6;
	constexpr double RadiansToDegrees = 180.0 / std::numbers::pi;

	const double pitchRadians = std::atan2(-m32, cosPitch);
	double yawRadians;
	double rollRadians;
	if (cosPitch > SingularTolerance)
	{
		yawRadians = std::atan2(m31, m33);
		rollRadians = std::atan2(m12, m22);
	}
	else
	{
		// pitch ±90도에서 겹치는 yaw와 roll은 roll 0인 표현으로 선택한다.
		yawRadians = std::atan2(-m13, m11);
		rollRadians = 0.0;
	}

	return {
		static_cast<float>(pitchRadians * RadiansToDegrees),
		static_cast<float>(yawRadians * RadiansToDegrees),
		static_cast<float>(rollRadians * RadiansToDegrees)};
}
