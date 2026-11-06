#pragma once

#include "Core/Matrix4x4.h"
#include "Core/Quaternion.h"
#include "Core/Vector3.h"

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
};
