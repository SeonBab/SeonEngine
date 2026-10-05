#pragma once

#include "Containers/SeonString.h"

// 운영체제마다 구현이 다른 작은 기능 모음의 목록이다. Unreal FGenericPlatformMisc에 해당한다.
// 모든 플랫폼이 제공해야 하는 함수를 여기에 선언하고 설명한다. 플랫폼 struct(FWindowsPlatformMisc 등)가 이것을 상속해
// 자기 구현을 다시 선언한다. 쓰는 쪽은 이 struct가 아니라 HAL/PlatformMisc.h의 FPlatformMisc를 쓴다.
struct FGenericPlatformMisc
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
