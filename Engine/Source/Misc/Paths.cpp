#include "Paths.h"

#include "Logging/Log.h"
#include "Windows/WindowsHWrapper.h"

#include <string>

std::optional<std::filesystem::path> FPaths::BaseDir()
{
	constexpr DWORD PathCapacity = 32768;
	std::wstring executablePath(PathCapacity, L'\0');
	const DWORD length = GetModuleFileNameW(nullptr, executablePath.data(), PathCapacity);
	if (length == 0)
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Paths", ELogLevel::Error, "GetModuleFileNameW failed (error {})", errorCode);
		return std::nullopt;
	}
	if (length >= PathCapacity)
	{
		WriteLog("Paths", ELogLevel::Error, "Executable path exceeds the supported buffer size.");
		return std::nullopt;
	}

	executablePath.resize(length);

	return std::filesystem::path(executablePath).parent_path();
}
