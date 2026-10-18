#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Templates/UniquePtr.h"
#include "Renderer/Texture2D.h"
#include "Widget.h"

/** 이미지 한 장을 표시하는 UI 요소의 기반이다. */
class UImageWidget : public UWidget
{
public:
	/** 텍스처가 있으면 현재 텍스처·위치·크기의 요소를 추가한다. GPU 검증·그리기 없이 true를 반환한다. */
	[[nodiscard]] bool CollectDrawElements(FUIDrawElementList& elements) override;

	/**
	 * 새 텍스처의 소유권을 받고 기존 텍스처 객체를 즉시 파괴한다. 성공하면 입력 포인터를 비운다.
	 * 빈 입력은 false이며 기존 텍스처를 유지한다. 교체 후 이전 텍스처의 포인터는 사용할 수 없다.
	 */
	[[nodiscard]] bool SetTexture(TUniquePtr<UTexture2D>& newTexture);

	/** 클라이언트 영역 왼쪽 위 기준 픽셀 위치를 복사해 저장한다. 좌표 변환이나 값 검사는 수행하지 않는다. */
	void SetPosition(FVector2 newPosition);

	/** 저장된 픽셀 위치의 복사본을 반환한다. */
	FVector2 GetPosition() const;

	/** 표시 너비·높이를 픽셀 단위로 복사해 저장한다. 값 검사는 수행하지 않는다. */
	void SetSize(FVector2 newSize);

	/** 저장된 표시 크기의 복사본을 반환한다. */
	FVector2 GetSize() const;

private:
	TUniquePtr<UTexture2D> texture;
	FVector2 position{};
	FVector2 size{};
};
