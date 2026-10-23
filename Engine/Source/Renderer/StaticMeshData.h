#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Renderer/TextureAddressMode.h"
#include "Renderer/Vertex.h"

////////////////////////////////////////////////////////////////////////////////
// 메시의 CPU 정점, 인덱스 배열, 재질 번호와 색상 텍스처의 X축(U)과 Y축(V) 주소 모드를 보관한다.
// 기본 배열은 비어 있고 주소 모드는 Clamp다.
// 각 인덱스는 vertices의 요소 번호다. materialIndex는 함께 사용하는 재질 목록의 번호이며
// -1은 재질 미지정이다. 재질이나 GPU 자원의 소유 및 데이터 검증은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FStaticMeshData
{
	TArray<FVertex> vertices;
	TArray<uint32> indices;
	int32 materialIndex = -1;
	ETextureAddress AddressX = ETextureAddress::Clamp;
	ETextureAddress AddressY = ETextureAddress::Clamp;
};
