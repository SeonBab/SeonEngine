#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/** CPU 메모리에서 RGBA8 이미지 한 장의 픽셀을 소유한다. */
struct Image
{
	// 픽셀 단위 크기다. 데이터가 없으면 두 값이 0이다.
	std::uint32_t width = 0;
	std::uint32_t height = 0;

	// 위쪽 행부터 R, G, B, A 순서로 저장한다.
	// 행 사이 여백 없이 width * height * 4바이트를 보관한다.
	std::vector<std::byte> pixels;
};
