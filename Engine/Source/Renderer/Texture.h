#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Object.h"
#include "Renderer/TextureAddressMode.h"
class FTextureRenderData;

/** 렌더러별 텍스처 데이터의 단독 소유권과 X/Y축 기본 주소 설정을 보관한다. */
class UTexture : public UObject
{
#pragma region Lifecycle

public:
	UTexture();

	~UTexture() override;

	UTexture(const UTexture&) = delete;

	UTexture& operator=(const UTexture&) = delete;

	UTexture(UTexture&&) noexcept;

	UTexture& operator=(UTexture&&) noexcept;

#pragma endregion

#pragma region TextureResource

public:
	/** 미지정이면 nullptr다. 반환 주소는 자산 수명 안에서만 빌리며 데이터 교체/비움 뒤에는 다시 조회해야 한다. */
	const FTextureRenderData* GetRenderData() const;

	/**
	 * 같은 스레드에서 렌더 데이터의 단독 소유권을 넘겨받는다. 이동 후 입력은 비며 nullptr는 비움이다.
	 * 교체 전 기존 데이터의 비소유 사용과 활성 출력 사용을 끝내고 이전 조회 주소는 재사용하지 않는다.
	 * 주소 설정은 유지한다. GPU 생성/유효성 검사, 구현 호환성 검사와 GPU 완료 대기는 수행하지 않는다.
	 */
	void SetRenderData(TUniquePtr<FTextureRenderData>&& newRenderData);

private:
	// 자원이 미지정이면 nullptr다. 실제 GPU 준비 여부는 사용하는 렌더러가 검사한다.
	TUniquePtr<FTextureRenderData> renderData;

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
