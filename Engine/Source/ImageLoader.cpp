#include "ImageLoader.h"

#include "Core/Log.h"
#include "Image.h"

#include <Windows.h>
#include <objbase.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <climits>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace
{
	/** 성공한 COM 초기화 호출에 대응하는 정리를 현재 스레드에서 수행한다. */
	class ScopedCOMUninitializer
	{
	public:
		explicit ScopedCOMUninitializer(HRESULT initializationResult)
			: shouldUninitialize(SUCCEEDED(initializationResult))
		{
		}

		~ScopedCOMUninitializer()
		{
			if (shouldUninitialize)
			{
				CoUninitialize();
			}
		}

		ScopedCOMUninitializer(const ScopedCOMUninitializer&) = delete;
		ScopedCOMUninitializer& operator=(const ScopedCOMUninitializer&) = delete;

	private:
		bool shouldUninitialize;
	};
}

bool ImageLoader::LoadPNG(const std::filesystem::path& path, Image& outImage)
{
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	// 다른 방식으로 초기화된 스레드에서는 기존 COM 환경을 사용한다.
	if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE)
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to initialize COM");
		return false;
	}

	// 이후 생성할 WIC 객체가 먼저 소멸하고 COM이 나중에 정리되도록 앞에 둔다.
	const ScopedCOMUninitializer comUninitializer(comResult);

	Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
	const HRESULT factoryResult = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(factoryResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to create WIC imaging factory");
		return false;
	}

	Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
	const HRESULT decoderResult = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
	if (FAILED(decoderResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to create image decoder from file");
		return false;
	}

	GUID containerFormat{};
	const HRESULT formatResult = decoder->GetContainerFormat(&containerFormat);
	if (FAILED(formatResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to get image container format");
		return false;
	}

	if (!IsEqualGUID(containerFormat, GUID_ContainerFormatPng))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Only PNG images are supported");
		return false;
	}

	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
	const HRESULT frameResult = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(frameResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to get PNG image frame");
		return false;
	}

	UINT width = 0;
	UINT height = 0;
	const HRESULT sizeResult = frame->GetSize(&width, &height);
	if (FAILED(sizeResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to get PNG image size");
		return false;
	}

	if (width == 0 || height == 0)
	{
		Log::Write(LogLevel::Error, "ImageLoader", "PNG image dimensions must be positive");
		return false;
	}

	Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
	const HRESULT converterResult = factory->CreateFormatConverter(converter.GetAddressOf());
	if (FAILED(converterResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to create image format converter");
		return false;
	}

	const HRESULT conversionResult = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(conversionResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to initialize RGBA8 conversion");
		return false;
	}

	// RGBA8은 픽셀당 4바이트다.
	const std::uint64_t rowPitch = static_cast<std::uint64_t>(width) * 4;
	if (rowPitch > UINT_MAX)
	{
		Log::Write(LogLevel::Error, "ImageLoader", "PNG row size exceeds the WIC buffer limit");
		return false;
	}

	const std::uint64_t byteCount = rowPitch * height;

	Image image;
	if (byteCount > UINT_MAX || byteCount > image.pixels.max_size())
	{
		Log::Write(LogLevel::Error, "ImageLoader", "PNG pixel data exceeds the supported buffer size");
		return false;
	}

	image.width = width;
	image.height = height;
	image.pixels.resize(static_cast<std::size_t>(byteCount));

	const HRESULT copyResult = converter->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(byteCount), reinterpret_cast<BYTE*>(image.pixels.data()));
	if (FAILED(copyResult))
	{
		Log::Write(LogLevel::Error, "ImageLoader", "Failed to copy PNG pixels");
		return false;
	}

	outImage = std::move(image);
	return true;
}
