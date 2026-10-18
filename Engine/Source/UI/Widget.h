#pragma once

#include "Object.h"

class FRenderer;

/** 모든 UI 위젯의 공통 기반이다. */
class UWidget : public UObject
{
public:
	/** 호출 동안 Renderer를 빌려 그리기를 요청한다. 그릴 내용이 없는 기본 구현은 true를 반환한다. */
	[[nodiscard]] virtual bool Draw([[maybe_unused]] FRenderer& renderer)
	{
		return true;
	}
};
