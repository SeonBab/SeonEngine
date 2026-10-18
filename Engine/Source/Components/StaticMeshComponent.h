#pragma once

#include "ActorComponent.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Image/Image.h"
#include "Renderer/StaticMeshData.h"

#include <optional>

class FRenderer;

////////////////////////////////////////////////////////////////////////////////
// 액터가 사용할 정적 메시와 선택적 이미지의 CPU 데이터를 복사해 소유한다.
// 명시적인 렌더 상태 생성·해제로 소속 World의 Scene 등록을 관리한다.
////////////////////////////////////////////////////////////////////////////////
class UStaticMeshComponent : public UActorComponent
{
public:
	/** 소속 액터와 빈 메시 데이터를 준비한다. 생성만으로 액터에 등록하지 않는다. */
	explicit UStaticMeshComponent(AActor& owner);

	/** 남은 Scene 등록을 해제한다. 소속 Actor와 World는 정리가 끝날 때까지 살아 있어야 한다. */
	~UStaticMeshComponent() override;

	/** 입력을 복사하고 생성된 렌더 상태를 변경 표시한다. 빈 데이터도 허용하며 GPU 갱신이나 유효성 검사는 하지 않는다. */
	void SetMeshData(const FStaticMeshData& newMeshData);

	/** 이미지를 복사하고 생성된 렌더 상태를 변경 표시한다. 유효성 검사와 GPU 생성은 렌더 상태 반영 시 수행한다. */
	void SetBaseColorImage(const FImage& newBaseColorImage);

	/** 저장한 데이터를 읽기 전용으로 빌린다. 컴포넌트 파괴 후 무효이며 setter 호출 뒤 배열 원소 참조는 다시 얻어야 한다. */
	const FStaticMeshData& GetMeshData() const;

	/** 소속 액터의 기본 Transform에서 부모 합성이 반영된 로컬→월드 행렬을 값으로 반환한다. 메시 정점은 변경하지 않는다. */
	[[nodiscard]] FMatrix4 GetLocalToWorld() const;

	/** 현재 CPU 메시·선택한 이미지와 월드 행렬을 소속 World의 Scene에 등록한다. 중복 생성·입력/자원 생성 실패는 false이며 기존 상태를 유지한다. */
	[[nodiscard]] bool CreateRenderState(FRenderer& renderer);

	/** 변경 표시된 메시와 이미지를 기존 Scene 등록에 반영한다. 미생성/갱신 실패는 false이며 실패 시 기존 GPU 메시와 표시를 유지한다. */
	[[nodiscard]] bool UpdateRenderState(FRenderer& renderer);

	/** 생성된 렌더 상태에 부모 합성 월드 행렬을 전달한다. GPU 메시/등록 ID는 유지하며 미생성 또는 Scene 갱신 실패는 false다. */
	[[nodiscard]] bool SendRenderTransform();

	/** 소유한 Scene 등록을 제거하고 상태를 초기화한다. Renderer를 사용하지 않으며 반복 호출할 수 있다. */
	void DestroyRenderState();

	/** 이 컴포넌트가 Scene 등록 ID를 보관하는지 조회한다. GPU 준비 상태나 외부에서의 등록 제거를 검사하지 않는다. */
	[[nodiscard]] bool IsRenderStateCreated() const;

	/** 생성된 렌더 상태에 메시·이미지 변경을 표시한다. GPU 갱신은 수행하지 않으며 미생성 상태에서는 무시한다. */
	void MarkRenderStateDirty();

	/** 렌더 상태에 반영하지 않은 메시·이미지 변경 표시를 조회한다. */
	[[nodiscard]] bool IsRenderStateDirty() const;

private:
	FStaticMeshData meshData;
	std::optional<FImage> baseColorImage;
	uint64 sceneMeshId = 0;
	bool bRenderStateDirty = false;
};
