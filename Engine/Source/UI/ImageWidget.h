#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Templates/SharedPtr.h"
#include "Renderer/Texture.h"
#include "Widget.h"

/** 이미지 한 장을 표시하는 UI 요소의 기반이다. */
class UImageWidget : public UWidget
{
#pragma region Texture

public:
	/**
	 * 새 텍스처의 공유 참조를 저장한다. 입력 포인터는 유지하며 이전 텍스처는 마지막 공유 참조가 해제될 때 파괴된다.
	 * 빈 입력은 false이며 기존 텍스처를 유지한다. 수집한 그리기 요소의 사용이 끝나기 전에는 텍스처를 교체하지 않는다.
	 */
	[[nodiscard]] bool SetTexture(TSharedPtr<UTexture> newTexture);

private:
	// 그리기 요소가 빌리는 텍스처 객체의 수명을 유지한다.
	TSharedPtr<UTexture> texture;

#pragma endregion

#pragma region Layout

public:
	/** 클라이언트 영역 왼쪽 위 기준 픽셀 위치를 복사해 저장한다. 좌표 변환이나 값 검사는 수행하지 않는다. */
	void SetPosition(FVector2 newPosition);

	/** 저장된 픽셀 위치의 복사본을 반환한다. */
	FVector2 GetPosition() const;

	/** 표시 너비, 높이를 픽셀 단위로 복사해 저장한다. 값 검사는 수행하지 않는다. */
	void SetSize(FVector2 newSize);

	/** 저장된 표시 크기의 복사본을 반환한다. */
	FVector2 GetSize() const;

private:
	FVector2 position{};

	FVector2 size{};

#pragma endregion

#pragma region Drawing

public:
	/** 텍스처가 있으면 현재 텍스처와 위치, 크기를 담은 요소를 추가한다. GPU 검증이나 그리기 없이 true를 반환한다. */
	[[nodiscard]] bool CollectDrawElements(FUIDrawElementList& elements) override;

#pragma endregion
};
