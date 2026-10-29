#include "Paths.h"

#include "Core/Logging/Log.h"
#include "Windows/WindowsHWrapper.h"

#include <limits>
#include <string>

namespace
{
	bool EqualPathComponent(const std::filesystem::path& left, const std::filesystem::path& right)
	{
		return CompareStringOrdinal(left.c_str(), -1, right.c_str(), -1, TRUE) == CSTR_EQUAL;
	}

	std::optional<FString> ConvertWideToUtf8(std::wstring_view text)
	{
		if (text.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
		{
			WriteLog("Paths", ELogLevel::Error, "Physical path exceeds the UTF-16 conversion length limit.");
			return std::nullopt;
		}

		const int inputLength = static_cast<int>(text.size());
		const int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), inputLength, nullptr, 0, nullptr, nullptr);
		if (utf8Length == 0)
		{
			WriteLog("Paths", ELogLevel::Error, "Physical path UTF-8 conversion failed (error {}).", GetLastError());
			return std::nullopt;
		}

		FString result(static_cast<size_t>(utf8Length), '\0');
		if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), inputLength, result.data(), utf8Length, nullptr, nullptr) != utf8Length)
		{
			WriteLog("Paths", ELogLevel::Error, "Physical path UTF-8 conversion failed (error {}).", GetLastError());
			return std::nullopt;
		}

		return result;
	}

	bool ValidateContentPath(FStringView contentPath)
	{
		if (!contentPath.starts_with("/Engine/") && !contentPath.starts_with("/Game/"))
		{
			WriteLog("Paths", ELogLevel::Error, "Content path must start with /Engine/ or /Game/.");
			return false;
		}
		if (contentPath.find('\0') != FStringView::npos || contentPath.find_first_of("\\:") != FStringView::npos)
		{
			WriteLog("Paths", ELogLevel::Error, "Content path must not contain NUL, backslashes or colons.");
			return false;
		}

		const FStringView relativePath = contentPath.substr(1);
		for (FStringView::size_type segmentStart = 0; segmentStart <= relativePath.size();)
		{
			FStringView::size_type segmentEnd = relativePath.find('/', segmentStart);
			const bool bLastSegment = segmentEnd == FStringView::npos;
			if (bLastSegment)
			{
				segmentEnd = relativePath.size();
			}
			const FStringView segment = relativePath.substr(segmentStart, segmentEnd - segmentStart);
			if (segment.empty() || segment == "." || segment == "..")
			{
				WriteLog("Paths", ELogLevel::Error, "Content path must contain a file part and must not contain empty, . or .. segments.");
				return false;
			}
			if (segment.back() == ' ' || segment.back() == '.')
			{
				WriteLog("Paths", ELogLevel::Error, "Content path segments must not end with a space or dot.");
				return false;
			}
			if (bLastSegment) { break; }
			segmentStart = segmentEnd + 1;
		}

		return true;
	}

	std::optional<std::wstring> ConvertUtf8ToWide(FStringView text)
	{
		if (text.size() > static_cast<FStringView::size_type>(std::numeric_limits<int>::max()))
		{
			WriteLog("Paths", ELogLevel::Error, "Content path exceeds the UTF-8 conversion length limit.");
			return std::nullopt;
		}

		const int inputLength = static_cast<int>(text.size());
		// 먼저 필요한 UTF-16 요소 수를 구한 뒤 그 크기의 버퍼에 실제 변환 결과를 쓴다.
		const int wideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), inputLength, nullptr, 0);
		if (wideLength == 0)
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Paths", ELogLevel::Error, "Content path UTF-8 conversion failed (error {}).", errorCode);
			return std::nullopt;
		}
		std::wstring widePath(static_cast<std::wstring::size_type>(wideLength), L'\0');
		const int convertedLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), inputLength, widePath.data(), wideLength);
		if (convertedLength != wideLength)
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Paths", ELogLevel::Error, "Content path UTF-8 conversion failed (error {}).", errorCode);
			return std::nullopt;
		}

		return widePath;
	}
}

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

std::optional<std::filesystem::path> FPaths::ResolveContentPath(FStringView contentPath)
{
	if (!ValidateContentPath(contentPath)) { return std::nullopt; }

	// 논리 루트의 선행 /를 제거해 OS 절대경로로 해석되지 않게 한다. Engine/Game 이름은 출력 하위 폴더와 같다.
	const FStringView relativePath = contentPath.substr(1);
	const std::optional<std::wstring> widePath = ConvertUtf8ToWide(relativePath);
	if (!widePath) { return std::nullopt; }

	const std::optional<std::filesystem::path> baseDir = BaseDir();
	if (!baseDir) { return std::nullopt; }

	return *baseDir / L"Content" / std::filesystem::path(*widePath);
}

std::optional<FString> FPaths::MakeContentPath(const std::filesystem::path& path)
{
	if (!path.is_absolute() || path.native().find(L'\0') != std::wstring::npos)
	{
		WriteLog("Paths", ELogLevel::Error, "Content conversion requires an absolute physical path without NUL.");
		return std::nullopt;
	}

	const std::optional<std::filesystem::path> baseDir = BaseDir();
	if (!baseDir) { return std::nullopt; }

	const std::filesystem::path contentRoot = (*baseDir / L"Content").lexically_normal();
	const std::filesystem::path normalizedPath = path.lexically_normal();
	auto component = normalizedPath.begin();
	for (const std::filesystem::path& rootComponent : contentRoot)
	{
		if (component == normalizedPath.end() || !EqualPathComponent(*component, rootComponent))
		{
			WriteLog("Paths", ELogLevel::Error, "Physical path must be inside the executable Content directory.");
			return std::nullopt;
		}
		++component;
	}

	std::wstring logicalPath;
	if (component != normalizedPath.end() && EqualPathComponent(*component, L"Engine"))
	{
		logicalPath = L"/Engine";
	}
	else if (component != normalizedPath.end() && EqualPathComponent(*component, L"Game"))
	{
		logicalPath = L"/Game";
	}
	else
	{
		WriteLog("Paths", ELogLevel::Error, "Physical path must be inside Content/Engine or Content/Game.");
		return std::nullopt;
	}

	for (++component; component != normalizedPath.end(); ++component)
	{
		logicalPath += L'/';
		logicalPath += component->native();
	}
	const std::optional<FString> result = ConvertWideToUtf8(logicalPath);
	if (!result || !ValidateContentPath(*result)) { return std::nullopt; }

	return result;
}
