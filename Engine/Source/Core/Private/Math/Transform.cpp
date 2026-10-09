#include "Math/Transform.h"

#include <DirectXMath.h>

#include <cmath>

namespace
{
	FVector3 GetSafeScaleReciprocal(const FVector3& inScale)
	{
		constexpr float tolerance = 1.0e-8f;

		return {
			std::abs(inScale.x) <= tolerance ? 0.0f : 1.0f / inScale.x,
			std::abs(inScale.y) <= tolerance ? 0.0f : 1.0f / inScale.y,
			std::abs(inScale.z) <= tolerance ? 0.0f : 1.0f / inScale.z};
	}

	FQuat RotationFromMatrix(const DirectX::XMMATRIX& matrix)
	{
		DirectX::XMFLOAT4X4 storedMatrix;
		DirectX::XMStoreFloat4x4(&storedMatrix, matrix);
		const float trace = storedMatrix.m[0][0] + storedMatrix.m[1][1] + storedMatrix.m[2][2];
		float quaternion[4]{};
		if (trace > 0.0f)
		{
			const float inverseRoot = 1.0f / std::sqrt(trace + 1.0f);
			const float factor = 0.5f * inverseRoot;
			quaternion[0] = (storedMatrix.m[1][2] - storedMatrix.m[2][1]) * factor;
			quaternion[1] = (storedMatrix.m[2][0] - storedMatrix.m[0][2]) * factor;
			quaternion[2] = (storedMatrix.m[0][1] - storedMatrix.m[1][0]) * factor;
			quaternion[3] = 0.5f / inverseRoot;
		}
		else
		{
			int axis = 0;
			if (storedMatrix.m[1][1] > storedMatrix.m[0][0])
			{
				axis = 1;
			}
			if (storedMatrix.m[2][2] > storedMatrix.m[axis][axis])
			{
				axis = 2;
			}
			const int nextAxis = (axis + 1) % 3;
			const int lastAxis = (nextAxis + 1) % 3;
			const float inverseRoot = 1.0f / std::sqrt(storedMatrix.m[axis][axis] - storedMatrix.m[nextAxis][nextAxis] - storedMatrix.m[lastAxis][lastAxis] + 1.0f);
			const float factor = 0.5f * inverseRoot;
			quaternion[axis] = 0.5f / inverseRoot;
			quaternion[nextAxis] = (storedMatrix.m[axis][nextAxis] + storedMatrix.m[nextAxis][axis]) * factor;
			quaternion[lastAxis] = (storedMatrix.m[axis][lastAxis] + storedMatrix.m[lastAxis][axis]) * factor;
			quaternion[3] = (storedMatrix.m[nextAxis][lastAxis] - storedMatrix.m[lastAxis][nextAxis]) * factor;
		}

		const DirectX::XMVECTOR extractedRotation = DirectX::XMVectorSet(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
		DirectX::XMFLOAT4 storedRotation;
		DirectX::XMStoreFloat4(&storedRotation, DirectX::XMQuaternionNormalize(extractedRotation));

		return {storedRotation.x, storedRotation.y, storedRotation.z, storedRotation.w};
	}

	DirectX::XMMATRIX LoadMatrix(const FMatrix4& matrix)
	{
		DirectX::XMFLOAT4X4 storedMatrix;
		for (int row = 0; row < 4; ++row)
		{
			for (int column = 0; column < 4; ++column)
			{
				storedMatrix.m[row][column] = matrix.m[row][column];
			}
		}

		return DirectX::XMLoadFloat4x4(&storedMatrix);
	}

	DirectX::XMMATRIX InverseOrIdentity(const DirectX::XMMATRIX& matrix)
	{
		const DirectX::XMVECTOR tolerance = DirectX::XMVectorReplicate(1.0e-8f);
		const DirectX::XMVECTOR zero = DirectX::XMVectorZero();
		if (DirectX::XMVector3NearEqual(matrix.r[0], zero, tolerance) && DirectX::XMVector3NearEqual(matrix.r[1], zero, tolerance) && DirectX::XMVector3NearEqual(matrix.r[2], zero, tolerance))
		{
			return DirectX::XMMatrixIdentity();
		}

		DirectX::XMVECTOR determinant;
		const DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&determinant, matrix);
		const float determinantValue = DirectX::XMVectorGetX(determinant);
		// DirectXMath가 반환한 특이 행렬의 결과 대신 Unreal처럼 항등을 사용한다.
		if (determinantValue == 0.0f || !std::isfinite(determinantValue))
		{
			return DirectX::XMMatrixIdentity();
		}

		return inverse;
	}

