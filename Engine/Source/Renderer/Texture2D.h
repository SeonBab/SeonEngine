#pragma once

#include "Object.h"
#include "Renderer/TextureAddressMode.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FRenderer;

/** 2D 텍스처를 읽는 GPU 뷰와 그 자원의 참조를 소유하고 X/Y축 기본 주소 설정을 보관한다. */
class UTexture2D : public UObject
{
	// GPU 뷰를 외부에 노출하지 않고 렌더러의 생성, 바인딩에만 사용한다.
	friend class FRenderer;

#pragma region Lifecycle

public:
	UTexture2D() = default;

	UTexture2D(const UTexture2D&) = delete;

	UTexture2D& operator=(const UTexture2D&) = delete;

	UTexture2D(UTexture2D&&) = default;

	UTexture2D& operator=(UTexture2D&&) = default;

#pragma endregion

#pragma region TextureResource

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

#pragma endregion

#pragma region Addressing

public:
	/** 텍스처 X축(U)의 기본 주소 설정을 값으로 반환한다. 현재 바인딩된 샘플러 상태를 조회하지 않는다. */
	ETextureAddress GetTextureAddressX() const { return AddressX; }

	/** 같은 스레드에서 X축(U) 기본 설정을 저장한다. 공유 텍스처의 모든 사용자가 같은 값을 보며 GPU 생성이나 샘플러 바인딩은 하지 않는다. */
	void SetTextureAddressX(ETextureAddress address)
	{
		AddressX = address;
	}

	/** 텍스처 Y축(V)의 기본 주소 설정을 값으로 반환한다. 현재 바인딩된 샘플러 상태를 조회하지 않는다. */
	ETextureAddress GetTextureAddressY() const { return AddressY; }

	/** 같은 스레드에서 Y축(V) 기본 설정을 저장한다. 공유 텍스처의 모든 사용자가 같은 값을 보며 GPU 생성이나 샘플러 바인딩은 하지 않는다. */
	void SetTextureAddressY(ETextureAddress address)
	{
		AddressY = address;
	}

private:
	// 텍스처 자체의 기본 설정이다. X는 샘플링 U축, Y는 V축에 대응한다.
	ETextureAddress AddressX = ETextureAddress::Clamp;

	ETextureAddress AddressY = ETextureAddress::Clamp;

#pragma endregion
};
