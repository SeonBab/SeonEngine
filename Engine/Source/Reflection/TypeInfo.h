#pragma once

#include "Core/Containers/SeonString.h"

#include <span>

class FPropertyInfo;

////////////////////////////////////////////////////////////////////////////////
// 클래스 이름과 직접 선언한 프로퍼티 목록, 부모 타입 하나를 연결한다. 객체를 생성하거나 소유하지 않는다.
// 이름, 목록과 부모 정보는 빌린다. 사용 중 원본을 변경하지 않으며 모든 조회보다 오래 유지해야 한다.
////////////////////////////////////////////////////////////////////////////////
class FTypeInfo final
{
#pragma region Lifecycle

public:
	/**
	 * 전달한 정보를 빌려 보관하며 프로퍼티 접근 함수를 실행하지 않는다.
	 * properties는 이 클래스에 직접 선언한 목록이다. baseType이 nullptr이면 부모가 없으며 부모 연결은 순환하지 않아야 한다.
	 */
	FTypeInfo(FStringView name, const FTypeInfo* baseType, std::span<const FPropertyInfo> properties);

#pragma endregion

#pragma region Information

public:
	FStringView GetName() const { return name; }
	const FTypeInfo* GetBaseType() const { return baseType; }

	/** 직접 선언한 목록만 빌려 반환한다. 부모 프로퍼티는 포함하지 않는다. */
	std::span<const FPropertyInfo> GetProperties() const { return properties; }

	/**
	 * 이름의 대소문자와 전체 길이를 구분해 현재 클래스부터 부모 순서로 찾는다. 없으면 nullptr다.
	 * 같은 이름이면 먼저 찾은 항목을 반환한다. 검색과 조회는 값 접근 함수를 실행하지 않는다.
	 * 반환 포인터는 빌린 프로퍼티 배열 안의 항목을 가리키며 배열의 수명을 연장하지 않는다.
	 */
	[[nodiscard]] const FPropertyInfo* FindProperty(FStringView propertyName) const;

private:
	FStringView name;
	const FTypeInfo* baseType;                 // 부모 정보는 비소유 참조다. nullptr는 계층의 끝을 뜻한다.
	std::span<const FPropertyInfo> properties; // 직접 선언한 프로퍼티 배열을 빌린다. 부모 목록을 복제하지 않는다.

#pragma endregion
};
