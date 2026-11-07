#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

/**
 * 정적 메시의 정점 하나다. 위치와 방향은 메시의 로컬 좌표(왼손, cm) 기준이다.
 * 필드 순서와 크기는 렌더러 입력 배치와 맞추므로 바꾸면 렌더러 쪽 배치도 함께 고친다.
 */
struct MeshVertex
{
	Vector3 position;
	// 텍스처 좌표다. 왼쪽 위가 (0, 0), 오른쪽 아래가 (1, 1)이다.
	Vector2 uv;
	// 면이 바라보는 단위 방향이다.
	Vector3 normal;
	// uv의 u가 커지는 단위 방향이다. 노멀맵 계산에 쓴다.
	Vector3 tangent;
};

static_assert(sizeof(MeshVertex) == 44, "MeshVertex는 float 11개로 빈틈없이 배치되어야 한다.");
