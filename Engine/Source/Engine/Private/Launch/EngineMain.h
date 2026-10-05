#pragma once

class FGenericWindow;

/**
 * 플랫폼과 관계없는 엔진 진입 함수다.
 * 플랫폼 진입점이 소유한 창 객체를 빌린다. 초기화와 명시적 종료는 이 함수가 수행한다.
 * 창 객체는 이 함수가 반환할 때까지 살아 있어야 한다.
 *
 * @return 프로세스 종료 코드.
 */
int EngineMain(FGenericWindow& window);
