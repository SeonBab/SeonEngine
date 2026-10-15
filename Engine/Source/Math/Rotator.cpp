#include "Math/Rotator.h"

#include <DirectXMath.h>

FQuat FRotator::ToQuat() const
{
	const DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(
		DirectX::XMConvertToRadians(pitch),
		DirectX::XMConvertToRadians(yaw),
		DirectX::XMConvertToRadians(roll));

	DirectX::XMFLOAT4 result;
	DirectX::XMStoreFloat4(&result, DirectX::XMQuaternionNormalize(rotation));

	return {result.x, result.y, result.z, result.w};
}
