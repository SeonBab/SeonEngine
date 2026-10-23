#pragma once

class FGameClient;
class FRenderer;
class UWorld;

/**
 * 엔진이 준비한 World의 초기 콘텐츠를 게임에서 구성한다. 실행 파일마다 게임 쪽 구현 하나를 제공한다.
 * World는 빌린 참조이며 게임은 main, Renderer, 실행 반복, 종료를 관리하지 않는다.
 * false이면 엔진은 반복을 시작하지 않고 World를 정리한다. 부분 구성을 자동 롤백하지 않는다.
 * 현재는 단일 실행 프로젝트에서 게임 구현을 연결하는 링크 계약이다.
 * NOTE: 모듈, 클래스 등록이 필요해지면 게임 구성의 연결 방식을 재검토한다.
 */
[[nodiscard]] bool ConfigureGameWorld(UWorld& world);

/**
 * 엔진이 준비한 Client의 초기 구성을 게임에서 수행한다. 실행 파일마다 게임 쪽 구현 하나를 제공한다.
 * Client는 UGameEngine, 초기화된 Renderer는 실행 루프가 소유하며 게임은 호출 동안 참조를 빌린다.
 * World 구성 성공 뒤 렌더 상태 준비 전에 한 번 호출한다.
 * false이면 반복을 시작하지 않고 Client와 World를 정리한다. 부분 구성을 자동 롤백하지 않는다.
 */
[[nodiscard]] bool ConfigureGameClient(FGameClient& gameClient, FRenderer& renderer);
