#pragma once

#include "Containers/SeonString.h"
#include "CoreTypes.h"

#include <optional>
#include <string>
#include <string_view>

// 변환할 수 없는 글자(짝이 없는 서로게이트 등)를 만났을 때 할 일. 어느 쪽이 맞는지는 결과를 쓰는 쪽이 정한다
enum class EInvalidCharacter : uint8
{
	// U+FFFD로 바꾸고 계속한다. 사람이 읽을 글자에 쓴다
	Replace,
	// 실패(std::nullopt)를 돌려준다. 경로처럼 OS에 다시 넘길 값에 쓴다. 글자가 바뀌면 다른 파일을 가리키게 된다
	Fail,
};

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

	/**
	 * UTF-16 문자열을 UTF-8로 바꾼다. Windows API가 돌려준 글자를 엔진 문자열로 받을 때 쓴다.
	 * Windows는 파일 이름 등에 깨진 UTF-16(짝이 없는 서로게이트)을 허용하고, 이것은 UTF-8로 그대로 옮길 수 없다.
	 *
	 * @param invalidCharacter 깨진 글자를 만났을 때 할 일. 기본은 UTF8ToWide와 같이 U+FFFD로 바꾸고 계속한다
	 * @return 변환한 문자열. 빈 입력은 빈 문자열이다. 변환 함수가 실패하거나, Fail일 때 깨진 글자가 있으면 std::nullopt
	 */
	static std::optional<FString> WideToUTF8(std::wstring_view wide, EInvalidCharacter invalidCharacter = EInvalidCharacter::Replace);
};
