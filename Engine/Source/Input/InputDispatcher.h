#pragma once

#include <vector>

class InputComponent;
class InputState;

/**
 * 한 World의 InputComponent에 프레임 입력을 전달한다. 메인 스레드에서 사용한다.
 * InputState를 해석해 키와 실행 조건을 정하고, 추가된 순서대로 컴포넌트에 실행을 요청한다.
 * 컴포넌트를 소유하지 않으며, 추가된 컴포넌트는 소멸 전에 스스로 제거한다.
 */
class InputDispatcher
{
	// 생성과 소멸
public:
	InputDispatcher() = default;

	// 컴포넌트가 이 객체의 주소를 보관하므로 복사와 이동을 막는다.
	InputDispatcher(const InputDispatcher&) = delete;
	InputDispatcher& operator=(const InputDispatcher&) = delete;
	InputDispatcher(InputDispatcher&&) = delete;
	InputDispatcher& operator=(InputDispatcher&&) = delete;

	// 컴포넌트 관리
public:
	/**
	 * 전달 대상으로 추가한다. nullptr이거나 이미 추가한 대상은 무시한다.
	 * 대상은 소멸 전에 RemoveInputComponent로 제거되어야 한다. 전달 중에는 호출하지 않는다. 할당 실패 예외는 전파한다.
	 */
	void AddInputComponent(InputComponent* inputComponent);

	/** 전달 대상에서 제거한다. 추가하지 않은 대상은 무시한다. 전달 중에는 호출하지 않는다. */
	void RemoveInputComponent(InputComponent* inputComponent);

private:
	// 비소유이며 추가한 순서를 유지한다.
	std::vector<InputComponent*> inputComponents;

	// 전달
public:
	/**
	 * 이번 프레임 이벤트를 들어온 순서대로 Pressed 또는 Released로 전달한 뒤, 누르는 중인 키마다 Held로 전달한다.
	 * deltaTime는 초 단위다. 콜백 예외는 전파되어 남은 전달을 중단한다.
	 * 콜백에서 컴포넌트 추가와 제거, 바인딩 추가를 하지 않는다.
	 */
	void Dispatch(const InputState& inputState, float deltaTime) const;
};
