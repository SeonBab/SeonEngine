#pragma once

#include "Core/Math/Matrix4.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

////////////////////////////////////////////////////////////////////////////////
// 정적 메시 Proxy를 만들 때 읽는 입력이다. 자산과 선택된 재질의 주소를 빌리고 행렬은 복사한다.
// 생성 이후의 자원 수명은 소유 측에서 유지한다. 입력 자체는 GPU 준비나 Scene 등록을 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FStaticMeshSceneProxyDesc
{
	/** 자산과 재질을 미지정하고 로컬에서 월드로의 변환을 항등행렬로 준비한다. */
	FStaticMeshSceneProxyDesc();

	/** 같은 스레드의 컴포넌트에서 자산·슬롯 0 재질 주소와 월드 행렬을 수집한다. 수명은 소유 측이 유지한다. */
	explicit FStaticMeshSceneProxyDesc(const UStaticMeshComponent& component);

	// nullptr는 자산 미지정이다. Proxy 사용 종료까지 자산과 렌더 데이터를 유지해야 한다.
	const UStaticMesh* StaticMesh = nullptr;

	// 선택이 끝난 현재 단일 그리기의 재질이다. nullptr면 공통 기본 재질을 사용한다.
	// Proxy 사용 종료까지 소유 측에서 유지하며 이 입력은 재질 슬롯 선택이나 소유권 취득을 하지 않는다.
	UMaterialInterface* Material = nullptr;

	// 행 벡터 규칙의 로컬에서 월드로의 변환 사본이다. 기본값은 항등행렬이다.
	FMatrix4 LocalToWorld;
};
