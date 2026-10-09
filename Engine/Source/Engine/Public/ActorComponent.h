#pragma once

class FActor;

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙는 기능 객체의 공통 기반이다.
// 소속 액터를 소유하지 않으며, 액터는 컴포넌트의 수명 동안 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class FActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성 후 소속을 변경하지 않는다. */
	explicit FActorComponent(FActor& owner);

	/** 기반 타입을 통한 파괴에서도 파생 컴포넌트의 소멸자를 실행한다. */
	virtual ~FActorComponent();

	/** 생성 시 지정한 소속 액터를 반환한다. 액터의 수명을 연장하지 않는다. */
	FActor& GetOwner() const;

	// TODO(seon): 리플렉션에서 실제 클래스의 중복 정책과 상속 설정을 조회할 수 있게 되면 이 함수의 구현을 메타데이터 조회로 이전한다.
	// 파생 타입의 정책을 이관하고 기존 결과와 같음을 검증한 뒤 정책용 override를 정리한다.
	/** 같은 실제 타입의 복수 추가 허용 정책이다. 기본은 false이며 파생 타입은 타입별 고정 정책으로 재정의한다. */
	virtual bool AllowsMultipleComponents() const;

private:
	FActor* const owner;
};
