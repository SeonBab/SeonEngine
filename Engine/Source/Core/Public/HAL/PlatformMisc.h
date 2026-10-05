#pragma once

#include "Containers/SeonString.h"

// 운영체제마다 구현이 다른 작은 기능 모음이다. Unreal FPlatformMisc에 해당한다.
// 여기에는 선언만 두고, 구현은 플랫폼 폴더의 .cpp(Platform/Windows/WindowsPlatformMisc.cpp)에 둔다.
// 쓰는 쪽은 플랫폼을 신경 쓰지 않고 FPlatformMisc::...만 부른다.
struct FPlatformMisc
{
	/**
	 * 디버거가 이 프로세스에 붙어 있는지 알려 준다. 멈추지는 않는다.
	 * 배포용 Release에서는 디버거에서 멈추는 동작을 쓰지 않으므로 항상 false다.
	 */
	static bool IsDebuggerPresent();

	/**
	 * 로그 시스템을 거치지 않고 디버거 출력 창(VS 출력 창 등)에 글자를 바로 보낸다. 디버거가 없으면 아무 일도 일어나지 않는다.
	 * 줄바꿈은 붙이지 않으므로 필요하면 message에 넣는다.
	 */
	static void LowLevelOutputDebugString(FStringView message);
};
