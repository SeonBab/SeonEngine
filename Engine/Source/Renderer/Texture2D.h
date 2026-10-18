#pragma once

#include "Object.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FRenderer;

/** 2D 텍스처를 읽는 GPU 뷰와 그 자원의 참조를 소유한다. */
class UTexture2D : public UObject
{
	// GPU 뷰를 외부에 노출하지 않고 렌더러의 생성·바인딩에만 사용한다.
	friend class FRenderer;

public:
	UTexture2D() = default;

	UTexture2D(const UTexture2D&) = delete;
	UTexture2D& operator=(const UTexture2D&) = delete;
	UTexture2D(UTexture2D&&) = default;
	UTexture2D& operator=(UTexture2D&&) = default;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
};
