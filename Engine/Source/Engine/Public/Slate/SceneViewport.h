#pragma once

#include "Containers/HashMap.h"
#include "InputCoreTypes.h"
#include "Rendering/RenderingCommon.h"

struct FKeyEvent;

////////////////////////////////////////////////////////////////////////////////
// 게임 화면의 입력 수신과 키 상태 관리를 담당한다.
////////////////////////////////////////////////////////////////////////////////
class FSceneViewport : public ISlateViewport
{
public:
	FSceneViewport() = default;

	/** 키의 눌림 상태를 반환한다. 저장된 상태가 없으면 false다. */
	bool KeyState(FKey key) const;

	/** 키 누름을 받아 상태를 갱신한다. 처리했으면 Handled, Invalid면 Unhandled다. */
	FReply OnKeyDown(const FKeyEvent& inKeyEvent) override;

	/** 키 뗌을 받아 상태를 해제한다. 처리했으면 Handled, Invalid면 Unhandled다. */
	FReply OnKeyUp(const FKeyEvent& inKeyEvent) override;

private:
	THashMap<FKey, bool> keyStateMap;
};
