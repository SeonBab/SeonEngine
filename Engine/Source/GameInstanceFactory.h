#pragma once

#include "Core/Templates/UniquePtr.h"

class UGameInstance;

/**
 * 게임 쪽에서 정확히 하나 구현하는 생성 연결이다. 엔진은 구체 게임 타입을 참조하지 않는다.
 * 미초기화 객체의 단독 소유권을 반환하며 nullptr는 생성 실패다. 초기화와 파괴는 UGameEngine이 맡는다.
 */
[[nodiscard]] TUniquePtr<UGameInstance> CreateGameInstance();
