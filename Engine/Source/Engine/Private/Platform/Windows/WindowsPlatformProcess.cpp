#include "Windows/WindowsPlatformProcess.h"

#include "Containers/SeonString.h"
#include "Platform/Windows/WindowsHWrapper.h"
#include "Platform/Windows/WindowsString.h"

#include <algorithm>
#include <optional>
#include <string>

namespace
{
	// Windows 경로의 최대 길이(32767칸)에 끝의 '\0' 한 칸을 더한 것. 이보다 긴 경로는 없어서 한 번에 받을 수 있다
	constexpr DWORD MaxPathLength = 32767 + 1;

	// 실행 파일의 전체 경로를 엔진 경로 모양(UTF-8, '/' 구분자)으로 받는다
	std::optional<FString> GetExecutablePath()
	{
		std::wstring widePath(MaxPathLength, L'\0');

		// 첫 인자 nullptr: 지금 실행 중인 프로그램의 실행 파일.
		// 버퍼가 작으면 잘린 경로를 주고도 성공하므로 최대 길이로 한 번에 받는다
		const DWORD length = ::GetModuleFileNameW(nullptr, widePath.data(), MaxPathLength);

		// 0은 실패, MaxPathLength는 잘렸다는 뜻이다. 최대 길이로 받으므로 잘림은 일어나지 않아야 한다
		if (length == 0 || length >= MaxPathLength)
		{
			return std::nullopt;
		}

		// 받은 글자 수만 남기고 뒤의 빈 칸을 버린다
		widePath.resize(length);

		// 경로는 OS에 다시 넘기므로, 깨진 글자를 바꾸면 다른 파일을 가리킨다. 바꾸지 않고 실패시킨다
		std::optional<FString> path = FWindowsString::WideToUTF8(widePath, EInvalidCharacter::Fail);
		if (!path)
		{
			return std::nullopt;
		}

		// 엔진 경로는 '/'로 나눈다. Windows API도 '/'를 구분자로 받는다
		std::replace(path->begin(), path->end(), '\\', '/');
		return path;
	}

	// 실행 파일 경로에서 마지막 '/'까지 남겨 폴더 경로를 만든다
	std::optional<FString> ComputeBaseDir()
	{
		std::optional<FString> path = GetExecutablePath();
		if (!path)
		{
			return std::nullopt;
		}

		// 마지막 '/' 뒤가 실행 파일 이름이다. 전체 경로라 '/'가 없을 수 없지만, 없으면 잘못 자르지 않고 실패시킨다
		const size_t lastSlash = path->find_last_of('/');
		if (lastSlash == FString::npos)
		{
			return std::nullopt;
		}

		// '/'까지 남겨 끝에 '/'가 붙은 폴더 경로로 만든다
		path->resize(lastSlash + 1);
		return path;
	}

	// 실행 파일 경로에서 마지막 '/' 뒤의 파일 이름을 꺼내고 확장자를 뺀다
	std::optional<FString> ComputeExecutableName()
	{
		std::optional<FString> path = GetExecutablePath();
		if (!path)
		{
			return std::nullopt;
		}

		// 마지막 '/' 뒤가 파일 이름이다. '/'가 없으면 전체가 파일 이름이다
		const size_t lastSlash = path->find_last_of('/');
		FString name = (lastSlash == FString::npos) ? *path : path->substr(lastSlash + 1);

		// 마지막 '.'부터 확장자다. "SampleGame.exe" → "SampleGame"
		const size_t lastDot = name.find_last_of('.');
		if (lastDot != FString::npos)
		{
			name.resize(lastDot);
		}

		// 이름이 비면(".exe" 같은 파일) 로그 파일 이름을 만들 수 없으므로 실패시킨다
		if (name.empty())
		{
			return std::nullopt;
		}
		return name;
	}
}

std::optional<FString> FWindowsPlatformProcess::BaseDir()
{
	// 실행 중에 실행 파일 위치는 바뀌지 않으므로 처음 부를 때 한 번만 구한다
	static const std::optional<FString> baseDir = ComputeBaseDir();
	return baseDir;
}

std::optional<FString> FWindowsPlatformProcess::ExecutableName()
{
	// 실행 중에 실행 파일 이름은 바뀌지 않으므로 처음 부를 때 한 번만 구한다
	static const std::optional<FString> executableName = ComputeExecutableName();
	return executableName;
}
