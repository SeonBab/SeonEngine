#pragma once

#include "Input/Key.h"

#include <array>
#include <vector>

/**
 * 창에서 수집한 키 상태를 메인 스레드에서 보관한다.
 * 키별 누르는 중 여부와 이번 프레임에 들어온 순서대로의 키 이벤트를 함께 유지한다.
 */
class InputState
{
	// 프레임 갱신
public:
	/** 메시지 처리 전에 프레임당 한 번 호출한다. 이벤트 목록만 비우고 누르는 중 상태와 목록 메모리는 유지한다. */
	void ClearFrameEvents();

	// 키 상태
public:
	/**
	 * 누르는 중이 아니던 키만 누름으로 기록하고 이벤트를 추가한다.
	 * 자동 반복과 Unknown은 무시한다. 할당 실패 시 상태를 바꾸지 않고 예외를 전파한다.
	 */
	void PressKey(Key key);

	/**
	 * 누르는 중인 키만 뗌으로 기록하고 이벤트를 추가한다. 짝 없는 뗌 이벤트를 만들지 않는다.
	 * 할당 실패 시 상태를 바꾸지 않고 예외를 전파한다.
	 */
	void ReleaseKey(Key key);

	/**
	 * 포커스 상실 시 호출한다. 누르는 중인 키마다 Key 값 순서로 뗌 이벤트를 추가한다.
	 * 실제 뗌과 강제 뗌을 구분하지 않는다. 할당 실패 시 이미 처리한 키는 유지하고 예외를 전파한다.
	 */
	void ReleaseAllKeys();

	/** Unknown과 범위 밖 값은 false를 반환한다. */
	bool IsKeyDown(Key key) const;

	/**
	 * 이번 프레임에 들어온 순서대로의 이벤트를 빌려준다.
	 * 다음 ClearFrameEvents 또는 키 기록 전까지만 순회한다.
	 */
	const std::vector<KeyEvent>& GetFrameEvents() const;

private:
	// 인덱스는 Key 값이다.
	std::array<bool, KeyCount> downKeys = {};
	std::vector<KeyEvent> frameEvents;
};
