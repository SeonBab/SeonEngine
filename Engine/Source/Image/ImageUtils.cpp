#include "ImageUtils.h"

#include "Core/CoreTypes.h"
#include "Core/Logging/Log.h"
#include "Windows/WindowsHWrapper.h"

#include <wincodec.h>
#include <wrl/client.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <limits>
#include <utility>

namespace
{
	struct FTGAHeader
	{
		uint8 idLength = 0;
		uint8 colorMapType = 0;
		uint8 imageType = 0;
		uint16 colorMapLength = 0;
		uint16 width = 0;
		uint16 height = 0;
		uint8 bitsPerPixel = 0;
		uint8 descriptor = 0;
	};

	bool ReadTGAHeader(std::ifstream& file, FTGAHeader& outHeader)
	{
		std::array<uint8, 18> bytes{};
		if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
		{
			WriteLog("Image", ELogLevel::Error, "Reading a TGA header requires 18 bytes.");
			return false;
		}

		// 파일의 16비트 값은 낮은 바이트부터 저장되므로 구조체 정렬과 무관하게 해석한다.
		const auto readUInt16 = [&bytes](std::size_t offset)
		{
			return static_cast<uint16>(bytes[offset] | (bytes[offset + 1] << 8));
		};
		FTGAHeader header;
		header.idLength = bytes[0];
		header.colorMapType = bytes[1];
		header.imageType = bytes[2];
		header.colorMapLength = readUInt16(5);
		header.width = readUInt16(12);
		header.height = readUInt16(14);
		header.bitsPerPixel = bytes[16];
		header.descriptor = bytes[17];
		outHeader = header;

		return true;
	}

	bool ValidateTGAHeader(const FTGAHeader& header)
	{
		if (header.imageType != 2)
		{
			WriteLog("Image", ELogLevel::Error, "Only uncompressed truecolor TGA images are supported.");
			return false;
		}
		if (header.colorMapType != 0 || header.colorMapLength != 0)
		{
			WriteLog("Image", ELogLevel::Error, "TGA color maps are not supported.");
			return false;
		}
		if (header.width == 0 || header.height == 0)
		{
			WriteLog("Image", ELogLevel::Error, "TGA image dimensions must be positive.");
			return false;
		}
		if (header.bitsPerPixel != 24)
		{
			WriteLog("Image", ELogLevel::Error, "Only 24-bit TGA images are supported.");
			return false;
		}
		if ((header.descriptor & 0xC0) != 0)
		{
			WriteLog("Image", ELogLevel::Error, "Interleaved TGA images are not supported.");
			return false;
		}

		return true;
	}

