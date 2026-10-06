#pragma once

class FGenericWindow;
class IRenderer;

/**
 * 플랫폼과 관계없는 엔진 진입 함수다.
 * 진입점이 소유한 창과 렌더러 객체를 빌린다. 소유권을 가져오지 않는다.
 * 두 객체는 이 함수가 반환할 때까지 살아 있어야 한다.
 * 창 초기화 후 출력 가능한 첫 시점에 렌더러를 초기화하고 프레임을 출력한다.
 * 최소화 / 크기 0에서는 그래픽스 호출을 생략한다. 정상 종료는 렌더러 뒤 창 순서다.
 * 렌더러 초기화 실패 시 구현이 부분 자원을 정리하며 이 함수는 치명 종료한다.
 *
 * @return 프로세스 종료 코드.
 */
int EngineMain(FGenericWindow& window, IRenderer& renderer);
