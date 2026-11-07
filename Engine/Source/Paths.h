#pragma once

#include <filesystem>
#include <string_view>

/** 자산 파일 경로를 구한다. 저장소의 원본 폴더를 가리키므로 개발 중 실행에서만 동작한다. */
namespace Paths
{
	/**
	 * 저장소 루트 폴더를 구한다. 실행 파일이 Directory.Build.props의 OutDir(Binaries 아래)에 있어야 하며,
	 * 아니면 출력값을 유지하고 false를 반환한다.
	 */
	bool GetRootDirectory(std::filesystem::path& outPath);

	/**
	 * 저장소 루트 기준 UTF-8 상대 경로(예: Game/Content/Meshes/Box.gltf)를 절대 경로로 바꾼다.
	 * GetRootDirectory가 실패하면 출력값을 유지하고 false를 반환한다.
	 */
	bool GetAbsolutePath(std::string_view relativePath, std::filesystem::path& outPath);
}
