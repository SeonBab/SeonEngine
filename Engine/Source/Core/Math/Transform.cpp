#include "Transform.h"

#include "Matrix4x4.h"
#include "Quaternion.h"

#include <cmath>

Matrix4x4 Transform::ToMatrix() const
{
	return Matrix4x4::CreateScale(scale) * Matrix4x4::CreateFromQuaternion(rotation) * Matrix4x4::CreateTranslation(position);
}

Transform Transform::CreateFromMatrix(const Matrix4x4& matrix)
{
	Transform result;

	// 이동은 마지막 행에 그대로 있다.
	result.position.x = matrix.m[3][0];
	result.position.y = matrix.m[3][1];
	result.position.z = matrix.m[3][2];

	// 위 세 행은 길이 1인 회전된 축에 크기를 곱한 것이라, 각 행의 길이가 그 축의 크기다.
	result.scale.x = std::sqrt(matrix.m[0][0] * matrix.m[0][0] + matrix.m[0][1] * matrix.m[0][1] + matrix.m[0][2] * matrix.m[0][2]);
	result.scale.y = std::sqrt(matrix.m[1][0] * matrix.m[1][0] + matrix.m[1][1] * matrix.m[1][1] + matrix.m[1][2] * matrix.m[1][2]);
	result.scale.z = std::sqrt(matrix.m[2][0] * matrix.m[2][0] + matrix.m[2][1] * matrix.m[2][1] + matrix.m[2][2] * matrix.m[2][2]);

	// 크기가 0인 축이 있으면 그 축의 방향을 알 수 없다. 그런 물체는 보이지 않으므로 회전 없음으로 둔다.
	if (result.scale.x > 0.0f && result.scale.y > 0.0f && result.scale.z > 0.0f)
	{
		// 위 세 행의 행렬식이 음수면 거울이 섞여 있다. 어느 축이 음수였는지는 행렬만으로 알 수 없어 x 축을 음수로 본다.
		// x 크기가 음수가 되면 아래에서 0행을 나눌 때 부호가 함께 뒤집혀 순수한 회전 행렬이 남는다.
		const float determinant =
			matrix.m[0][0] * (matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1])
			- matrix.m[0][1] * (matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0])
			+ matrix.m[0][2] * (matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0]);
		if (determinant < 0.0f)
		{
			result.scale.x = -result.scale.x;
		}

		// 각 행을 그 축의 크기로 나누면 길이 1인 회전된 축, 곧 회전 행렬이 남는다.
		Matrix4x4 rotation;
		rotation.m[0][0] = matrix.m[0][0] / result.scale.x;
		rotation.m[0][1] = matrix.m[0][1] / result.scale.x;
		rotation.m[0][2] = matrix.m[0][2] / result.scale.x;
		rotation.m[1][0] = matrix.m[1][0] / result.scale.y;
		rotation.m[1][1] = matrix.m[1][1] / result.scale.y;
		rotation.m[1][2] = matrix.m[1][2] / result.scale.y;
		rotation.m[2][0] = matrix.m[2][0] / result.scale.z;
		rotation.m[2][1] = matrix.m[2][1] / result.scale.z;
		rotation.m[2][2] = matrix.m[2][2] / result.scale.z;

		result.rotation = Quaternion::CreateFromRotationMatrix(rotation);
	}

	return result;
}
