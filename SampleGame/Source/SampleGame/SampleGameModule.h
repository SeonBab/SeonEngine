#pragma once

#include "Modules/ModuleInterface.h"

////////////////////////////////////////////////////////////////////////////////
// SampleGame 코드 준비를 담당한다. 게임 세션의 준비 / 시작과 구분한다.
////////////////////////////////////////////////////////////////////////////////
class FSampleGameModule : public IModuleInterface
{
public:
	/** 엔진 초기화 전에 세션 생성 함수 주소를 등록한다. 등록 실패는 Fatal이다. */
	void StartupModule() override;

	/** 세션 정리 뒤 생성 함수 주소를 해제한다. 실패는 Error로 기록하고 종료를 계속한다. */
	void ShutdownModule() override;
};
