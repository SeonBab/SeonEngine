#pragma once

#include "Templates/UniquePtr.h"

class UEngine;
class UGameInstance;

/**
 * 소속 엔진을 받아 미초기화 세션의 소유권을 반환하는 생성 함수 형식이다.
 * 생성 함수는 Init / InitializeStandalone / StartGameInstance를 호출하지 않는다.
 * 실패 시 빈 포인터를 반환할 수 있으며 실패 정책은 호출자가 정한다.
 */
using FGameInstanceFactory = TUniquePtr<UGameInstance> (*)(UEngine& inEngine);
