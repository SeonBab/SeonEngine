#pragma once

#include "Templates/UniquePtr.h"

class IModuleInterface;

/** 게임 모듈 객체의 소유권을 반환한다. 준비 훅은 호출하지 않는다. */
[[nodiscard]] TUniquePtr<IModuleInterface> CreateGameModule();
