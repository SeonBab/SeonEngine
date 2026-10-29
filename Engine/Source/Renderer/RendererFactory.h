#pragma once

#include "Core/Templates/UniquePtr.h"

class IRenderer;

/**
 * 실행 구성을 제공하는 쪽에서 정확히 하나 정의하는 렌더러 생성 연결이다.
 * 호출마다 새 미초기화 객체의 단독 소유권을 반환하며 nullptr는 생성 실패다.
 * 생성 함수는 창 생성이나 GPU 초기화를 하지 않는다. 초기화와 정리는 Loop가 맡는다.
 * Loop는 같은 스레드에서 호출하고 반환 객체를 Shutdown한 뒤 창보다 먼저 파괴한다.
 */
[[nodiscard]] TUniquePtr<IRenderer> CreateRenderer();
