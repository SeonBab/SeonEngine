#pragma once

#include "ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Core/Templates/SharedPtr.h"

class UStaticMesh;
class UMaterialInterface;
class FStaticMeshSceneProxy;

////////////////////////////////////////////////////////////////////////////////
// 선택한 정적 메시 자산과 재질의 수명을 공유하고 렌더 상태 생성/변경/해제를 요청한다.
// 등록된 Proxy는 Scene이 소유한다. 컴포넌트는 현재 Proxy의 자원을 사용 종료까지 유지한다.
////////////////////////////////////////////////////////////////////////////////
class UStaticMeshComponent : public UActorComponent
{
public:
	/** 소속 액터와 빈 자산 설정을 준비한다. 생성만으로 렌더 상태를 만들지 않는다. */
	explicit UStaticMeshComponent(AActor& owner);

	/** 남은 Proxy를 Scene에서 제거한 뒤 자원 참조를 해제한다. Actor와 World는 그때까지 살아 있어야 한다. */
	~UStaticMeshComponent() override;

	/** 설정 참조가 바뀌면 true와 함께 dirty 표시한다. nullptr는 표시 해제이며 GPU 생성은 하지 않는다. */
	bool SetStaticMesh(TSharedPtr<UStaticMesh> newStaticMesh);

	/** 선택 자산을 비소유로 빌린다. 설정 교체/파괴 후 수명이 보장되지 않으며 삭제하지 않는다. */
	UStaticMesh* GetStaticMesh() const;

	/** 슬롯의 공유 참조를 변경하고 dirty 표시한다. 음수는 무시하고 nullptr는 기본 재질로 복귀한다. */
	void SetMaterial(int32 materialIndex, TSharedPtr<UMaterialInterface> newMaterial);

	/** 오버라이드, 자산 기본 순서로 조회한 비소유 주소다. 음수/미지정은 nullptr이며 기본 재질 대체는 Proxy가 한다. */
	UMaterialInterface* GetMaterial(int32 materialIndex) const;

	/** 부모 합성을 반영한 행 벡터 규칙의 로컬에서 월드로의 행렬 사본이다. 메시 정점은 변경하지 않는다. */
	[[nodiscard]] FMatrix4 GetLocalToWorld() const;

	/**
	 * 준비된 자산에서 Proxy를 만들어 Scene에 소유권을 넘긴다. nullptr 자산은 Proxy 없는 생성 상태다.
	 * 중복 생성/미준비 자산은 false다. GPU 생성은 하지 않으며 같은 스레드에서 호출한다.
	 */
	[[nodiscard]] bool CreateRenderState();

	/** dirty 설정으로 Proxy를 교체한다. 실패하면 기존 Proxy/자원 참조/dirty를 유지한다. 미생성은 false다. */
	[[nodiscard]] bool UpdateRenderState();

	/** 현재 Proxy의 변환만 갱신한다. 미생성/외부 등록 제거는 false이며 표시 해제 상태는 할 일 없이 true다. */
	[[nodiscard]] bool SendRenderTransform();

	/** Scene 소유 Proxy를 먼저 제거하고 현재 렌더 자원 참조를 놓는다. 반복 가능하며 설정 참조는 유지한다. */
	void DestroyRenderState();

	/** 논리적 렌더 상태 생성 여부다. 자산 미지정이면 true여도 등록된 Proxy가 없을 수 있다. */
	[[nodiscard]] bool IsRenderStateCreated() const;

	/** 생성된 렌더 상태만 dirty 표시한다. 자산의 기본 재질 변경을 반영할 때도 명시 호출한다. */
	void MarkRenderStateDirty();

	/** 아직 현재 렌더 상태에 반영하지 않은 설정 변경 표시다. */
	[[nodiscard]] bool IsRenderStateDirty() const;

private:
	TSharedPtr<UMaterialInterface> ResolveMaterial(int32 materialIndex) const;
	bool ApplyRenderState();

	// 사용자가 선택한 설정이다. 변경 직후 이전 Proxy의 자원 수명은 아래 별도 참조로 유지한다.
	TSharedPtr<UStaticMesh> staticMesh;
	TArray<TSharedPtr<UMaterialInterface>> overrideMaterials;

	// 현재 Proxy가 빌린 자원이다. Proxy 제거 후 해제하며 기본 재질의 정적 객체는 공유 소유하지 않는다.
	// 자산 수명 유지와 내부 RenderData 교체 안전은 별개다. 기존 Proxy 사용 중 데이터 교체/비움은 금지한다.
	TSharedPtr<UStaticMesh> renderStateStaticMesh;
	TSharedPtr<UMaterialInterface> renderStateMaterial;

	// Scene이 소유한다. 컴포넌트는 삭제하지 않고 같은 스레드에서 Scene을 통해 제거한다.
	FStaticMeshSceneProxy* sceneProxy = nullptr;
	bool bRenderStateCreated = false;
	bool bRenderStateDirty = false;
};
