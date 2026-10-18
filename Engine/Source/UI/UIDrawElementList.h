#pragma once

#include "Core/Containers/Array.h"
#include "UI/UIImageDrawElement.h"

////////////////////////////////////////////////////////////////////////////////
// UI 그리기 요소를 추가한 순서대로 보관한다.
// 요소의 텍스처는 소유하지 않으므로 사용하는 동안 참조 대상이 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class FUIDrawElementList
{
public:
	/** 이미지 요소를 복사해 목록 끝에 추가한다. 값이나 GPU 자원의 유효성은 검사하지 않는다. */
	void AddImage(const FUIImageDrawElement& element)
	{
		imageElements.push_back(element);
	}

	/** 보관된 이미지 요소를 읽는다. 반환 참조는 이 목록이 살아 있는 동안만 사용한다. */
	const TArray<FUIImageDrawElement>& GetImageElements() const
	{
		return imageElements;
	}

private:
	TArray<FUIImageDrawElement> imageElements;
};
