#pragma once

class FActorTypeRegistry;
class FComponentTypeRegistry;

/**
 * 게임 쪽에서 정확히 하나 구현하는 타입 등록 연결이다. 엔진 기본 타입 등록 후 호출한다.
 * 호출마다 새 등록표를 빌리며 게임 전용 이름과 생성 함수만 등록한다. 객체 생성과 맵 로딩은 하지 않는다.
 * 등록표 참조를 저장하지 않고, 생성 함수와 등록한 타입 정보는 로딩보다 오래 유지한다.
 * false면 앞선 등록은 남지만 호출자는 이번 등록표를 폐기한다. 클래스 자동 등록은 수행하지 않는다.
 */
[[nodiscard]] bool RegisterGameTypes(FActorTypeRegistry& actorTypes, FComponentTypeRegistry& componentTypes);
