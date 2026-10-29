#include "TextWidget.h"

#include "Core/Logging/Log.h"
#include "Renderer/Font.h"
#include "UI/UIDrawElementList.h"

#include <algorithm>
#include <utility>

#pragma region Font

bool UTextWidget::SetFont(TSharedPtr<UFont> newFont)
{
	if (!newFont)
	{
		WriteLog("UI", ELogLevel::Error, "Setting a text widget font requires a nonempty shared font.");
		return false;
	}

	font = std::move(newFont);

	return true;
}

#pragma endregion

#pragma region Text

void UTextWidget::SetText(FString newText)
{
	text = std::move(newText);
}

const FString& UTextWidget::GetText() const
{
	return text;
}

#pragma endregion

#pragma region Layout

void UTextWidget::SetPosition(FVector2 newPosition)
{
	position = newPosition;
}

FVector2 UTextWidget::GetPosition() const
{
	return position;
}

void UTextWidget::SetScale(float newScale)
{
	scale = newScale;
}

float UTextWidget::GetScale() const
{
	return scale;
}

#pragma endregion

#pragma region Appearance

void UTextWidget::SetColor(std::array<float, 4> newColor)
{
	color = newColor;
}

std::array<float, 4> UTextWidget::GetColor() const
{
	return color;
}

#pragma endregion

#pragma region Drawing

bool UTextWidget::CollectDrawElements(FUIDrawElementList& elements)
{
	if (font)
	{
		FUITextDrawElement element;
		element.font = font.get();
		element.text = text;
		element.position = position;
		std::copy(color.begin(), color.end(), element.color);
		element.scale = scale;
		elements.AddText(element);
	}

	return true;
}

#pragma endregion
