#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"

#include <filesystem>

/** 리소스 형식이나 렌더링 API와 무관한 파일 바이트 읽기 기능이다. */
struct FFileHelper
{
	/**
	 * 물리 경로의 파일을 binary로 열고 확인한 크기만큼 읽어 출력 배열을 교체한다. 빈 파일도 성공하며 출력을 비운다.
	 * 열기/크기/위치 이동/전체 읽기 실패는 로그/false와 기존 출력 유지다. 메모리 할당 등의 예외는 전파한다.
	 * 반환 배열은 바이트를 소유한다. 경로 변환, 파일 형식 검증, GPU 생성과 캐시는 수행하지 않는다.
	 */
	[[nodiscard]] static bool ReadBinaryFile(const std::filesystem::path& path, TArray<uint8>& outBytes);
};
