#pragma once

#include "Core/Containers/SeonString.h"

/** Game과 Editor가 시작할 맵의 논리 경로다. 타입 등록이나 World 생성은 수행하지 않는다. */
struct FGameMapsSettings
{
	FString editorStartupMap; // /Engine/... 또는 /Game/... JSON 맵 경로. 빈 값은 시작 맵 로딩 실패다.
	FString gameDefaultMap;   // Editor와 같은 맵을 지정할 수 있다. 게임 카메라·UI 설정은 포함하지 않는다.
};

/**
 * 게임 쪽에서 정확히 하나 구현하는 설정 연결이다. 엔진은 구체 게임 타입을 참조하지 않는다.
 * 반환 설정은 실행 중 변경하지 않는 정적 수명이며 호출자는 소유하거나 변경하지 않는다.
 * 함수는 World나 자산을 만들지 않는다. 경로 검사와 파일 로딩은 호출자가 담당한다.
 */
[[nodiscard]] const FGameMapsSettings& GetGameMapsSettings();
