#include "ImageWidget.h"

#include "Renderer/Renderer.h"

#include <utility>

bool UImageWidget::Draw(FRenderer& renderer)
{
	if (!texture) { return true; }

	return renderer.DrawImage(*texture, position, size);
}

bool UImageWidget::SetTexture(TUniquePtr<UTexture2D>& newTexture)
{
	if (!newTexture) { return false; }

	texture = std::move(newTexture);

	return true;
}

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
