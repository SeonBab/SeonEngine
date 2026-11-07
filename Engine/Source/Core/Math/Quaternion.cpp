#include "Quaternion.h"

#include "Core/Math/EulerAngles.h"
#include "Core/Math/MathUtility.h"

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
