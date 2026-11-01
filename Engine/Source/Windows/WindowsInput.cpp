#include "WindowsInput.h"

#include "Core/CoreTypes.h"

namespace
{
	struct FWindowsKeyMapping
	{
		uint32 virtualKey;
		const FKey* key; // 프로그램 수명 동안 유지되는 EKeys 상수를 빌린다. 변환 결과는 값으로 복사한다.
	};

	// NOTE: 수집할 키를 추가할 때 EKeys의 지원 이름과 이 대응표를 함께 갱신한다. 키 이름은 Windows 키 코드가 아니다.
	// 고정 표를 순회하며 조회 비용이나 동적 등록 요구가 생기면 저장과 조회 방식을 재검토한다.
	constexpr FWindowsKeyMapping KeyMappings[] =
		{
			{'W', &EKeys::W},
			{'A', &EKeys::A},
			{'S', &EKeys::S},
			{'D', &EKeys::D},
			{'Q', &EKeys::Q},
			{'E', &EKeys::E},
			{VK_PRIOR, &EKeys::PageUp},
			{VK_NEXT, &EKeys::PageDown},
			{VK_OEM_3, &EKeys::Tilde},
			{VK_INSERT, &EKeys::Insert},
			{VK_DELETE, &EKeys::Delete},
			{VK_HOME, &EKeys::Home},
			{VK_END, &EKeys::End},
			{VK_BACK, &EKeys::BackSpace},
	};
}

FKey TranslateWindowsKey(WPARAM virtualKey)
{
	for (const FWindowsKeyMapping& mapping : KeyMappings)
	{
		if (mapping.virtualKey == virtualKey)
		{
			return *mapping.key;
		}
	}

	return EKeys::Invalid;
}
