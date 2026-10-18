#include "Transform.h"

#include <DirectXMath.h>

#include <cmath>

namespace
{
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

	FTransform ConstructTransformFromMatrixWithDesiredScale(DirectX::XMMATRIX matrix, const FVector3& inDesiredScale)
	{
		FTransform result;
		result.scale = inDesiredScale;
		DirectX::XMFLOAT3 storedPosition;
		DirectX::XMStoreFloat3(&storedPosition, matrix.r[3]);
		result.position = {storedPosition.x, storedPosition.y, storedPosition.z};

		// Unreal의 RemoveScaling / IsNearlyZero에 대응하는 float 축 복원 기준이다.
		constexpr float ScaleSquaredTolerance = 1.0e-8f;
		constexpr float AxisZeroTolerance = 1.0e-4f;
		const float desiredScale[3] = {result.scale.x, result.scale.y, result.scale.z};
		bool bHasMissingAxis = false;
		for (int axis = 0; axis < 3; ++axis)
		{
			const float lengthSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(matrix.r[axis]));
			if (lengthSquared >= ScaleSquaredTolerance)
			{
				matrix.r[axis] = DirectX::XMVector3Normalize(matrix.r[axis]);
			}
			const float sign = desiredScale[axis] < 0.0f ? -1.0f : 1.0f;
			matrix.r[axis] = DirectX::XMVectorScale(matrix.r[axis], sign);
			const DirectX::XMVECTOR absoluteAxis = DirectX::XMVectorAbs(matrix.r[axis]);
			bHasMissingAxis = bHasMissingAxis || DirectX::XMVector3LessOrEqual(absoluteAxis, DirectX::XMVectorReplicate(AxisZeroTolerance));
		}

		if (!bHasMissingAxis)
		{
			result.rotation = RotationFromMatrix(matrix);
		}

		return result;
	}
}

FTransform FTransform::operator*(const FTransform& other) const
{
	if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f || other.scale.x < 0.0f || other.scale.y < 0.0f || other.scale.z < 0.0f)
	{
		const DirectX::XMMATRIX firstMatrix = LoadMatrix(ToMatrixWithScale());
		const DirectX::XMMATRIX secondMatrix = LoadMatrix(other.ToMatrixWithScale());
		const DirectX::XMMATRIX combinedMatrix = DirectX::XMMatrixMultiply(firstMatrix, secondMatrix);
		const FVector3 desiredScale = {scale.x * other.scale.x, scale.y * other.scale.y, scale.z * other.scale.z};

		return ConstructTransformFromMatrixWithDesiredScale(combinedMatrix, desiredScale);
	}

	const DirectX::XMVECTOR firstRotation = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	const DirectX::XMVECTOR secondRotation = DirectX::XMVectorSet(other.rotation.x, other.rotation.y, other.rotation.z, other.rotation.w);
	const DirectX::XMVECTOR firstScale = DirectX::XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
	const DirectX::XMVECTOR secondScale = DirectX::XMVectorSet(other.scale.x, other.scale.y, other.scale.z, 0.0f);
	const DirectX::XMVECTOR firstPosition = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	const DirectX::XMVECTOR secondPosition = DirectX::XMVectorSet(other.position.x, other.position.y, other.position.z, 0.0f);

	// NOTE: DirectXMath는 인수 순서와 반대로 secondRotation * firstRotation을 반환한다.
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

FTransform FTransform::GetRelativeTransform(const FTransform& other) const
{
	if (!other.IsRotationNormalized())
	{
		return FTransform{};
	}

	constexpr float ScaleTolerance = 1.0e-8f;
	const FVector3 reciprocal = {
		std::abs(other.scale.x) <= ScaleTolerance ? 0.0f : 1.0f / other.scale.x,
		std::abs(other.scale.y) <= ScaleTolerance ? 0.0f : 1.0f / other.scale.y,
		std::abs(other.scale.z) <= ScaleTolerance ? 0.0f : 1.0f / other.scale.z};
	if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f || other.scale.x < 0.0f || other.scale.y < 0.0f || other.scale.z < 0.0f)
	{
		const FMatrix4 relativeMatrix = ToMatrixWithScale() * other.ToMatrixWithScale().Inverse();
		const FVector3 desiredScale = {scale.x * reciprocal.x, scale.y * reciprocal.y, scale.z * reciprocal.z};

		return ConstructTransformFromMatrixWithDesiredScale(LoadMatrix(relativeMatrix), desiredScale);
	}

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

bool FTransform::IsRotationNormalized() const
{
	return rotation.IsNormalized();
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
