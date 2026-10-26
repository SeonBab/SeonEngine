#pragma once

#include "GameInstance.h"

////////////////////////////////////////////////////////////////////////////////
// SampleGame의 초기 콘텐츠와 UI를 구성한다. World/Client의 소유와 프레임 반복은 엔진이 맡는다.
////////////////////////////////////////////////////////////////////////////////
class USampleGameInstance : public UGameInstance
{
public:
	/** 준비된 World와 Client에 샘플을 추가한다. 실패 시 호출자가 부분 생성 객체를 정리하며 인수는 저장하지 않는다. */
	[[nodiscard]] bool Initialize(UWorld& world, FGameClient& gameClient, IRenderer& renderer) override;
};
