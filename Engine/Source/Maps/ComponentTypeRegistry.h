#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/SeonString.h"
#include "Core/Templates/UniquePtr.h"
#include "Properties/ComponentPropertyInfo.h"

#include <span>
#include <typeinfo>

class AActor;
class UActorComponent;

// 전달한 액터 소속의 등록 타입을 만들고 소유 포인터를 반환한다. 액터에 추가하는 일은 호출자가 맡는다.
using FComponentFactory = TUniquePtr<UActorComponent> (*)(AActor&);

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트 종류 이름과 생성 함수, 실제 C++ 타입 및 프로퍼티 목록을 보관한다. 등록과 조회는 객체를 생성하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FComponentTypeRegistry final
{
public:
	/** 기본 "Transform" 생성 함수와 타입 및 position, rotation, scale 프로퍼티를 등록한다. 객체는 생성하지 않으며 중복 이름은 기존 등록을 유지하고 false다. */
	[[nodiscard]] bool RegisterBuiltInTypes();

	/**
	 * 이름을 복사하고 typeid로 얻은 타입 정보를 빌려 등록한다. 생성 함수는 해당 타입을 반환해야 한다.
	 * 빈 이름, null 함수, 중복 이름은 로그와 false로 거부하며 기존 등록을 유지한다.
	 */
	[[nodiscard]] bool Register(FStringView typeName, FComponentFactory factory, const std::type_info& componentType);

	/** 대소문자를 구분해 생성 함수 주소를 반환한다. 미등록 이름은 nullptr이며 함수를 실행하지 않는다. */
	[[nodiscard]] FComponentFactory Find(FStringView typeName) const;

	/** 등록한 실제 C++ 타입과 정확히 같은지 비교한다. 미등록 이름은 false이며 파생 타입을 같은 타입으로 취급하지 않는다. */
	[[nodiscard]] bool IsType(FStringView typeName, const UActorComponent& component) const;

	/**
	 * 등록된 종류에 프로퍼티를 옮겨 보관한다. 이름은 대소문자를 구분한다.
	 * 미등록 종류, 빈 이름, 중복 이름과 두 함수가 모두 null인 항목은 로그와 false로 거부한다.
	 * 실패 시 기존 등록을 유지하며 접근 함수를 실행하지 않는다.
	 */
	[[nodiscard]] bool RegisterProperty(FStringView typeName, FComponentPropertyInfo property);

	/**
	 * 등록표가 소유한 읽기 전용 목록을 반환한다. 미등록 종류나 프로퍼티가 없는 종류는 빈 목록이다.
	 * 등록 추가 또는 등록표 이동과 파괴 이후에는 다시 조회해야 한다. 접근 함수는 실행하지 않는다.
	 */
	[[nodiscard]] std::span<const FComponentPropertyInfo> GetProperties(FStringView typeName) const;

private:
	struct FEntry
	{
		FString typeName;
		FComponentFactory factory = nullptr;
		const std::type_info* componentType = nullptr; // typeid가 제공하는 정적 타입 정보를 빌린다.
		TArray<FComponentPropertyInfo> properties;
	};

	TArray<FEntry> entries;
};
