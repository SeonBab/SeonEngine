#pragma once

#include "Containers/HashMap.h"
#include "Containers/SeonString.h"
#include "CoreTypes.h"
#include "Templates/SharedPointer.h"

////////////////////////////////////////////////////////////////////////////////
// 입력 키 하나를 이름으로 식별하는 값 타입이다. 눌림 상태를 보관하지 않는다.
// Unreal의 FKey 형태를 따르되 현재는 FName 대신 소유하는 FString을 사용한다.
////////////////////////////////////////////////////////////////////////////////
struct FKey
{
public:
	/** 이름이 비어 있는 키를 만든다. 등록 여부를 판단하지 않는다. */
	FKey() = default;

	/** 키 식별 이름을 복사해 보관한다. 이름은 대소문자를 구분한다. */
	explicit FKey(const FString& inName);

	/** 두 키의 식별 이름이 같으면 true다. */
	bool operator==(const FKey& other) const;

	/** 키 식별 이름의 해시를 반환한다. */
	friend uint32 GetTypeHash(const FKey& key);

private:
	FString keyName;
};

////////////////////////////////////////////////////////////////////////////////
// 공통 입력 키 상수를 제공한다. 객체를 생성하지 않고 EKeys::W처럼 사용한다.
////////////////////////////////////////////////////////////////////////////////
struct EKeys
{
	/** 이름이 비어 있는 키다. 등록 여부 / 유효성 판정은 아직 제공하지 않는다. */
	static const FKey Invalid;
	static const FKey W;
	static const FKey A;
	static const FKey S;
	static const FKey D;
};

////////////////////////////////////////////////////////////////////////////////
// 플랫폼 키 번호에서 공통 키를 찾기 위한 관리자 선언이다.
// 현재는 보관 계약만 준비하며 생성 / 초기화 / 조회 본문과 호출은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
struct FInputKeyManager
{
public:
	/** 하나의 관리자를 얻는 입구다. 현재는 선언만 준비한다. */
	static FInputKeyManager& Get();

	/** 플랫폼의 번호 / 이름 목록으로 매핑을 초기화한다. 현재는 선언만 준비한다. */
	void InitKeyMappings();

	/** 플랫폼 키 코드와 문자 코드로 공통 키를 조회한다. 본문과 문자 매핑은 후속 단계다. */
	FKey GetKeyFromCodes(uint32 keyCode, uint32 characterCode) const;

private:
	FInputKeyManager();

	/** 관리자를 공유 소유하는 정적 포인터다. 정의와 생성은 후속 단계다. */
	static TSharedPtr<FInputKeyManager> instance;

	/** 플랫폼 키 번호에서 FKey를 찾는 매핑이다. Enum은 Unreal의 기존 멤버 이름을 따른다. */
	THashMap<uint32, FKey> keyMapVirtualToEnum;
};
