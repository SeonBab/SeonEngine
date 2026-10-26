#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/SeonString.h"

class AActor;
class UWorld;

// 생성 함수가 반환하는 액터는 전달받은 World가 소유해야 한다.
using FActorFactory = AActor& (*)(UWorld&);

////////////////////////////////////////////////////////////////////////////////
// 액터 종류 이름과 생성 함수를 보관한다. 등록과 조회는 함수를 실행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FActorTypeRegistry final
{
public:
	/** 기본 "Actor" 생성 함수를 등록한다. 액터는 생성하지 않으며 이미 등록된 이름은 변경 없이 false를 반환한다. */
	[[nodiscard]] bool RegisterBuiltInTypes();

	/** 이름을 복사해 등록한다. 빈 이름, null 함수, 중복 이름은 로그와 false로 거부하고 기존 등록을 유지한다. */
	[[nodiscard]] bool Register(FStringView typeName, FActorFactory factory);

	/** 대소문자를 구분해 생성 함수 주소를 반환한다. 미등록 이름은 nullptr이며 액터를 생성하지 않는다. */
	[[nodiscard]] FActorFactory Find(FStringView typeName) const;

private:
	struct FEntry
	{
		FString typeName;
		FActorFactory factory = nullptr;
	};

	TArray<FEntry> entries;
};
