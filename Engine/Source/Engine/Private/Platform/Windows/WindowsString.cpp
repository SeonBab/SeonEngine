#include "Platform/Windows/WindowsString.h"

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHWrapper.h"

#include <limits>

std::optional<std::wstring> FWindowsString::UTF8ToWide(FStringView utf8)
{
	// MultiByteToWideChar는 길이 0을 잘못된 인자로 보고 실패한다. 빈 입력은 실패가 아니므로 먼저 돌려준다
	if (utf8.empty())
	{
		return std::wstring();
	}

	// 길이를 int로 받는 API라, 그보다 길면 숫자가 잘려 엉뚱하게 변환되지 않게 실패로 처리한다
	if (utf8.size() > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		return std::nullopt;
	}
	const int32 utf8Length = static_cast<int32>(utf8.size());

	// 플래그 0: MB_ERR_INVALID_CHARS를 주지 않으면 깨진 바이트를 U+FFFD로 바꾸고 계속한다.
	// string_view는 끝에 '\0'이 있다는 보장이 없어 길이를 직접 넘긴다. 그래서 결과에도 '\0'이 붙지 않는다.
	// 첫 호출은 결과 버퍼 없이 필요한 칸 수만 묻는다
	const int32 wideLength = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, nullptr, 0);
	if (wideLength == 0)
	{
		return std::nullopt;
	}

	std::wstring wide(static_cast<size_t>(wideLength), L'\0');
	const int32 written = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, wide.data(), wideLength);
	if (written != wideLength)
	{
		return std::nullopt;
	}
	return wide;
}
