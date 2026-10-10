#pragma once

#include "Containers/Array.h"
#include "Math/Matrix4.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 메시 하나의 로컬 정점 위치와 월드 변환을 소유하는 CPU 렌더 전달용 데이터다.
// 게임 객체 주소나 GPU 자원을 보관하지 않는다. 월드 수집 뒤 렌더러에 값으로 전달한다.
////////////////////////////////////////////////////////////////////////////////
struct FMeshRenderData
{
	/** 빈 정점 배열과 항등 월드 변환을 준비한다. */
	FMeshRenderData();

	TArray<FVector3> vertices;
	FMatrix4 localToWorld;
};
