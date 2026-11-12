#pragma once

#include "Asset.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct Image;

enum class TextureFormat
{
	Unknown,
	RGBA8UNorm,
	RGBA8SRGB
};

class Texture : public Asset
{
public:
	/** UTF-8 이름과 저장소 루트 기준 상대 경로를 받는다. 파일 없이 생성하면 빈 경로를 사용한다. */
	Texture(std::string newName, std::string newPath);

public:
	/**
	 * RGBA8 이미지의 픽셀 소유권을 가져와 한 번 초기화한다.
	 * 이미 초기화했거나 입력이 유효하지 않으면 Texture와 image를 변경하지 않고 false를 반환한다.
	 */
	bool Initialize(Image&& image, TextureFormat pixelFormat);

	std::uint32_t GetWidth() const { return width; }
	std::uint32_t GetHeight() const { return height; }
	TextureFormat GetFormat() const { return format; }
	const std::vector<std::byte>& GetPixels() const { return pixels; }

private:
	// 크기가 정해지기 전에는 두 값이 0이다.
	std::uint32_t width = 0;
	std::uint32_t height = 0;

	// 이미지 형식이 정해지기 전에는 Unknown이다.
	TextureFormat format = TextureFormat::Unknown;

	// 위쪽 행부터 RGBA 순서이며, 행 사이 여백은 없다.
	std::vector<std::byte> pixels;
};
