#include "Math/Quat.h"

#include "Math/Rotator.h"

#include <DirectXMath.h>

#include <cmath>

FRotator FQuat::ToRotator() const
{
	// 고정 축 XYZ 각도 추출에 필요한 행 벡터 회전 행렬 성분이다.
	const float m11 = 1.0f - 2.0f * (y * y + z * z);
	const float m12 = 2.0f * (x * y + w * z);
	const float m13 = 2.0f * (x * z - w * y);
	const float m23 = 2.0f * (y * z + w * x);
	const float m33 = 1.0f - 2.0f * (x * x + y * y);
	const float cosYaw = std::sqrt(m11 * m11 + m12 * m12);

	return {
		DirectX::XMConvertToDegrees(std::atan2(m23, m33)),
		DirectX::XMConvertToDegrees(std::atan2(-m13, cosYaw)),
		DirectX::XMConvertToDegrees(std::atan2(m12, m11))};
}
