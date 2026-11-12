#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

/**
 * 위치, 회전, 크기를 묶은 값이다. 기본값은 위치 0, 회전 없음, 크기 1배다.
 * 부모 관계나 좌표 공간은 저장하지 않는다.
 */
struct Transform
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	/** 크기, 회전, 이동 순서로 적용하는 행렬을 계산해 반환한다. 행 벡터 규칙을 따른다. */
	Matrix4x4 ToMatrix() const;

	/**
	 * ToMatrix로 만든 모양의 행렬(크기, 회전, 이동의 곱)을 다시 위치, 회전, 크기로 나눈다.
	 * 기울임이 섞인 행렬은 넣지 않는다. 크기가 0인 축이 있으면 회전은 회전 없음으로 둔다.
	 * 거울이 섞인 행렬은 x 크기를 음수로 두고 나눈다.
	 */
	static Transform CreateFromMatrix(const Matrix4x4& matrix);
};
