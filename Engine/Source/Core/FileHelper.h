#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

/** 파일을 읽는 도우미 함수를 모은다. */
namespace FileHelper
{
	/** 파일 전체를 바이트 배열로 읽는다. 열거나 읽지 못하면 출력값을 유지하고 false를 반환한다. */
	bool ReadAllBytes(const std::filesystem::path& path, std::vector<std::byte>& outBytes);

	/** 텍스트 파일 전체를 텍스트 모드로 읽는다. 열거나 읽지 못하면 출력값을 유지하고 false를 반환한다. */
	bool ReadAllText(const std::filesystem::path& path, std::string& outText);
}
