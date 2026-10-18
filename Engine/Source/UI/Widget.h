#pragma once

#include "Object.h"

class FUIDrawElementList;

/** 모든 UI 위젯의 공통 기반이다. */
class UWidget : public UObject
{
public:
	/** 목록에 그리기 요소를 추가한다. 기본 구현은 목록을 변경하지 않고 true를 반환한다. */
	[[nodiscard]] virtual bool CollectDrawElements([[maybe_unused]] FUIDrawElementList& elements)
	{
		return true;
	}
};
