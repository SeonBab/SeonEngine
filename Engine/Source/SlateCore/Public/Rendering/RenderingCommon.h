#pragma once

#include "Input/Reply.h"

struct FKeyEvent;

////////////////////////////////////////////////////////////////////////////////
// 위젯에서 화면 구현으로 키 입력을 전달하는 최소 계약이다.
////////////////////////////////////////////////////////////////////////////////
class ISlateViewport
{
public:
	virtual ~ISlateViewport() = default;

	/** 키 누름을 처리한다. 기본 구현은 미처리 결과를 반환한다. 이벤트 참조를 보관하지 않는다. */
	virtual FReply OnKeyDown(const FKeyEvent&)
	{
		return FReply::Unhandled();
	}

	/** 키 뗌을 처리한다. 기본 구현은 미처리 결과를 반환한다. 이벤트 참조를 보관하지 않는다. */
	virtual FReply OnKeyUp(const FKeyEvent&)
	{
		return FReply::Unhandled();
	}
};
