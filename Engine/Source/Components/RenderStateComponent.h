#pragma once

#include "ActorComponent.h"
#include "RenderStateComponent.generated.h"

////////////////////////////////////////////////////////////////////////////////
// Scene에 반영할 렌더 상태의 생성, 갱신, 해제 계약을 제공하는 추상 컴포넌트다.
// 구체 렌더 데이터와 Proxy의 소유 방식은 파생 클래스가 결정한다.
////////////////////////////////////////////////////////////////////////////////
UCLASS()
class URenderStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 소속 액터를 기반 컴포넌트에 전달한다. 렌더 상태는 생성하지 않는다. */
	explicit URenderStateComponent(AActor& owner);

	/** 준비된 설정으로 렌더 상태를 생성한다. 성공은 true, 중복 생성이나 생성 실패는 false다. */
	[[nodiscard]] virtual bool CreateRenderState() = 0;

	/** 생성된 상태에 계산된 설정과 변환을 반영한다. 미생성이나 반영 실패는 false이며 전체 롤백은 보장하지 않는다. */
	[[nodiscard]] virtual bool SendRenderUpdates() = 0;

	/** Scene 등록을 해제하고 렌더 상태를 정리한다. 미생성 상태에서도 반복 호출할 수 있어야 한다. */
	virtual void DestroyRenderState() = 0;

	/** 논리적인 렌더 상태 생성 여부다. true여도 실제 Proxy나 GPU 자원이 존재한다는 보장은 없다. */
	[[nodiscard]] virtual bool IsRenderStateCreated() const = 0;
};
