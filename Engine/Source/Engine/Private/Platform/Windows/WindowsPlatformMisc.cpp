#include "Windows/WindowsPlatformMisc.h"

#include "Platform/Windows/WindowsHWrapper.h"
#include "Platform/Windows/WindowsString.h"

bool FWindowsPlatformMisc::IsDebuggerPresent()
{
#if SE_BUILD_RELEASE
	return false;
#else
	// 앞의 ::는 이 함수가 아니라 같은 이름의 Windows API를 부른다는 뜻이다.
	// Windows의 BOOL은 int라서 bool로 바꿀 때 FALSE와 비교한다
	return ::IsDebuggerPresent() != FALSE;
#endif
}

void FWindowsPlatformMisc::LowLevelOutputDebugString(FStringView message)
{
	if (const std::optional<std::wstring> wideMessage = FWindowsString::UTF8ToWide(message))
	{
		::OutputDebugStringW(wideMessage->c_str());
	}
	else
	{
		// 변환 실패를 로그로 다시 보고하면 같은 경로를 반복해서 탈 수 있다. 변환이 필요 없는 고정 영어 문장을 직접 보낸다
		::OutputDebugStringA("[SeonEngine] LowLevelOutputDebugString: UTF-8 to UTF-16 conversion failed\n");
	}
}
