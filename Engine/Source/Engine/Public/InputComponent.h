#pragma once

#include "ActorComponent.h"
#include "Containers/HashMap.h"
#include "InputCoreTypes.h"

class FActor;

////////////////////////////////////////////////////////////////////////////////
// 액터에 부착하는 게임 입력 담당이다. 현재는 소속과 키 눌림 상태 조회를 제공한다.
// 입력 수신·매핑·액션·동작 바인딩은 후속이며 화면 상태나 동작 대상과 아직 연결되지 않았다.
// Unreal의 UInputComponent는 바인딩을 보관하고 PlayerInput이 처리한다.
// 자체 입력 관리 경계는 동일한 전체 책임이 아니며 UObject·컨트롤러 기반을 요구하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FInputComponent : public FActorComponent
{
public:
	/** 소속만 지정한다. 액터 부착이나 입력 등록·활성화·프레임 처리를 수행하지 않는다. */
	explicit FInputComponent(FActor& ownerActor);

	/** 기반 포인터로 파괴할 수 있다. 현재 별도 입력 연결이나 등록을 보관하지 않는다. */
	~FInputComponent() override = default;

	/** 저장된 눌림 상태를 반환한다. 항목이 없으면 false이며 조회는 상태를 변경하지 않는다. */
	bool IsPressed(FKey inKey) const;

private:
	// 이 컴포넌트의 게임 입력 상태다. 아직 갱신 통로가 없어 생성 후 빈 상태를 유지한다.
	THashMap<FKey, bool> keyStateMap;
};