	FTransform ConstructTransformFromMatrixWithDesiredScale(DirectX::XMMATRIX matrix, const FVector3& inDesiredScale)
	{
		FTransform result;
		result.scale = inDesiredScale;
		DirectX::XMFLOAT3 storedPosition;
		DirectX::XMStoreFloat3(&storedPosition, matrix.r[3]);
		result.position = {storedPosition.x, storedPosition.y, storedPosition.z};

		// Unreal의 RemoveScaling / IsNearlyZero에 대응하는 float 축 복원 기준이다.
		constexpr float scaleSquaredTolerance = 1.0e-8f;
		constexpr float axisZeroTolerance = 1.0e-4f;
		const float desiredScale[3] = {result.scale.x, result.scale.y, result.scale.z};
		bool bHasMissingAxis = false;
		for (int axis = 0; axis < 3; ++axis)
		{
			const float lengthSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(matrix.r[axis]));
			if (lengthSquared >= scaleSquaredTolerance)
			{
				matrix.r[axis] = DirectX::XMVector3Normalize(matrix.r[axis]);
			}
			const float sign = desiredScale[axis] < 0.0f ? -1.0f : 1.0f;
			matrix.r[axis] = DirectX::XMVectorScale(matrix.r[axis], sign);
			const DirectX::XMVECTOR absoluteAxis = DirectX::XMVectorAbs(matrix.r[axis]);
			bHasMissingAxis = bHasMissingAxis || DirectX::XMVector3LessOrEqual(absoluteAxis, DirectX::XMVectorReplicate(axisZeroTolerance));
		}

		if (!bHasMissingAxis)
		{
			result.rotation = RotationFromMatrix(matrix);
		}

		return result;
	}

	FTransform MultiplyUsingMatrixWithScale(const FTransform& first, const FTransform& second)
	{
		const DirectX::XMMATRIX firstMatrix = LoadMatrix(first.ToMatrixWithScale());
		const DirectX::XMMATRIX secondMatrix = LoadMatrix(second.ToMatrixWithScale());
		const DirectX::XMMATRIX combinedMatrix = DirectX::XMMatrixMultiply(firstMatrix, secondMatrix);
		const FVector3 desiredScale = {first.scale.x * second.scale.x, first.scale.y * second.scale.y, first.scale.z * second.scale.z};

		return ConstructTransformFromMatrixWithDesiredScale(combinedMatrix, desiredScale);
	}

	FTransform GetRelativeTransformUsingMatrixWithScale(const FTransform& base, const FTransform& relative)
	{
		const DirectX::XMMATRIX baseMatrix = LoadMatrix(base.ToMatrixWithScale());
		const DirectX::XMMATRIX relativeMatrix = LoadMatrix(relative.ToMatrixWithScale());
		const DirectX::XMMATRIX inverseRelativeMatrix = InverseOrIdentity(relativeMatrix);
		const DirectX::XMMATRIX combinedMatrix = DirectX::XMMatrixMultiply(baseMatrix, inverseRelativeMatrix);
		const FVector3 reciprocal = GetSafeScaleReciprocal(relative.scale);
		const FVector3 desiredScale = {base.scale.x * reciprocal.x, base.scale.y * reciprocal.y, base.scale.z * reciprocal.z};

		return ConstructTransformFromMatrixWithDesiredScale(combinedMatrix, desiredScale);
	}
}

