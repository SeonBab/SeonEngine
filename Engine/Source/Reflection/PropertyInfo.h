#pragma once

#include "Core/Containers/SeonString.h"
#include "Object.h"

#include <concepts>
#include <type_traits>
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// 프로퍼티 하나의 이름, 값 타입과 접근 함수를 보관한다. 객체나 실제 프로퍼티 값은 소유하지 않는다.
// 이름의 원본과 접근 함수는 이 정보의 모든 사용보다 오래 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class FPropertyInfo final
{
#pragma region Creation

public:
	/**
	 * 타입별 접근 함수를 공통 객체 접근 함수로 연결한다. 향후 생성 코드가 호출할 진입점이다.
	 * OwnerType은 public UObject 계열이고 ValueType은 const/volatile/배열이 아닌 복사 가능한 값 타입이어야 한다.
	 * 두 접근 함수는 null일 수 없다. 이름은 복사하지 않으며 문자열 리터럴처럼 충분히 오래 사는 저장소를 요구한다.
	 * 생성은 접근 함수를 실행하지 않는다. 값 검사나 변경 후 갱신은 연결한 함수 또는 호출자의 책임이다.
	 */
	template<typename OwnerType, typename ValueType, ValueType (*Getter)(const OwnerType&), void (*Setter)(OwnerType&, const ValueType&)>
		requires std::derived_from<OwnerType, UObject> && std::same_as<ValueType, std::remove_cv_t<ValueType>> && std::is_object_v<ValueType> && (!std::is_array_v<ValueType>) && std::is_copy_constructible_v<ValueType> && std::is_copy_assignable_v<ValueType>
	[[nodiscard]] static FPropertyInfo Make(FStringView name)
	{
		static_assert(Getter != nullptr && Setter != nullptr, "Property accessors must not be null.");

		return FPropertyInfo(name, typeid(ValueType),
			[](const UObject& object, void* output)
		{
			const OwnerType* typedObject = dynamic_cast<const OwnerType*>(&object);
			if (!typedObject) { return false; }

			*static_cast<ValueType*>(output) = Getter(*typedObject);

			return true;
		},
			[](UObject& object, const void* value)
		{
			OwnerType* typedObject = dynamic_cast<OwnerType*>(&object);
			if (!typedObject) { return false; }

			Setter(*typedObject, *static_cast<const ValueType*>(value));

			return true;
		});
	}

private:
	FPropertyInfo(FStringView name, const std::type_info& valueType, bool (*getValue)(const UObject&, void*), bool (*setValue)(UObject&, const void*))
		: name(name),
		  valueType(&valueType),
		  getValue(getValue),
		  setValue(setValue)
	{
	}

#pragma endregion

#pragma region Information

public:
	FStringView GetName() const { return name; }
	const std::type_info& GetValueType() const { return *valueType; }

private:
	FStringView name;                // 이름을 복사하지 않는다. 원본 문자열은 이 정보의 모든 사용보다 오래 살아 있어야 한다.
	const std::type_info* valueType; // typeid가 반환하는 정적 타입 정보를 빌린다.

#pragma endregion

#pragma region ValueAccess

public:
	/**
	 * 값 타입이 정확히 같고 대상이 선언 클래스 또는 그 파생 클래스이면 값을 복사한다.
	 * 타입이나 대상이 맞지 않으면 false를 반환하고 출력을 유지한다. 숫자 타입 간 변환은 하지 않는다.
	 */
	template<typename ValueType>
		requires(!std::is_const_v<ValueType>) && (!std::is_volatile_v<ValueType>)
	[[nodiscard]] bool TryGetValue(const UObject& object, ValueType& outValue) const
	{
		if (*valueType != typeid(ValueType)) { return false; }

		return getValue(object, &outValue);
	}

	/**
	 * 값 타입과 대상을 검사한 뒤 연결된 쓰기 함수를 호출한다. 불일치이면 false를 반환하고 대상을 유지한다.
	 * true는 함수 호출 완료이며 값의 유효성 검사나 Transform·렌더 상태 갱신을 보장하지 않는다.
	 */
	template<typename ValueType>
		requires(!std::is_volatile_v<ValueType>)
	[[nodiscard]] bool TrySetValue(UObject& object, const ValueType& value) const
	{
		if (*valueType != typeid(ValueType)) { return false; }

		return setValue(object, &value);
	}

private:
	// 값 타입 검사 후에만 내부 함수에 생 포인터를 전달한다. 선언 클래스 검사는 각 함수의 dynamic_cast가 맡는다.
	bool (*getValue)(const UObject&, void*);
	bool (*setValue)(UObject&, const void*);

#pragma endregion
};
