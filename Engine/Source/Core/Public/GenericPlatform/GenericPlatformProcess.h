#pragma once

#include "Containers/SeonString.h"

#include <optional>

// 실행 중인 프로그램(프로세스)에 대해 운영체제에 묻는 기능 모음의 목록이다. Unreal FGenericPlatformProcess에 해당한다.
// 모든 플랫폼이 제공해야 하는 함수를 여기에 선언하고 설명한다. 플랫폼 struct(FWindowsPlatformProcess 등)가 이것을 상속해
// 자기 구현을 다시 선언한다. 쓰는 쪽은 이 struct가 아니라 HAL/PlatformProcess.h의 FPlatformProcess를 쓴다.
struct FGenericPlatformProcess
{
	/**
	 * 현재 스레드를 지정한 시간 동안 쉬게 한다. 메시지를 처리하거나 메시지 도착으로 깨어나지는 않는다.
	 * 밀리초 미만은 버리며 0밀리초이면 다른 스레드에 실행 기회를 양보한다.
	 * 실제 재개 시점은 OS 스케줄링에 따라 달라진다.
	 * @param seconds 유한한 0 이상의 초. 밀리초 환산값은 Windows의 INFINITE 값보다 작아야 한다.
	 */
	static void Sleep(float seconds);

	/**
	 * 실행 파일이 있는 폴더. 구분자는 '/'이고 끝에 '/'가 붙는다. 예: "C:/Git/SE/SeonEngine/Binaries/x64/Debug/"
	 * 처음 부를 때 한 번 구해 두고 같은 값을 돌려준다.
	 *
	 * @return 경로를 구하지 못했거나 경로에 UTF-8로 바꿀 수 없는 글자가 있으면 std::nullopt
	 */
	static std::optional<FString> BaseDir();

	/**
	 * 실행 파일 이름에서 확장자를 뺀 것. 예: "SampleGame"
	 * 처음 부를 때 한 번 구해 두고 같은 값을 돌려준다.
	 *
	 * @return 경로를 구하지 못했거나, 경로에 UTF-8로 바꿀 수 없는 글자가 있거나, 이름이 비어 있으면 std::nullopt
	 */
	static std::optional<FString> ExecutableName();
};
