#pragma once

#include "Mesh/MeshVertex.h"

#include <cstdint>
#include <vector>

/**
 * 정적 메시의 CPU 정점과 인덱스 배열이다. 인덱스 3개가 삼각형 하나이며,
 * 바깥에서 볼 때 시계 방향으로 감긴 면이 앞면이다. GPU 자원은 소유하지 않는다.
 */
struct StaticMeshData
{
	std::vector<MeshVertex> vertices;
	// 각 값은 vertices의 요소 번호다.
	std::vector<std::uint32_t> indices;
};
