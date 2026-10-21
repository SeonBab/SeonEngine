#pragma once

#include <filesystem>

struct FModelData;

struct FModelLoader
{
	// 텍스트 X 파일을 읽어 출처 경로와 CPU 모델 데이터를 구성한다
	// 실패하면 기존 출력 모델을 유지한다
	[[nodiscard]] static bool LoadTextXModel(const std::filesystem::path& path, FModelData& outModel);
};
