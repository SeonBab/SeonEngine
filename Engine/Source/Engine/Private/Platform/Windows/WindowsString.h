#pragma once

#include "Containers/SeonString.h"

#include <optional>
#include <string>

// Windows API 경계에서 쓰는 문자열 변환이다. 엔진 문자열은 UTF-8이고, 이름이 W로 끝나는 Windows API는 UTF-16(wchar_t)을 받는다.
// Windows 구현 파일끼리만 쓰는 도우미라 HAL(FPlatform...)이 아니다.
struct FWindowsString
{
	/**
	 * UTF-8 문자열을 UTF-16으로 바꾼다. 깨진 바이트는 U+FFFD로 바꾸고 나머지를 계속 변환한다.
	 *
	 * @return 변환한 문자열. 빈 입력은 빈 문자열이다. 변환 함수 자체가 실패하면 std::nullopt
	 */
	static std::optional<std::wstring> UTF8ToWide(FStringView utf8);
};
