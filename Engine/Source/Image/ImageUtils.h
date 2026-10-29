#pragma once

#include "Image/Image.h"

#include <filesystem>

////////////////////////////////////////////////////////////////////////////////
// 이미지 파일을 CPU 이미지 데이터로 읽고 RGBA8 픽셀을 처리하는 보조 함수를 제공한다.
// Windows WIC로 PNG, BMP와 단일 2D BC1/BC2/BC3 UNORM DDS, 자체 로더로 비압축 24비트 TGA를 지원한다.
////////////////////////////////////////////////////////////////////////////////
struct FImageUtils
{
	/**
	 * 단일 프레임 PNG/BMP, 지원 DDS의 최상위 밉 또는 비압축 24비트 TGA를 RGBA8로 읽는다. 상대 경로는 현재 작업 폴더 기준이다.
	 * 대소문자 구분 없는 .tga 확장자는 자체 로더로, 나머지는 WIC로 읽는다.
	 * TGA는 색상표, 인터리브를 지원하지 않으며 저장 방향을 위쪽 행/왼쪽 픽셀부터로 맞추고 알파를 255로 채운다.
	 * DDS의 압축, 하위 밉은 출력에 보관하지 않으며 배열, 큐브, 3D 텍스처는 거부한다.
	 * 실패하면 로그와 false를 반환하며 기존 outImage는 유지한다. GPU 자원은 생성하지 않는다.
	 * 감마, 색상 프로필 변환과 PNG/DDS의 방향 메타데이터 적용은 수행하지 않으며 메모리 부족의 false 복구는 보장하지 않는다.
	 */
	[[nodiscard]] static bool LoadImageFile(const std::filesystem::path& path, FImage& outImage);

	/**
	 * RGB 각 채널의 차이가 tolerance 이하인 픽셀을 투명 검정 RGBA(0, 0, 0, 0)으로 바꾼다.
	 * tolerance는 RGBA8의 0~255 단위이며 기본 0은 정확 일치다. 비교에 기존 알파는 사용하지 않는다.
	 * 불일치 픽셀의 RGB와 알파는 유지한다. 파일 쓰기나 GPU 자원 생성은 수행하지 않는다.
	 * 크기가 양수가 아니거나 RGBA8 배열 길이가 맞지 않으면 로그와 false를 반환하고 입력을 유지한다.
	 */
	[[nodiscard]] static bool ApplyColorKey(FImage& image, uint8 red, uint8 green, uint8 blue, uint8 tolerance = 0);
};
