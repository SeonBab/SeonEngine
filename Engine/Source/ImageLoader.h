#pragma once

#include <filesystem>

struct Image;

namespace ImageLoader
{
	/**
	 * PNG 파일을 RGBA8 이미지로 읽는다. 상대 경로는 현재 작업 폴더 기준이다.
	 * 성공하면 결과를 outImage에 저장하고 true를 반환한다.
	 * 실패하면 기존 outImage를 유지하고 오류를 기록한 뒤 false를 반환한다.
	 */
	bool LoadPNG(const std::filesystem::path& path, Image& outImage);
}
