#include "ImageWidget.h"

#include "UI/UIDrawElementList.h"

#include <utility>

#pragma region Texture

bool UImageWidget::SetTexture(TSharedPtr<UTexture> newTexture)
{
	if (!newTexture) { return false; }

	texture = std::move(newTexture);

	return true;
}

#pragma endregion

#pragma region Layout

void UImageWidget::SetPosition(FVector2 newPosition)
{
	position = newPosition;
}

FVector2 UImageWidget::GetPosition() const
{
	return position;
}

void UImageWidget::SetSize(FVector2 newSize)
{
	size = newSize;
}

FVector2 UImageWidget::GetSize() const
{
	return size;
}

#pragma endregion

#pragma region Drawing

bool UImageWidget::CollectDrawElements(FUIDrawElementList& elements)
{
	if (texture)
	{
		elements.AddImage({texture.get(), position, size});
	}

	return true;
}

#pragma endregion
