#include "WindowsInput.h"

#include "Core/CoreTypes.h"

namespace
{
	struct FWindowsKeyMapping
	{
		uint32 virtualKey;
		EKey key;
	};

	// NOTE: 지원 키를 추가할 때 EKey와 이 대응표를 함께 갱신한다. enum의 정수 값은 Windows 키 코드가 아니다.
	// 고정 표를 순회하며 조회 비용이나 동적 등록 요구가 생기면 저장과 조회 방식을 재검토한다.
	constexpr FWindowsKeyMapping KeyMappings[] =
		{
			{'W', EKey::W},
			{'A', EKey::A},
			{'S', EKey::S},
			{'D', EKey::D},
			{VK_PRIOR, EKey::PageUp},
			{VK_NEXT, EKey::PageDown},
			{VK_OEM_3, EKey::Backtick},
			{VK_INSERT, EKey::Insert},
			{VK_DELETE, EKey::Delete},
			{VK_HOME, EKey::Home},
			{VK_END, EKey::End},
			{VK_BACK, EKey::Backspace},
	};
}

EKey TranslateWindowsKey(WPARAM virtualKey)
{
	for (const FWindowsKeyMapping& mapping : KeyMappings)
	{
		if (mapping.virtualKey == virtualKey)
		{
			return mapping.key;
		}
	}

	return EKey::Invalid;
}
