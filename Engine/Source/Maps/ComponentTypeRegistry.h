#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/SeonString.h"
#include "Core/Templates/UniquePtr.h"
#include "Reflection/TypeInfo.h"

#include <typeinfo>

class AActor;
class UActorComponent;

// 전달한 액터 소속의 등록 타입을 만들고 소유 포인터를 반환한다. 액터에 추가하는 일은 호출자가 맡는다.
using FComponentFactory = TUniquePtr<UActorComponent> (*)(AActor&);

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트 종류 이름과 생성 함수, 실제 C++ 타입 및 공유 리플렉션 정보를 연결한다. 등록과 조회는 객체를 생성하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FComponentTypeRegistry final
{
public:
	/** 기본 Transform/DirectionalLight/SkyLight/StaticMesh의 생성 함수·실제 타입·생성 정보를 순서대로 등록한다. 객체는 생성하지 않는다. 첫 실패에서 false이며 기존/앞선 성공 등록은 유지한다. */
	[[nodiscard]] bool RegisterBuiltInTypes();

	/**
	 * 이름을 복사하고 typeid 및 공유 FTypeInfo를 빌려 등록한다. 두 정보는 등록표와 모든 조회보다 오래 살아야 한다.
	 * 생성 함수는 등록한 실제 타입과 같은 GetTypeInfo를 제공하는 객체를 반환해야 한다. 등록 자체는 함수를 실행하지 않는다.
	 * 빈 이름, null 함수, 중복 이름은 로그와 false로 거부하며 기존 등록을 유지한다.
	 */
	[[nodiscard]] bool Register(FStringView typeName, FComponentFactory factory, const std::type_info& componentType, const FTypeInfo& typeInfo);

	/** 대소문자를 구분해 생성 함수 주소를 반환한다. 미등록 이름은 nullptr이며 함수를 실행하지 않는다. */
	[[nodiscard]] FComponentFactory Find(FStringView typeName) const;

	/** 등록한 실제 C++ 타입 및 GetTypeInfo의 주소가 정확히 같은지 비교한다. 미등록 이름은 false이며 파생 타입을 같은 타입으로 취급하지 않는다. */
	[[nodiscard]] bool IsType(FStringView typeName, const UActorComponent& component) const;

	/** 미등록 종류는 nullptr다. 프로퍼티 목록을 복제하지 않고 등록한 공유 클래스 정보를 빌려 반환한다. */
	[[nodiscard]] const FTypeInfo* FindTypeInfo(FStringView typeName) const;

private:
	struct FEntry
	{
		FString typeName;
		FComponentFactory factory = nullptr;
		const std::type_info* componentType = nullptr; // typeid가 제공하는 정적 타입 정보를 빌린다.
		const FTypeInfo* typeInfo = nullptr;           // 생성된 클래스 정보는 정적 수명이며 등록표가 소유하지 않는다.
	};

	TArray<FEntry> entries;
};
