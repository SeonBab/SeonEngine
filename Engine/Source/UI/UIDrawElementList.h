#pragma once

#include "Core/Containers/Array.h"
#include "UI/UIImageDrawElement.h"
#include "UI/UITextDrawElement.h"

#include <variant>

/** 목록 한 항목은 이미지 또는 텍스트 요청 하나를 값으로 소유한다. */
using FUIDrawElement = std::variant<FUIImageDrawElement, FUITextDrawElement>;

////////////////////////////////////////////////////////////////////////////////
// UI 그리기 요소를 추가한 순서대로 보관한다.
// 문자열은 소유하지만 텍스처/폰트는 빌린다. 수집부터 동기식 제출 완료까지 자산과 렌더 데이터를 유지·교체 금지한다.
////////////////////////////////////////////////////////////////////////////////
class FUIDrawElementList
{
public:
	/** 이미지 요소를 복사해 목록 끝에 추가한다. 값이나 GPU 자원의 유효성은 검사하지 않는다. */
	void AddImage(const FUIImageDrawElement& element)
	{
		elements.emplace_back(element);
	}

	/** 문자열을 포함한 텍스트 요소를 복사해 목록 끝에 추가한다. 값이나 GPU 자원의 유효성은 검사하지 않는다. */
	void AddText(const FUITextDrawElement& element)
	{
		elements.emplace_back(element);
	}

	/** 추가 순서대로 읽는다. 배열 참조는 목록 수명 안에서만, 항목 참조는 목록 수정 전까지만 사용한다. */
	const TArray<FUIDrawElement>& GetElements() const
	{
		return elements;
	}

private:
	TArray<FUIDrawElement> elements;
};
