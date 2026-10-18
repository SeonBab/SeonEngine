#pragma once

#include "Object.h"

class AActor;

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙는 기능 객체의 공통 기반이다.
// 소속 액터를 소유하지 않으며 액터는 컴포넌트보다 오래 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class UActorComponent : public UObject
{
public:
	/** 소속 액터를 지정한다. 생성 후 소속을 변경하지 않는다. */
	explicit UActorComponent(AActor& owner);

	/** 기반 포인터를 통한 파괴에서도 파생 컴포넌트의 소멸자를 실행한다. */
	~UActorComponent() override;

	UActorComponent(const UActorComponent&) = delete;
	UActorComponent& operator=(const UActorComponent&) = delete;
	UActorComponent(UActorComponent&&) = delete;
	UActorComponent& operator=(UActorComponent&&) = delete;

	/** 소속 액터를 반환한다. 액터의 수명을 연장하지 않는다. */
	AActor& GetOwner() const { return owner; }

	/** 같은 실제 타입의 복수 추가 허용 정책이다. 기본은 false이며 파생 타입은 타입별 고정 정책으로 재정의한다. */
	virtual bool AllowsMultipleComponents() const;

private:
	AActor& owner;
};
