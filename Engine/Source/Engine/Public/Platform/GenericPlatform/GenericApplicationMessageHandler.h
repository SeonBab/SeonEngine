#pragma once

#include "CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 플랫폼이 받은 입력을 상위 계층에 알리는 공개 공통 메시지 수신자다.
// 기본 구현은 이벤트를 처리하지 않는다. 수신자는 필요한 함수를 재정의한다.
////////////////////////////////////////////////////////////////////////////////
class FGenericApplicationMessageHandler
{
public:
	virtual ~FGenericApplicationMessageHandler() = default;

	/** 키 누름을 알린다. keyCode는 키 식별값, characterCode는 문자 정보, bIsRepeat는 반복 입력 여부다. 이벤트를 처리했으면 true다. */
	virtual bool OnKeyDown(int32 keyCode, uint32 characterCode, bool bIsRepeat);

	/** 키 뗌을 알린다. 인수와 처리 여부 반환은 OnKeyDown과 같은 계약이다. */
	virtual bool OnKeyUp(int32 keyCode, uint32 characterCode, bool bIsRepeat);
};
