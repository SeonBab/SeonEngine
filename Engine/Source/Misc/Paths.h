#pragma once

#include <filesystem>
#include <optional>

////////////////////////////////////////////////////////////////////////////////
// 실행 파일을 기준으로 경로를 구한다. 현재 구현은 Windows를 사용한다.
////////////////////////////////////////////////////////////////////////////////
struct FPaths
{
	/** 실행 파일의 부모 폴더를 반환한다. API 실패나 경로 잘림은 로그와 nullopt로 전달하며 폴더를 생성하지 않는다. */
	[[nodiscard]] static std::optional<std::filesystem::path> BaseDir();
};
