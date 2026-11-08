#include "Matrix4x4.h"

#include "MathUtility.h"
#include "Quaternion.h"
#include "Vector3.h"

#include <cmath>

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
	Matrix4x4 result;

	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			// 왼쪽 행렬의 행과 오른쪽 행렬의 열을 칸끼리 곱해 더한다.
			float sum = 0.0f;

			for (int index = 0; index < 4; ++index)
			{
				sum += m[row][index] * other.m[index][column];
			}

			result.m[row][column] = sum;
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::CreateScale(const Vector3& scale)
{
	Matrix4x4 result;

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;

	return result;
}

Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& position)
{
	Matrix4x4 result;

	result.m[3][0] = position.x;
	result.m[3][1] = position.y;
	result.m[3][2] = position.z;

	return result;
}

Matrix4x4 Matrix4x4::CreateFromQuaternion(const Quaternion& rotation)
{
	// 같은 곱이 여러 칸에 쓰이므로 미리 계산한다.
	const float xx = rotation.x * rotation.x;
	const float yy = rotation.y * rotation.y;
	const float zz = rotation.z * rotation.z;
	const float xy = rotation.x * rotation.y;
	const float xz = rotation.x * rotation.z;
	const float yz = rotation.y * rotation.z;
	const float wx = rotation.w * rotation.x;
	const float wy = rotation.w * rotation.y;
	const float wz = rotation.w * rotation.z;

	// 대각선은 1 - 2(나머지 두 축의 제곱 합), 마주 보는 칸은 w 항의 부호만 다르다.
	Matrix4x4 result;
	result.m[0][0] = 1.0f - 2.0f * (yy + zz);
	result.m[0][1] = 2.0f * (xy + wz);
	result.m[0][2] = 2.0f * (xz - wy);

	result.m[1][0] = 2.0f * (xy - wz);
	result.m[1][1] = 1.0f - 2.0f * (xx + zz);
	result.m[1][2] = 2.0f * (yz + wx);

	result.m[2][0] = 2.0f * (xz + wy);
	result.m[2][1] = 2.0f * (yz - wx);
	result.m[2][2] = 1.0f - 2.0f * (xx + yy);

	return result;
}

Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfView(float verticalFovDegrees, float aspectRatio, float nearZ, float farZ)
{
	// 시야각 경계가 화면 끝(-1과 1)이 되도록 맞추는 배율이다.
	const float height = 1.0f / std::tan(Math::DegreesToRadians(verticalFovDegrees) * 0.5f);
	const float width = height / aspectRatio;
	// near는 깊이 0, far는 깊이 1이 되도록 맞추는 배율이다.
	const float depthRange = farZ / (farZ - nearZ);

	Matrix4x4 result;
	result.m[0][0] = width;
	result.m[1][1] = height;
	result.m[2][2] = depthRange;
	// z를 w로 복사해 GPU의 원근 나눗셈(멀수록 작게)에 쓰게 한다.
	result.m[2][3] = 1.0f;
	result.m[3][2] = -depthRange * nearZ;
	result.m[3][3] = 0.0f;

	return result;
}

Matrix4x4 Matrix4x4::CreateOrthographicViewSize(float width, float height, float nearZ, float farZ)
{
	// near부터 far까지의 거리를 깊이 범위 0부터 1에 맞춘다.
	const float depthScale = 1.0f / (farZ - nearZ);

	Matrix4x4 result;
	result.m[0][0] = 2.0f / width;
	result.m[1][1] = 2.0f / height;
	result.m[2][2] = depthScale;
	result.m[3][2] = -nearZ * depthScale;
	return result;
}
