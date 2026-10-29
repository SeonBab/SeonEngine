#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Vector2.h"

class UTexture;

/** UI 텍스처의 필터와 주소 처리를 나타낸다. 실제 GPU 샘플러는 백엔드가 선택한다. */
enum class EUISampling : uint8
{
	PointClamp,
	LinearClamp
};

/** 화면 UI 꼭짓점 하나다. 출력 크기에 따른 클립 좌표 변환은 GPU 제출 경로에서 수행한다. */
struct FUIVertex
{
	// 현재 출력 영역 왼쪽 위를 원점으로 하는 픽셀 좌표다. X는 오른쪽, Y는 아래쪽으로 증가한다.
	FVector2 position{};
	// 텍스처의 정규화된 UV 좌표다. 전체 이미지는 왼쪽 위 (0, 0)부터 오른쪽 아래 (1, 1)을 사용한다.
	FVector2 uv{};
	// 각 성분은 0~1의 Straight RGBA다. RGB에 알파를 미리 곱하지 않으며 기본값은 불투명 흰색이다.
	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

/** 같은 텍스처와 렌더링 조건으로 연속된 인덱스 범위를 제출하기 위한 공통 정보다. */
struct FUIRenderBatch
{
	// 비소유 참조다. 수집부터 제출 완료까지 자산과 렌더 데이터를 유지하고 교체하지 않아야 한다.
	// nullptr는 미지정 상태이며 실제로 제출할 비어 있지 않은 배치에는 유효한 텍스처가 필요하다.
	const UTexture* texture = nullptr;
	// FUIBatchData::indices의 시작 원소 번호다. 바이트 오프셋이 아니다.
	uint32 firstIndex = 0;
	// 삼각형 목록에 사용할 인덱스 개수다. 제출 범위는 indices 배열 안에 있어야 한다.
	uint32 indexCount = 0;
	EUISampling sampling = EUISampling::PointClamp;
};

/**
 * 배처가 생성할 CPU 정점, 인덱스, 배치의 보관 공간이다. GPU 자원은 소유하지 않는다.
 * 참조 자산은 동기식 제출 완료까지 유지한다. 이 데이터 자체는 값과 자원의 유효성을 검사하지 않는다.
 */
struct FUIBatchData
{
	TArray<FUIVertex> vertices;
	// 각 값은 vertices 전체의 절대 인덱스다. 배치별 정점 시작 오프셋을 더하지 않는다.
	TArray<uint32> indices;
	// 그리기 순서대로 보관한다. 호환 조건이 같더라도 떨어진 범위를 재배열하지 않는다.
	TArray<FUIRenderBatch> renderBatches;
};
