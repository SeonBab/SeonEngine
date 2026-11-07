#include "Paths.h"

#include <Windows.h>

#include <string>

bool Paths::GetRootDirectory(std::filesystem::path& outPath)
{
	// MAX_PATH(260자)보다 긴 경로도 받도록 Windows 최대 경로 길이로 잡는다.
	std::wstring executablePath(32768, L'\0');
	const DWORD length = GetModuleFileNameW(nullptr, executablePath.data(), static_cast<DWORD>(executablePath.size()));

	// 0은 실패, 버퍼 크기 이상이면 경로가 잘렸다는 뜻이다.
	if (length == 0 || length >= executablePath.size()) { return false; }
	executablePath.resize(length);

	// <루트>/Binaries/<도구 집합>/<플랫폼>/<구성>/SeonEngine.exe에서 네 번 올라간 곳이 Binaries가 아니면
	// 출력 배치가 바뀐 것이므로 엉뚱한 폴더를 루트로 쓰지 않는다.
	const std::filesystem::path binariesDirectory = std::filesystem::path(executablePath).parent_path().parent_path().parent_path().parent_path();
	if (binariesDirectory.filename() != L"Binaries") { return false; }

	outPath = binariesDirectory.parent_path();
	return true;
}

bool Paths::GetAbsolutePath(std::string_view relativePath, std::filesystem::path& outPath)
{
	std::filesystem::path rootDirectory;
	if (!GetRootDirectory(rootDirectory)) { return false; }

	// Windows에서 std::string으로 만든 path는 CP949로 해석되므로 UTF-8로 해석하게 한다.
	outPath = rootDirectory / std::filesystem::path(std::u8string(relativePath.begin(), relativePath.end()));
	return true;
}
