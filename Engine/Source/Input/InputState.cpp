#include "Input/InputState.h"

#include <cstddef>

namespace
{
	bool IsRecordableKey(Key key)
	{
		return key != Key::Unknown && key < Key::Count;
	}

	std::size_t ToIndex(Key key)
	{
		return static_cast<std::size_t>(key);
	}
}

void InputState::ClearFrameEvents()
{
	// clear는 원소만 지우고 할당한 메모리는 유지하므로 다음 프레임에 재사용한다.
	frameEvents.clear();
}

void InputState::PressKey(Key key)
{
	if (!IsRecordableKey(key)) { return; }

	bool& isDown = downKeys[ToIndex(key)];

	// 이미 누르는 중이면 Windows 자동 반복 같은 중복 누름이므로 새 이벤트를 만들지 않는다.
	if (isDown) { return; }

	// 이벤트 추가가 실패하면 상태도 바꾸지 않도록 추가를 먼저 한다.
	frameEvents.push_back(KeyEvent{ key, KeyEventType::Pressed });
	isDown = true;
}

void InputState::ReleaseKey(Key key)
{
	if (!IsRecordableKey(key)) { return; }

	bool& isDown = downKeys[ToIndex(key)];
	if (!isDown) { return; }

	frameEvents.push_back(KeyEvent{ key, KeyEventType::Released });
	isDown = false;
}

void InputState::ReleaseAllKeys()
{
	for (std::size_t index = 0; index < KeyCount; ++index)
	{
		if (!downKeys[index]) { continue; }

		frameEvents.push_back(KeyEvent{ static_cast<Key>(index), KeyEventType::Released });
		downKeys[index] = false;
	}
}

bool InputState::IsKeyDown(Key key) const
{
	if (!IsRecordableKey(key)) { return false; }

	return downKeys[ToIndex(key)];
}

const std::vector<KeyEvent>& InputState::GetFrameEvents() const
{
	return frameEvents;
}
