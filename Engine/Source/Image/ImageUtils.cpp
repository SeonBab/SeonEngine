#include "ImageUtils.h"

#include "Core/Logging/Log.h"
#include "Windows/WindowsHWrapper.h"

#include <wincodec.h>
#include <wrl/client.h>

#include <limits>
#include <utility>

namespace
{
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
