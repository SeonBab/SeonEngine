#pragma once

#include "Input/InputMappingContext.h"
#include "Input/PlayerInput.h"
#include "UI/GameWidgetSubsystem.h"

/** 로컬 게임 실행에 필요한 하위 기능을 소유하고 연결한다. */
class FGameClient
{
public:
	/** 소유한 입력 관리자를 빌린다. 반환 참조는 Client가 살아 있는 동안만 유효하다. */
	UPlayerInput& GetPlayerInput() { return playerInput; }

	/** 소유한 키 매핑 Context를 빌린다. 반환 참조는 Client가 살아 있는 동안만 유효하다. */
	UInputMappingContext& GetInputMappingContext() { return inputMappingContext; }

	/** 소유한 위젯 Subsystem을 빌린다. 반환 참조는 Client가 살아 있는 동안만 유효하다. */
	FGameWidgetSubsystem& GetWidgetSubsystem() { return widgetSubsystem; }

private:
	UPlayerInput playerInput;
	UInputMappingContext inputMappingContext;
	FGameWidgetSubsystem widgetSubsystem;
};
