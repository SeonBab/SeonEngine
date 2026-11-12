#include "Quaternion.h"

#include "Core/Math/EulerAngles.h"
#include "Core/Math/MathUtility.h"
#include "Core/Math/Matrix4x4.h"

#include <algorithm>
#include <cmath>

namespace
{
	// |sin(pitch)|가 이 값보다 크면 pitch를 90도나 -90도로 보고 짐벌락으로 처리한다. 각도 크기로 약 89.92도 이상이며 Unreal FQuat::Rotator와 같은 기준이다.
	constexpr float GimbalLockThreshold = 0.999999f;
}

Quaternion Quaternion::CreateFromEulerAngles(const EulerAngles& angles)
{
	// 쿼터니언은 각도의 절반을 쓴다.
	const float halfPitch = Math::DegreesToRadians(angles.pitch) * 0.5f;
	const float halfYaw = Math::DegreesToRadians(angles.yaw) * 0.5f;
	const float halfRoll = Math::DegreesToRadians(angles.roll) * 0.5f;

	const float cosPitch = std::cos(halfPitch);
	const float sinPitch = std::sin(halfPitch);
	const float cosYaw = std::cos(halfYaw);
	const float sinYaw = std::sin(halfYaw);
	const float cosRoll = std::cos(halfRoll);
	const float sinRoll = std::sin(halfRoll);

	// roll(Z), pitch(X), yaw(Y) 축 쿼터니언을 차례로 곱해 정리한 식이다.
	Quaternion result;
	result.x = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
	result.y = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
	result.z = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
	result.w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
	return result;
}

Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& rotation)
{
	// 대각선 합은 4 * w * w - 1이다.
	const float trace = rotation.m[0][0] + rotation.m[1][1] + rotation.m[2][2];

	Quaternion result;
	if (trace > 0.0f)
	{
		// 대각선 합이 양수면 w가 0.5보다 커서 4w로 나누어도 안전하다.
		const float fourW = std::sqrt(trace + 1.0f) * 2.0f;
		result.w = fourW * 0.25f;

		// 대각선을 사이에 두고 마주 보는 두 칸의 차는 4w에 각 성분을 곱한 값이다.
		result.x = (rotation.m[1][2] - rotation.m[2][1]) / fourW;
		result.y = (rotation.m[2][0] - rotation.m[0][2]) / fourW;
		result.z = (rotation.m[0][1] - rotation.m[1][0]) / fourW;
	}
	else if (rotation.m[0][0] >= rotation.m[1][1] && rotation.m[0][0] >= rotation.m[2][2])
	{
		// x가 가장 크고 0.5 이상이라 4x로 나누어도 안전하다.
		const float fourX = std::sqrt(1.0f + rotation.m[0][0] - rotation.m[1][1] - rotation.m[2][2]) * 2.0f;
		result.x = fourX * 0.25f;

		// 마주 보는 두 칸의 합은 x와 다른 성분의 곱, 차는 w와 x의 곱에 4를 곱한 값이다.
		result.y = (rotation.m[0][1] + rotation.m[1][0]) / fourX;
		result.z = (rotation.m[0][2] + rotation.m[2][0]) / fourX;
		result.w = (rotation.m[1][2] - rotation.m[2][1]) / fourX;
	}
	else if (rotation.m[1][1] >= rotation.m[2][2])
	{
		// y가 가장 크고 0.5 이상이라 4y로 나누어도 안전하다.
		const float fourY = std::sqrt(1.0f - rotation.m[0][0] + rotation.m[1][1] - rotation.m[2][2]) * 2.0f;
		result.y = fourY * 0.25f;

		result.x = (rotation.m[0][1] + rotation.m[1][0]) / fourY;
		result.z = (rotation.m[1][2] + rotation.m[2][1]) / fourY;
		result.w = (rotation.m[2][0] - rotation.m[0][2]) / fourY;
	}
	else
	{
		// z가 가장 크고 0.5 이상이라 4z로 나누어도 안전하다.
		const float fourZ = std::sqrt(1.0f - rotation.m[0][0] - rotation.m[1][1] + rotation.m[2][2]) * 2.0f;
		result.z = fourZ * 0.25f;

		result.x = (rotation.m[0][2] + rotation.m[2][0]) / fourZ;
		result.y = (rotation.m[1][2] + rotation.m[2][1]) / fourZ;
		result.w = (rotation.m[0][1] - rotation.m[1][0]) / fourZ;
	}

	return result;
}

EulerAngles Quaternion::ToEulerAngles() const
{
	// 회전 행렬의 칸을 쿼터니언으로 바로 계산해 각도를 거꾸로 읽는다. 행렬 [2][1]은 -sin(pitch)다.
	// float 오차로 1을 살짝 넘으면 asin이 실패하므로 범위를 자른다.
	const float sinPitch = std::clamp(2.0f * (w * x - y * z), -1.0f, 1.0f);

	EulerAngles result;
	if (std::abs(sinPitch) > GimbalLockThreshold)
	{
		// cos(pitch)가 0에 가까워 yaw와 roll이 같은 축을 돈다. roll을 0으로 두고 행렬 [0][2], [0][0]에서 yaw를 읽는다.
		result.pitch = std::copysign(90.0f, sinPitch);
		result.yaw = Math::RadiansToDegrees(std::atan2(2.0f * (w * y - x * z), 1.0f - 2.0f * (y * y + z * z)));
		result.roll = 0.0f;
	}
	else
	{
		result.pitch = Math::RadiansToDegrees(std::asin(sinPitch));
		// 행렬 [2][0]은 cos(pitch)sin(yaw), [2][2]는 cos(pitch)cos(yaw)다.
		result.yaw = Math::RadiansToDegrees(std::atan2(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y)));
		// 행렬 [0][1]은 sin(roll)cos(pitch), [1][1]은 cos(roll)cos(pitch)다.
		result.roll = Math::RadiansToDegrees(std::atan2(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z)));
	}

	return result;
}

Quaternion Quaternion::GetInverse() const
{
	return Quaternion{ -x, -y, -z, w };
}
