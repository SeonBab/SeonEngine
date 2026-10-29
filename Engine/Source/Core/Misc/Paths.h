#pragma once

#include "Core/Containers/SeonString.h"

#include <filesystem>
#include <optional>

////////////////////////////////////////////////////////////////////////////////
// 실행 파일을 기준으로 경로를 구한다. 현재 구현은 Windows를 사용한다.
////////////////////////////////////////////////////////////////////////////////
struct FPaths
{
	/** 실행 파일의 부모 폴더를 반환한다. API 실패나 경로 잘림은 로그와 nullopt로 전달하며 폴더를 생성하지 않는다. */
	[[nodiscard]] static std::optional<std::filesystem::path> BaseDir();

	/**
	 * UTF-8 논리 경로 /Engine/... 또는 /Game/...을 실행 파일 기준 Content/Engine 또는 Content/Game 아래 경로로 변환한다.
	 * 루트 이름은 대소문자를 구분하고 구분자는 /만 허용한다. 파일 부분이 있어야 하며 빈 요소, .과 .., 역슬래시, 콜론, NUL은 거부한다.
	 * Windows의 경로 별칭 해석을 피하도록 각 요소 끝의 공백과 마침표도 거부한다.
	 * 잘못된 입력, UTF-8 변환이나 BaseDir 실패는 로그와 nullopt다. 입력을 저장하지 않고 반환 경로가 자체 문자열을 소유한다.
	 * 파일 존재나 종류, 심볼릭 링크의 실제 목적지는 검사하지 않으며 파일 읽기, 폴더 생성, 자산 로딩도 수행하지 않는다.
	 */
	[[nodiscard]] static std::optional<std::filesystem::path> ResolveContentPath(FStringView contentPath);

	/**
	 * 절대 물리 경로를 실행 파일 기준 Content/Engine 또는 Content/Game 아래의 UTF-8 논리 경로로 바꾼다.
	 * .과 ..을 어휘적으로 정리한 뒤 폴더 요소별로 Windows의 대소문자 무시 비교를 한다. 상대경로·루트 폴더 자체·범위 밖은 거부한다.
	 * 논리 루트는 /Engine 또는 /Game으로 표기하고 나머지 철자는 유지한다. 결과에는 ResolveContentPath의 문법 제약을 적용한다.
	 * 실패는 로그와 nullopt다. 파일 존재나 심볼릭 링크의 실제 목적지는 검사하지 않으며 보안 경계 검사는 아니다.
	 */
	[[nodiscard]] static std::optional<FString> MakeContentPath(const std::filesystem::path& path);
};
