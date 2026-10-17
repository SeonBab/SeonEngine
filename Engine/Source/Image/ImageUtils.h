#pragma once

#include "Image/Image.h"

#include <filesystem>

////////////////////////////////////////////////////////////////////////////////
// 이미지 파일을 CPU 이미지 데이터로 읽는 보조 함수를 제공한다.
// Windows WIC로 PNG와 단일 2D BC1/BC2/BC3 UNORM DDS를 지원한다.
////////////////////////////////////////////////////////////////////////////////
struct FImageUtils
{
	/**
	 * 단일 프레임 PNG 또는 지원 DDS의 최상위 밉을 RGBA8로 읽는다. 상대 경로는 현재 작업 폴더 기준이다.
	 * DDS의 압축·하위 밉은 출력에 보관하지 않으며 배열·큐브·3D 텍스처는 거부한다.
	 * 실패하면 로그와 false를 반환하며 기존 outImage는 유지한다. GPU 자원은 생성하지 않는다.
	 * 감마·색상 프로필·방향 변환은 수행하지 않으며 메모리 부족의 false 복구는 보장하지 않는다.
	 */
	[[nodiscard]] static bool LoadImageFile(const std::filesystem::path& path, FImage& outImage);
};
