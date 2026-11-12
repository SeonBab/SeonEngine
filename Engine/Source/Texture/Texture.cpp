#include "Texture.h"

#include "Image.h"

#include <limits>
#include <utility>

Texture::Texture(std::string newName, std::string newPath)
	: Asset(std::move(newName), std::move(newPath))
{
}

bool Texture::Initialize(Image&& image, TextureFormat pixelFormat)
{
	if (format != TextureFormat::Unknown) { return false; }
	if (image.width == 0 || image.height == 0) { return false; }

	const bool isSupportedFormat = pixelFormat == TextureFormat::RGBA8UNorm || pixelFormat == TextureFormat::RGBA8SRGB;
	if (!isSupportedFormat) { return false; }

	const std::uint64_t pixelCount = static_cast<std::uint64_t>(image.width) * image.height;
	if (pixelCount > (std::numeric_limits<std::size_t>::max)() / 4) { return false; }
	const std::size_t byteCount = static_cast<std::size_t>(pixelCount) * 4;
	if (image.pixels.size() != byteCount) { return false; }

	width = image.width;
	height = image.height;
	format = pixelFormat;
	pixels = std::move(image.pixels);
	return true;
}
