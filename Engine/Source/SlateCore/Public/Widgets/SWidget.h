#pragma once

#include "Input/Reply.h"

struct FKeyEvent;

////////////////////////////////////////////////////////////////////////////////
// 위젯의 최소 키 입력 수신 계약이다. 배치·그리기·포커스 기능은 포함하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class SWidget
{
public:
	virtual ~SWidget() = default;

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
