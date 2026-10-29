#pragma once

#include "Core/Containers/Array.h"
#include "Core/Math/Matrix4.h"
#include "Core/Math/Vector3.h"
#include "Core/Templates/SharedPtr.h"

class FStaticMeshRenderData;
class UTexture;

/** 월드→뷰, 뷰→클립 행렬과 월드 시점 위치·직교 여부의 값 사본이다. 행벡터 기준으로 두 행렬을 순서대로 곱한다. */
struct FRenderView
{
	FRenderView()
	{
		worldToView.SetIdentity();
		viewToClip.SetIdentity();
	}

	FMatrix4 worldToView;
	FMatrix4 viewToClip;
	FVector3 location{};
	// true이면 직교 투영의 일정한 시선 방향을 사용한다. 실제 투영 행렬과 일치시킨다.
	bool bOrthographic = false;
};

/** 메시 하나의 형상을 빌리고 변환, 정반사와 알파 마스킹 설정 사본과 색상 텍스처의 공유 참조를 보관한다. */
struct FMeshDrawItem
{
	FMeshDrawItem()
	{
		localToWorld.SetIdentity();
	}

	// 호출 동안만 빌린다. 원본 데이터의 교체와 파괴 전에 요청 처리를 끝내야 한다.
	const FStaticMeshRenderData* mesh = nullptr;
	FMatrix4 localToWorld;
	// 자산 수명을 유지하지만 내부 렌더 데이터의 교체까지 막지는 않는다. nullptr는 흰색 대체다.
	TSharedPtr<UTexture> baseColorTexture;
	// 그리기 요청 작성 시 조회한 선형 RGB 반사율 사본이다. 각 성분은 유한한 0~1이며 기본 검정은 기여 없음이다.
	FVector3 specularColor{};
	// 같은 시점의 Blinn-Phong 광택 지수 사본이다. 유한한 1 이상이며 기본값은 32다.
	float shininess = 32.0f;
	// 마스킹을 꺼도 기준값은 보관한다. 값은 같은 요청을 작성할 때 재질에서 복사한다.
	bool bAlphaMasked = false;
	// 유한한 0~1 값이다. 마스킹 시 텍스처와 정점색의 곱 알파가 이 값보다 낮으면 제거한다.
	float alphaCutoff = 0.5f;
};

/**
 * 한 시점의 장면 요청이다. 현재 DX11 구현은 메시 배열 순서대로 그린다.
 * 호출 동안 자원 교체·파괴나 재진입 변경을 하지 않는다. 요청 자체가 GPU 작업 완료까지 자원을 소유하는 것은 아니다.
 * 지연 구현은 반환 전에 CPU 값·주소 설정을 복사하고 필요한 GPU 자원 수명을 확보해야 한다.
 * 요청 배열이나 Proxy, 빌린 형상 주소를 반환 후 사용하려고 저장하면 안 된다.
 */
struct FSceneRenderData
{
	FRenderView view;
	// 빛이 진행하는 월드 단위 방향과 세기가 반영된 선형 RGB다. 미지정 광원의 색은 0이다.
	FVector3 lightDirection{0.0f, 0.0f, 1.0f};
	FVector3 lightColor{};
	// 균일 주변광의 세기가 반영된 선형 RGB다.
	FVector3 skyLightColor{};
	TArray<FMeshDrawItem> meshes;
};
