#pragma once

#include "Containers/Array.h"
#include "Math/Matrix4.h"
#include "Renderer/PositionColorVertex.h"
#include "Renderer/PrimitiveType.h"

////////////////////////////////////////////////////////////////////////////////
// 메시 하나의 로컬 위치 / 색상 정점, 정점 연결 규칙과 월드 변환을 소유하는 CPU 렌더 전달용 데이터다.
// 게임 객체 주소나 GPU 자원을 보관하지 않는다. 월드 수집 뒤 렌더러에 값으로 전달한다.
////////////////////////////////////////////////////////////////////////////////
struct FMeshRenderData
{
	/** 빈 정점 배열, 항등 월드 변환과 Triangle List 규칙을 준비한다. */
	FMeshRenderData();

	TArray<FPositionColorVertex> vertices;
	FMatrix4 localToWorld;
	EPrimitiveType primitiveType = EPrimitiveType::PT_TriangleList;
};
