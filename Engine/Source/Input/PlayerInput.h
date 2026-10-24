#pragma once

#include "Core/Containers/Array.h"
#include "Input/InputActionInstance.h"
#include "Object.h"

class FInputState;
class UInputComponent;
class UInputMappingContext;

////////////////////////////////////////////////////////////////////////////////
// 액션별 실행 인스턴스를 소유한다. Boolean/Axis1D 평가, 값 갱신, 조회와 컴포넌트에 개별 이벤트 전달을 제공한다.
////////////////////////////////////////////////////////////////////////////////
class UPlayerInput final : public UObject
{
#pragma region ActionState

public:
	/** 이벤트 전달 중이거나 빈 액션, 잘못된 값 형식, 같은 액션 객체의 중복 등록은 false이며 기존 목록을 유지한다. */
	[[nodiscard]] bool RegisterAction(TSharedPtr<const UInputAction> action);

	/**
	 * Context와 현재 키 상태로 액션 형식의 값을 계산해 등록된 인스턴스를 갱신한다. 입력/매핑은 변경하지 않는다.
	 * 이벤트 전달 중, 미등록, 지원하지 않는 형식/규칙, 계산 실패면 false와 기존 데이터 유지다. 매핑/눌린 키 없음은 정상 false/0이며 한 호출이 한 평가다.
	 */
	[[nodiscard]] bool EvaluateAction(const UInputAction& action, const UInputMappingContext& context, const FInputState& input);

	/**
	 * 등록 순서대로 모든 액션을 한 번씩 평가한다. 임시 인스턴스에서 전부 성공한 뒤 기존 인스턴스에 반영한다.
	 * 이벤트 전달 중이거나 하나라도 실패하면 false와 전체 기존 데이터 유지다. 빈 목록은 true이며 미매핑 액션은 정상 false/0으로 갱신한다.
	 * 조회 포인터는 유지하지만 빌린 값 참조의 내용은 바뀔 수 있다. 호출 동안 등록 목록, 정의, 입력, Context를 변경하지 않는다.
	 * 입력/Context 변경, 자동 등록, 이벤트 전달은 하지 않으며 호출 횟수를 프레임별로 제한하지 않는다.
	 */
	[[nodiscard]] bool EvaluateInputActions(const UInputMappingContext& context, const FInputState& input);

	/** 이벤트 전달 중, 미등록 액션, 형식 불일치, 비유한 값이면 false이며 기존 데이터는 유지한다. 한 호출이 한 평가이며 자동 등록이나 이벤트 전달은 없다. */
	[[nodiscard]] bool UpdateActionValue(const UInputAction& action, const FInputActionValue& newValue);

	/**
	 * 같은 액션 객체의 인스턴스를 빌린다. 미등록이면 nullptr다.
	 * 반환 포인터는 등록 성공, 관리자의 이동/대입, 파괴 후 다시 사용하지 않는다.
	 */
	const FInputActionInstance* FindActionInstanceData(const UInputAction& action) const;

private:
	TArray<FInputActionInstance> actionInstances;

#pragma endregion

#pragma region EventDispatch

public:
	/**
	 * 등록 순서로 저장된 전이 결과를 한 컴포넌트에 전달한다. StartedAndTriggered는 해당 액션의 Started 다음 Triggered로 보낸다.
	 * 평가/상태 변경/소비는 하지 않는다. 재호출하면 같은 결과를 다시 보내며 평가 실패 시 호출하지 않는 것은 호출자 책임이다.
	 * 전달 중 등록, 단일/일괄 평가, 직접 값 갱신은 false로 거부하고 중첩 전달은 무시한다. 조회는 허용한다.
	 * 콜백은 정상 반환해야 하며 관리자를 이동/대입하거나 관리자, 대상 컴포넌트, 소유 액터를 즉시 파괴하면 안 된다. 값 참조는 전달 중에만 사용한다.
	 */
	void DispatchActionEvents(UInputComponent& inputComponent, float deltaSeconds);

private:
	bool bDispatchingActionEvents = false;

#pragma endregion
};
