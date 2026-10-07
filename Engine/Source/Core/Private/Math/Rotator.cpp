#include "Math/Rotator.h"

#include <DirectXMath.h>

FQuat FRotator::ToQuat() const
{
	const DirectX::XMVECTOR pitchRotation = DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), DirectX::XMConvertToRadians(pitch));
	const DirectX::XMVECTOR yawRotation = DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), DirectX::XMConvertToRadians(yaw));
	const DirectX::XMVECTOR rollRotation = DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMConvertToRadians(roll));

	// XMQuaternionMultiply(a, b)는 a 회전 다음 b 회전을 적용한다.
	const DirectX::XMVECTOR rotation = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(pitchRotation, yawRotation), rollRotation);
	DirectX::XMFLOAT4 result;
	DirectX::XMStoreFloat4(&result, DirectX::XMQuaternionNormalize(rotation));

	return {result.x, result.y, result.z, result.w};
}
