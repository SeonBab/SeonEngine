#pragma once

class FTypeInfo;

////////////////////////////////////////////////////////////////////////////////
// UObject 계열의 기반과 가상 타입 정보 조회를 제공한다. GC나 객체 생성·등록은 수행하지 않는다.
// 리플렉션에 참여하는 파생 클래스는 자기 타입 정보를 반환하도록 GetTypeInfo를 재정의한다.
////////////////////////////////////////////////////////////////////////////////
class UObject
{
#pragma region Lifecycle

public:
	virtual ~UObject() = default;

	/**
	 * 맵 로더가 이 객체의 프로퍼티 적용과 소유자 등록을 마친 뒤 호출한다. 기본 구현은 비어 있다.
	 * 같은 스레드의 플레이 시작 전 호출이며 다른 객체의 복원 완료를 보장하지 않는다.
	 * 파생 구현은 기반 함수를 호출하고 자기 파생 데이터를 갱신한다. 소유/계층 변경, 플레이 시작이나 렌더 자원 생성은 수행하지 않는다.
	 * 개별 TrySetValue는 호출하지 않는다. 재적용 시 다시 호출될 수 있으며 실패 복구/롤백 기능은 아니다.
	 */
	virtual void PostLoad();

#pragma endregion

#pragma region Reflection

public:
	/** UObject 자체의 공유 타입 정보를 반환한다. 부모와 직접 선언한 프로퍼티는 없으며 객체를 생성하지 않는다. */
	static const FTypeInfo& StaticTypeInfo();

	/**
	 * 기반 구현은 UObject의 정보를 반환한다. 참여 파생 클래스의 재정의가 해당 클래스 정보를 제공한다.
	 * 재정의하지 않은 클래스는 가장 가까운 기반 구현을 상속하며 실제 C++ 타입을 자동으로 발견하지 않는다.
	 * 반환한 정보는 빌린다. 파생 구현도 반환 정보와 그 참조 데이터가 모든 사용보다 오래 유지되도록 해야 한다.
	 */
	virtual const FTypeInfo& GetTypeInfo() const;

#pragma endregion
};
