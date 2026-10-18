#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Renderer/Vertex.h"

////////////////////////////////////////////////////////////////////////////////
// 메시의 CPU 정점·인덱스 배열과 재질 번호 하나를 보관한다. 기본 배열은 비어 있다.
// 각 인덱스는 vertices의 요소 번호다. materialIndex는 함께 사용하는 재질 목록의 번호이며
// -1은 재질 미지정이다. 재질·GPU 자원 소유나 데이터 검증은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FStaticMeshData
{
	TArray<FVertex> vertices;
	TArray<uint32> indices;
	int32 materialIndex = -1;
};
