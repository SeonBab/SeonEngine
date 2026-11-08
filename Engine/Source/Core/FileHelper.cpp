#include "FileHelper.h"

#include <fstream>
#include <iterator>
#include <utility>

bool FileHelper::ReadAllBytes(const std::filesystem::path& path, std::vector<std::byte>& outBytes)
{
	// 끝에서 열어 현재 위치로 파일 크기를 얻는다.
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file) { return false; }

	const std::streamsize size = file.tellg();
	if (size < 0) { return false; }

	// 실패해도 outBytes가 일부만 채워지지 않도록 지역 배열에 먼저 읽는다.
	std::vector<std::byte> bytes(static_cast<std::size_t>(size));
	file.seekg(0);
	if (!file.read(reinterpret_cast<char*>(bytes.data()), size)) { return false; }

	outBytes = std::move(bytes);
	return true;
}

bool FileHelper::ReadAllText(const std::filesystem::path& path, std::string& outText)
{
	// 옵션 없이 열면 텍스트 모드다.
	std::ifstream file(path);
	if (!file) { return false; }

	// 줄바꿈 변환 때문에 읽은 양이 파일 크기와 다를 수 있어 크기를 미리 구하지 않고 끝까지 차례로 읽는다.
	std::string text{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	if (file.bad()) { return false; }

	outText = std::move(text);
	return true;
}
