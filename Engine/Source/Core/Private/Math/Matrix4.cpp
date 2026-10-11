#include "Math/Matrix4.h"

#include <DirectXMath.h>

#include <cmath>

void FMatrix4::SetIdentity()
{
	*this = FMatrix4{};
	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;
}

FMatrix4 FMatrix4::operator*(const FMatrix4& other) const
{
	DirectX::XMFLOAT4X4 storedLeft;
	DirectX::XMFLOAT4X4 storedRight;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			storedLeft.m[row][column] = m[row][column];
			storedRight.m[row][column] = other.m[row][column];
		}
	}

	const DirectX::XMMATRIX leftMatrix = DirectX::XMLoadFloat4x4(&storedLeft);
	const DirectX::XMMATRIX rightMatrix = DirectX::XMLoadFloat4x4(&storedRight);
	const DirectX::XMMATRIX product = DirectX::XMMatrixMultiply(leftMatrix, rightMatrix);
	DirectX::XMStoreFloat4x4(&storedLeft, product);

	FMatrix4 result;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			result.m[row][column] = storedLeft.m[row][column];
		}
	}

	return result;
}

FMatrix4 FMatrix4::Inverse() const
{
	DirectX::XMFLOAT4X4 storedMatrix;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			storedMatrix.m[row][column] = m[row][column];
		}
	}

	const DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(&storedMatrix);
	DirectX::XMVECTOR determinant;
	const DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&determinant, matrix);
	const float determinantValue = DirectX::XMVectorGetX(determinant);

	FMatrix4 result;
	if (determinantValue == 0.0f || !std::isfinite(determinantValue))
	{
		result.SetIdentity();

		return result;
	}

	DirectX::XMStoreFloat4x4(&storedMatrix, inverse);
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			if (!std::isfinite(storedMatrix.m[row][column]))
			{
				result.SetIdentity();

				return result;
			}
			result.m[row][column] = storedMatrix.m[row][column];
		}
	}

	return result;
}