FTransform FTransform::operator*(const FTransform& other) const
{
	if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f || other.scale.x < 0.0f || other.scale.y < 0.0f || other.scale.z < 0.0f)
	{
		return MultiplyUsingMatrixWithScale(*this, other);
	}

	const DirectX::XMVECTOR firstRotation = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	const DirectX::XMVECTOR secondRotation = DirectX::XMVectorSet(other.rotation.x, other.rotation.y, other.rotation.z, other.rotation.w);
	const DirectX::XMVECTOR firstScale = DirectX::XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
	const DirectX::XMVECTOR secondScale = DirectX::XMVectorSet(other.scale.x, other.scale.y, other.scale.z, 0.0f);
	const DirectX::XMVECTOR firstPosition = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	const DirectX::XMVECTOR secondPosition = DirectX::XMVectorSet(other.position.x, other.position.y, other.position.z, 0.0f);

	// DirectXMath는 인수 순서와 반대로 secondRotation * firstRotation을 반환한다.
	const DirectX::XMVECTOR combinedRotation = DirectX::XMQuaternionMultiply(firstRotation, secondRotation);
	const DirectX::XMVECTOR combinedScale = DirectX::XMVectorMultiply(firstScale, secondScale);
	const DirectX::XMVECTOR scaledPosition = DirectX::XMVectorMultiply(firstPosition, secondScale);
	const DirectX::XMVECTOR rotatedPosition = DirectX::XMVector3Rotate(scaledPosition, secondRotation);
	const DirectX::XMVECTOR combinedPosition = DirectX::XMVectorAdd(rotatedPosition, secondPosition);

	DirectX::XMFLOAT4 storedRotation;
	DirectX::XMFLOAT3 storedScale;
	DirectX::XMFLOAT3 storedPosition;
	DirectX::XMStoreFloat4(&storedRotation, combinedRotation);
	DirectX::XMStoreFloat3(&storedScale, combinedScale);
	DirectX::XMStoreFloat3(&storedPosition, combinedPosition);

	FTransform result;
	result.rotation = {storedRotation.x, storedRotation.y, storedRotation.z, storedRotation.w};
	result.scale = {storedScale.x, storedScale.y, storedScale.z};
	result.position = {storedPosition.x, storedPosition.y, storedPosition.z};

	return result;
}

bool FTransform::IsRotationNormalized() const
{
	return rotation.IsNormalized();
}

FTransform FTransform::GetRelativeTransform(const FTransform& other) const
{
	if (!other.IsRotationNormalized())
	{
		return FTransform{};
	}

	if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f || other.scale.x < 0.0f || other.scale.y < 0.0f || other.scale.z < 0.0f)
	{
		return GetRelativeTransformUsingMatrixWithScale(*this, other);
	}

	const FVector3 reciprocal = GetSafeScaleReciprocal(other.scale);
	const DirectX::XMVECTOR safeReciprocalScale = DirectX::XMVectorSet(reciprocal.x, reciprocal.y, reciprocal.z, 0.0f);
	const DirectX::XMVECTOR currentScale = DirectX::XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
	const DirectX::XMVECTOR currentRotation = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	const DirectX::XMVECTOR otherRotation = DirectX::XMVectorSet(other.rotation.x, other.rotation.y, other.rotation.z, other.rotation.w);
	const DirectX::XMVECTOR currentPosition = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	const DirectX::XMVECTOR otherPosition = DirectX::XMVectorSet(other.position.x, other.position.y, other.position.z, 0.0f);

	// 단위 회전의 켤레는 역회전이 된다.
	const DirectX::XMVECTOR inverseOtherRotation = DirectX::XMQuaternionConjugate(otherRotation);
	const DirectX::XMVECTOR relativeScale = DirectX::XMVectorMultiply(currentScale, safeReciprocalScale);
	const DirectX::XMVECTOR relativeRotation = DirectX::XMQuaternionMultiply(currentRotation, inverseOtherRotation);
	const DirectX::XMVECTOR positionDifference = DirectX::XMVectorSubtract(currentPosition, otherPosition);
	const DirectX::XMVECTOR unrotatedPosition = DirectX::XMVector3Rotate(positionDifference, inverseOtherRotation);
	const DirectX::XMVECTOR relativePosition = DirectX::XMVectorMultiply(unrotatedPosition, safeReciprocalScale);

	DirectX::XMFLOAT3 storedScale;
	DirectX::XMFLOAT4 storedRotation;
	DirectX::XMFLOAT3 storedPosition;
	DirectX::XMStoreFloat3(&storedScale, relativeScale);
	DirectX::XMStoreFloat4(&storedRotation, relativeRotation);
	DirectX::XMStoreFloat3(&storedPosition, relativePosition);

	FTransform result;
	result.scale = {storedScale.x, storedScale.y, storedScale.z};
	result.rotation = {storedRotation.x, storedRotation.y, storedRotation.z, storedRotation.w};
	result.position = {storedPosition.x, storedPosition.y, storedPosition.z};

	return result;
}

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
