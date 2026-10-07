#include "Math/Transform.h"

#include <DirectXMath.h>

FMatrix4 FTransform::ToMatrixWithScale() const
{
	const DirectX::XMVECTOR scaling = DirectX::XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
	const DirectX::XMVECTOR rotationQuaternion = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	const DirectX::XMVECTOR translation = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	const DirectX::XMMATRIX matrix = DirectX::XMMatrixAffineTransformation(scaling, DirectX::XMVectorZero(), rotationQuaternion, translation);

	DirectX::XMFLOAT4X4 storedMatrix;
	DirectX::XMStoreFloat4x4(&storedMatrix, matrix);

	FMatrix4 result;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			result.m[row][column] = storedMatrix.m[row][column];
		}
	}

	return result;
}

FMatrix4 FTransform::ToMatrixNoScale() const
{
	const DirectX::XMVECTOR scaling = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	const DirectX::XMVECTOR rotationQuaternion = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	const DirectX::XMVECTOR translation = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	const DirectX::XMMATRIX matrix = DirectX::XMMatrixAffineTransformation(scaling, DirectX::XMVectorZero(), rotationQuaternion, translation);

	DirectX::XMFLOAT4X4 storedMatrix;
	DirectX::XMStoreFloat4x4(&storedMatrix, matrix);

	FMatrix4 result;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			result.m[row][column] = storedMatrix.m[row][column];
		}
	}

	return result;
}
