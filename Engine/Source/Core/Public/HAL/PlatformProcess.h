#pragma once

#include "Containers/SeonString.h"

#include <optional>

// 실행 중인 프로그램(프로세스)에 대해 운영체제에 묻는 기능 모음이다. Unreal FPlatformProcess에 해당한다.
// 여기에는 선언만 두고, 구현은 플랫폼 폴더의 .cpp(Platform/Windows/WindowsPlatformProcess.cpp)에 둔다.
struct FPlatformProcess
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
