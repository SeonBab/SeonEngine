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

private:
	FActor* const owner;
};
