#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Renderer/StaticMeshData.h"

#include <filesystem>

////////////////////////////////////////////////////////////////////////////////
// 기본 색상과 색상 텍스처 경로를 소유한다. 기본값은 불투명한 흰색·빈 경로다.
// 로더가 모델 파일 기준 참조를 해석해 절대경로를 넣는다. 빈 경로는 텍스처 없음이며
// 이미지 디코딩이나 GPU 자원 생성은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FModelMaterialData
{
	float baseColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	std::filesystem::path baseColorTexturePath;
};

////////////////////////////////////////////////////////////////////////////////
// 모델 내부의 메시 배치와 부모 관계를 보관한다. Actor나 GPU 자원은 아니다.
// localTransform은 부모 기준 행 벡터 행렬이고 루트에서는 모델 기준이다.
// parentIndex는 FModelData::nodes의 번호, -1은 루트다. meshIndices는
// FModelData::meshes의 번호이며 비어 있을 수 있다. 계층 유효성은 자동 검사하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FModelNodeData
{
	/** FMatrix4의 영행렬 기본값 대신 변환 없음인 항등행렬로 시작한다. */
	FModelNodeData()
	{
		localTransform.SetIdentity();
	}

	FMatrix4 localTransform;
	int32 parentIndex = -1;
	TArray<uint32> meshIndices;
};

////////////////////////////////////////////////////////////////////////////////
// 출처 형식과 독립적인 모델 CPU 데이터를 소유한다. 기본값은 빈 모델이다.
// 각 메시 배열을 구분해 보관하며 이 데이터로 직접 GPU 자원을 준비할 수 있다.
// 외부 파서의 포인터·GPU 자원을 보관하지 않고 파일 읽기·저장·검증은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FModelData
{
	TArray<FStaticMeshData> meshes;
	TArray<FModelMaterialData> materials;
	TArray<FModelNodeData> nodes;
};
