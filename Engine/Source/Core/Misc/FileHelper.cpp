#include "FileHelper.h"

#include "Core/Logging/Log.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <utility>

bool FFileHelper::ReadBinaryFile(const std::filesystem::path& path, TArray<uint8>& outBytes)
{
	// ate는 파일을 연 직후 끝으로 이동시켜 읽을 바이트 수를 확인하기 위한 설정이다.
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		WriteLog("File", ELogLevel::Error, "Opening a binary file failed.");
		return false;
	}

	const std::streampos endPosition = file.tellg();
	if (endPosition < std::streampos(0))
	{
		WriteLog("File", ELogLevel::Error, "Reading the binary file size failed.");
		return false;
	}

	const std::uintmax_t byteCount = static_cast<std::uintmax_t>(static_cast<std::streamoff>(endPosition));
	TArray<uint8> candidate;
	if (byteCount > candidate.max_size())
	{
		WriteLog("File", ELogLevel::Error, "Binary file size exceeds the supported array size.");
		return false;
	}
	if (byteCount > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
	{
		WriteLog("File", ELogLevel::Error, "Binary file size exceeds the supported stream read size.");
		return false;
	}

	file.seekg(0, std::ios::beg);
	if (!file)
	{
		WriteLog("File", ELogLevel::Error, "Seeking to the beginning of the binary file failed.");
		return false;
	}

	candidate.resize(static_cast<size_t>(byteCount));
	if (byteCount != 0)
	{
		if (!file.read(reinterpret_cast<char*>(candidate.data()), static_cast<std::streamsize>(byteCount)))
		{
			WriteLog("File", ELogLevel::Error, "Reading the complete binary file failed.");
			return false;
		}
	}

	// 유효한 빈 파일도 후보를 이동해 기존 출력을 비운다. 실패 경로에서는 출력에 손대지 않는다.
	outBytes = std::move(candidate);

	return true;
}