	bool LoadTGAImage(const std::filesystem::path& path, FImage& outImage)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			WriteLog("Image", ELogLevel::Error, "Opening the TGA image file failed.");
			return false;
		}
		const std::streamoff fileSize = file.tellg();
		if (fileSize < 0 || !file.seekg(0, std::ios::beg))
		{
			WriteLog("Image", ELogLevel::Error, "Reading the TGA file size or seeking to its header failed.");
			return false;
		}
		FTGAHeader header;
		if (!ReadTGAHeader(file, header) || !ValidateTGAHeader(header)) { return false; }

		const uint64 pixelCount = static_cast<uint64>(header.width) * header.height;
		const uint64 sourceByteCount = pixelCount * 3;
		const uint64 byteCount = pixelCount * 4;
		const std::streamoff pixelOffset = 18 + header.idLength;
		FImage image;
		if (fileSize < pixelOffset || static_cast<uint64>(fileSize - pixelOffset) < sourceByteCount || byteCount > image.rawData.max_size())
		{
			WriteLog("Image", ELogLevel::Error, "TGA pixel data is incomplete or exceeds the supported buffer size.");
			return false;
		}
		file.ignore(header.idLength);
		if (!file || file.gcount() != header.idLength)
		{
			WriteLog("Image", ELogLevel::Error, "Skipping the TGA image ID failed.");
			return false;
		}

		image.sizeX = header.width;
		image.sizeY = header.height;
		image.rawData.resize(static_cast<std::size_t>(byteCount));
		TArray<uint8> row(static_cast<std::size_t>(header.width) * 3);
		const bool bTopOrigin = (header.descriptor & 0x20) != 0;
		const bool bRightOrigin = (header.descriptor & 0x10) != 0;
		for (std::size_t y = 0; y < header.height; ++y)
		{
			if (!file.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(row.size())))
			{
				WriteLog("Image", ELogLevel::Error, "Reading a TGA pixel row failed.");
				return false;
			}
			const std::size_t destinationY = bTopOrigin ? y : header.height - 1 - y;
			for (std::size_t x = 0; x < header.width; ++x)
			{
				const std::size_t destinationX = bRightOrigin ? header.width - 1 - x : x;
				const std::size_t source = x * 3;
				const std::size_t destination = (destinationY * header.width + destinationX) * 4;
				image.rawData[destination] = row[source + 2];
				image.rawData[destination + 1] = row[source + 1];
				image.rawData[destination + 2] = row[source];
				image.rawData[destination + 3] = 255;
			}
		}
		outImage = std::move(image);

		return true;
	}

	bool CheckWICResult(HRESULT result, const char* operation)
	{
		if (FAILED(result))
		{
			WriteLog("Image", ELogLevel::Error, "{} failed (HRESULT 0x{:08X}).", operation, static_cast<unsigned long>(result));
			return false;
		}

		return true;
	}

	bool GetImageFrame(IWICBitmapDecoder* decoder, Microsoft::WRL::ComPtr<IWICBitmapFrameDecode>& outFrame)
	{
		GUID containerFormat{};
		if (!CheckWICResult(decoder->GetContainerFormat(&containerFormat), "Get image container format")) { return false; }
		if (IsEqualGUID(containerFormat, GUID_ContainerFormatPng))
		{
			UINT frameCount = 0;
			if (!CheckWICResult(decoder->GetFrameCount(&frameCount), "Get image frame count")) { return false; }
			if (frameCount != 1)
			{
				WriteLog("Image", ELogLevel::Error, "A PNG image must have one frame (got {}).", frameCount);
				return false;
			}

			return CheckWICResult(decoder->GetFrame(0, outFrame.GetAddressOf()), "Get image frame");
		}
		if (IsEqualGUID(containerFormat, GUID_ContainerFormatDds))
		{
			Microsoft::WRL::ComPtr<IWICDdsDecoder> ddsDecoder;
			if (!CheckWICResult(decoder->QueryInterface(IID_PPV_ARGS(ddsDecoder.GetAddressOf())), "Get DDS decoder")) { return false; }
			WICDdsParameters parameters{};
			if (!CheckWICResult(ddsDecoder->GetParameters(&parameters), "Get DDS parameters")) { return false; }
			if (parameters.Dimension != WICDdsTexture2D || parameters.ArraySize != 1 || parameters.Depth != 1 || parameters.MipLevels == 0)
			{
				WriteLog("Image", ELogLevel::Error, "Only a single 2D DDS texture with at least one mip level is supported.");
				return false;
			}
			if (parameters.DxgiFormat != DXGI_FORMAT_BC1_UNORM && parameters.DxgiFormat != DXGI_FORMAT_BC2_UNORM && parameters.DxgiFormat != DXGI_FORMAT_BC3_UNORM)
			{
				WriteLog("Image", ELogLevel::Error, "Only BC1, BC2 and BC3 UNORM DDS formats are supported.");
				return false;
			}

			// DDS의 밉을 PNG의 별도 프레임처럼 거부하지 않고 가장 큰 이미지 하나만 읽는다.
			return CheckWICResult(ddsDecoder->GetFrame(0, 0, 0, outFrame.GetAddressOf()), "Get DDS top mip frame");
		}

		WriteLog("Image", ELogLevel::Error, "Only PNG and supported 2D DDS image files are supported.");

		return false;
	}
}

