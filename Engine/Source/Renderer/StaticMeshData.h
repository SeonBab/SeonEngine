#pragma once

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Renderer/Vertex.h"

////////////////////////////////////////////////////////////////////////////////
// 메시의 CPU 정점·인덱스 배열을 소유한다. 기본값은 두 배열이 비어 있는 상태다.
// 각 인덱스는 vertices의 요소 번호다. GPU 자원 소유나 데이터 검증은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FStaticMeshData
{
	TArray<FVertex> vertices;
	TArray<uint32> indices;
};
