#include "Key.h"

namespace
{
	// 문자열 리터럴을 참조하므로 EKeys 정적 객체의 초기화 순서에 의존하지 않는다.
	constexpr FStringView SupportedKeyNames[] =
		{
			"W",
			"A",
			"S",
			"D",
			"PageUp",
			"PageDown",
			"Tilde",
			"Insert",
			"Delete",
			"Home",
			"End",
			"BackSpace",
			"Q",
			"E",
			"RightMouseButton",
	};
}

const FKey EKeys::Invalid;
const FKey EKeys::W("W");
const FKey EKeys::A("A");
const FKey EKeys::S("S");
const FKey EKeys::D("D");
const FKey EKeys::PageUp("PageUp");
const FKey EKeys::PageDown("PageDown");
const FKey EKeys::Tilde("Tilde");
const FKey EKeys::Insert("Insert");
const FKey EKeys::Delete("Delete");
const FKey EKeys::Home("Home");
const FKey EKeys::End("End");
const FKey EKeys::BackSpace("BackSpace");
const FKey EKeys::Q("Q");
const FKey EKeys::E("E");
const FKey EKeys::RightMouseButton("RightMouseButton");

FKey::FKey(FStringView name)
	: keyName(name)
{
}

bool FKey::IsValid() const
{
	for (FStringView name : SupportedKeyNames)
	{
		if (keyName == name) { return true; }
	}

	return false;
}

FString FKey::ToString() const
{
	return keyName;
}

bool FKey::operator==(const FKey& other) const
{
	return keyName == other.keyName;
}
