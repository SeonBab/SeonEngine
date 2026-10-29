#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/Math/Vector2.h"
#include "Core/Templates/SharedPtr.h"
#include "Widget.h"

#include <array>

class UFont;

/** 문자열과 공유 폰트를 보관하고 텍스트 그리기 요청을 수집하는 UI 요소다. */
class UTextWidget : public UWidget
{
#pragma region Font

public:
	/**
	 * 폰트의 공유 참조를 저장한다. 입력 포인터는 유지하며 빈 입력은 로그/false와 기존 폰트 유지다.
	 * 렌더 데이터 준비 여부는 검사하지 않는다. 같은 스레드에서 수집부터 제출 완료까지
	 * 위젯과 폰트/렌더 데이터를 유지하며 폰트 교체나 렌더 데이터 교체/해제를 하지 않는다.
	 */
	[[nodiscard]] bool SetFont(TSharedPtr<UFont> newFont);

private:
	// 그리기 요청이 빌리는 폰트 자산의 수명을 유지한다. 기본 상태는 미지정이다.
	TSharedPtr<UFont> font;

#pragma endregion

#pragma region Text

public:
	/** UTF-8 문자열을 값으로 받아 소유한다. 중간 NUL/탭/인코딩 검사는 배처가 수행한다. */
	void SetText(FString newText);

	/** 문자열을 빌려 읽는다. 위젯 수명 안에서만 사용하며 문자 주소/뷰는 텍스트 변경 후 다시 얻어야 한다. */
	const FString& GetText() const;

private:
	FString text;

#pragma endregion

#pragma region Layout

public:
	/** 출력 영역 왼쪽 위 기준 첫 줄 펜의 픽셀 위치를 저장한다. 값 검사는 배처가 수행한다. */
	void SetPosition(FVector2 newPosition);

	/** 저장된 첫 줄 펜 위치의 복사본을 반환한다. */
	FVector2 GetPosition() const;

	/** 준비된 폰트 크기에 적용할 배율을 저장한다. 유한한 양수인지 검사는 배처가 수행한다. */
	void SetScale(float newScale);

	/** 저장된 배율을 반환한다. 1은 준비된 폰트의 원래 크기다. */
	float GetScale() const;

private:
	FVector2 position{};

	float scale = 1.0f;

#pragma endregion

#pragma region Appearance

public:
	/** Straight RGBA를 복사해 저장한다. 성분별 유한성/0~1 범위 검사는 배처가 수행한다. */
	void SetColor(std::array<float, 4> newColor);

	/** RGBA 순서의 색상 복사본을 반환한다. RGB에 알파를 미리 곱하지 않는다. */
	std::array<float, 4> GetColor() const;

private:
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};

#pragma endregion

#pragma region Drawing

public:
	/**
	 * 폰트가 있으면 현재 설정과 문자열 사본을 목록 끝에 추가한다. 폰트가 없으면 목록을 유지한다.
	 * 값/GPU 검증이나 그리기 없이 true를 반환한다. 목록은 폰트 수명을 연장하지 않으며 할당 예외는 전파한다.
	 */
	[[nodiscard]] bool CollectDrawElements(FUIDrawElementList& elements) override;

#pragma endregion
};
