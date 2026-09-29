#pragma once

/**
 * 플랫폼과 관계없는 엔진 진입 함수다.
 * 플랫폼 진입점(WinMain 등)이 플랫폼별 준비를 마친 뒤 호출한다.
 *
 * @return 프로세스 종료 코드.
 */
int EngineMain();
