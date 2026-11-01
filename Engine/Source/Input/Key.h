#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 키 이름을 소유하는 값 타입이다. 이름의 대소문자를 구분하며 기본 이름은 비어 있다.
// 지원 키 판정은 고정 이름 목록을 사용하며 장치 수집과 눌림 상태 관리는 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FKey
{
public:
	FKey() = default;

	/** 빌린 이름을 복사한다. 호출 후 원본이 바뀌거나 사라져도 보관한 이름은 유지한다. */
	explicit FKey(FStringView name);

	/** 고정 지원 목록에 이름이 있는지 반환한다. 실제 장치 수집이나 게임 매핑 연결 여부는 뜻하지 않는다. */
	bool IsValid() const;

	/** 보관한 이름의 사본을 반환한다. 반환값을 수정해도 이 객체의 이름은 바뀌지 않는다. */
	FString ToString() const;

	/** 객체 주소가 아닌 이름을 대소문자를 구분해 비교한다. 빈 이름끼리도 같다. */
	bool operator==(const FKey& other) const;

private:
	FString keyName;
};

////////////////////////////////////////////////////////////////////////////////
// 엔진 키 상수 모음이다. 이름을 직접 적지 않고 FKey 값을 사용할 수 있게 한다.
// Invalid는 빈 이름이며 유효하지 않다. 상수 정의만으로 실제 입력을 수집하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct EKeys
{
	static const FKey Invalid;
	static const FKey W;
	static const FKey A;
	static const FKey S;
	static const FKey D;
	static const FKey PageUp;
	static const FKey PageDown;
	static const FKey Tilde;
	static const FKey Insert;
	static const FKey Delete;
	static const FKey Home;
	static const FKey End;
	static const FKey BackSpace;
	static const FKey Q;
	static const FKey E;
	static const FKey RightMouseButton;
};

/** 키 입력 사건의 종류다. 누르는 동안 매 프레임 실행하는 조건과는 구분한다. */
enum class EKeyEventType : uint8
{
	Pressed,
	Released,
	Repeat
};

/** 어떤 키에 어떤 입력 사건이 발생했는지 표현한다. 생성만으로 입력을 저장하거나 전달하지 않는다. */
struct FKeyEvent
{
	FKey key;
	EKeyEventType type = EKeyEventType::Pressed;
};