bool FImageUtils::LoadImageFile(const std::filesystem::path& path, FImage& outImage)
{
	if (path.empty() || path.native().find(L'\0') != std::filesystem::path::string_type::npos)
	{
		WriteLog("Image", ELogLevel::Error, "An image file path must be nonempty and contain no embedded null characters.");
		return false;
	}

	const std::filesystem::path::string_type extension = path.extension().native();
	if (extension.size() == 4 && extension[0] == L'.' && (extension[1] == L't' || extension[1] == L'T') && (extension[2] == L'g' || extension[2] == L'G') && (extension[3] == L'a' || extension[3] == L'A'))
	{
		return LoadTGAImage(path, outImage);
	}

	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	// 기존 apartment를 바꾸지 않고 이미 초기화된 환경에서도 WIC를 사용한다.
	if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE)
	{
		(void)CheckWICResult(comResult, "CoInitializeEx");
		return false;
	}

	// NOTE: CoUninitialize 전에 모든 WIC 참조가 파괴되도록 중간 실패도 이 람다 안에서 반환한다.
	const bool loaded = [&]() -> bool
	{
		Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
		if (!CheckWICResult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf())), "Create WIC factory"))
		{
			return false;
		}

		Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
		if (!CheckWICResult(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf()), "Open image decoder"))
		{
			return false;
		}

		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
		if (!GetImageFrame(decoder.Get(), frame))
		{
			return false;
		}
		UINT width = 0;
		UINT height = 0;
		if (!CheckWICResult(frame->GetSize(&width, &height), "Get image dimensions"))
		{
			return false;
		}
		if (width == 0 || height == 0 || width > static_cast<UINT>(std::numeric_limits<int32>::max()) || height > static_cast<UINT>(std::numeric_limits<int32>::max()))
		{
			WriteLog("Image", ELogLevel::Error, "Unsupported image dimensions ({} x {}).", width, height);
			return false;
		}

		// 각 차원을 int32로 제한한 뒤 계산하므로 곱은 uint64 안에서 표현된다.
		const uint64 stride = static_cast<uint64>(width) * 4;
		const uint64 byteCount = stride * height;
		FImage image;
		if (stride > std::numeric_limits<UINT>::max() || byteCount > std::numeric_limits<UINT>::max() || byteCount > image.rawData.max_size())
		{
			WriteLog("Image", ELogLevel::Error, "Image RGBA8 data exceeds the supported buffer size.");
			return false;
		}

		Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
		if (!CheckWICResult(factory->CreateFormatConverter(converter.GetAddressOf()), "Create RGBA converter"))
		{
			return false;
		}
		// RGBA는 알파를 미리 곱한 PRGBA와 다르다.
		if (!CheckWICResult(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom), "Initialize RGBA conversion"))
		{
			return false;
		}
		image.sizeX = static_cast<int32>(width);
		image.sizeY = static_cast<int32>(height);
		image.rawData.resize(static_cast<size_t>(byteCount));
		if (!CheckWICResult(converter->CopyPixels(nullptr, static_cast<UINT>(stride), static_cast<UINT>(byteCount), image.rawData.data()), "Copy RGBA pixels"))
		{
			return false;
		}

		outImage = std::move(image);

		return true;
	}();

	// NOTE: S_FALSE도 초기화 성공이므로 대응 해제한다. 실패인 RPC_E_CHANGED_MODE는 해제하지 않는다.
	if (SUCCEEDED(comResult))
	{
		CoUninitialize();
	}

	return loaded;
}

bool FImageUtils::ApplyColorKey(FImage& image, uint8 red, uint8 green, uint8 blue, uint8 tolerance)
{
	if (image.sizeX <= 0 || image.sizeY <= 0)
	{
		WriteLog("Image", ELogLevel::Error, "Applying a color key requires positive image dimensions.");
		return false;
	}
	const uint64 byteCount = static_cast<uint64>(image.sizeX) * static_cast<uint64>(image.sizeY) * 4;
	if (byteCount != image.rawData.size())
	{
		WriteLog("Image", ELogLevel::Error, "Applying a color key requires a complete RGBA8 image buffer.");
		return false;
	}

	for (size_t offset = 0; offset < image.rawData.size(); offset += 4)
	{
		const int32 redDifference = static_cast<int32>(image.rawData[offset]) - red;
		const int32 greenDifference = static_cast<int32>(image.rawData[offset + 1]) - green;
		const int32 blueDifference = static_cast<int32>(image.rawData[offset + 2]) - blue;
		if (redDifference >= -static_cast<int32>(tolerance) && redDifference <= tolerance &&
			greenDifference >= -static_cast<int32>(tolerance) && greenDifference <= tolerance &&
			blueDifference >= -static_cast<int32>(tolerance) && blueDifference <= tolerance)
		{
			image.rawData[offset] = 0;
			image.rawData[offset + 1] = 0;
			image.rawData[offset + 2] = 0;
			image.rawData[offset + 3] = 0;
		}
	}

	return true;
}
