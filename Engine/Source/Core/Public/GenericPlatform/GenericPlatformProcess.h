#pragma once

#include "Containers/SeonString.h"

#include <optional>

// 실행 중인 프로그램(프로세스)에 대해 운영체제에 묻는 기능 모음의 목록이다. Unreal FGenericPlatformProcess에 해당한다.
// 모든 플랫폼이 제공해야 하는 함수를 여기에 선언하고 설명한다. 플랫폼 struct(FWindowsPlatformProcess 등)가 이것을 상속해
// 자기 구현을 다시 선언한다. 쓰는 쪽은 이 struct가 아니라 HAL/PlatformProcess.h의 FPlatformProcess를 쓴다.
struct FGenericPlatformProcess
{
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
