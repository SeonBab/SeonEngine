#pragma once

#include "Core/Containers/SeonString.h"
#include "Properties/PropertyValue.h"

class UActorComponent;

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트 프로퍼티의 이름과 값 접근 함수를 보관한다. 객체를 소유하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FComponentPropertyInfo
{
	FString name;

	// null이면 읽기를 지원하지 않는다. 실패 시 false를 반환하고 출력 값을 유지한다.
	bool (*getValue)(const UActorComponent&, FPropertyValue&) = nullptr;

	// null이면 쓰기를 지원하지 않는다. 대상이나 값을 거부하면 false를 반환하고 컴포넌트를 유지한다.
	bool (*setValue)(UActorComponent&, const FPropertyValue&) = nullptr;
};
