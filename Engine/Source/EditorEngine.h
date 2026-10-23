#pragma once

#include "EditorViewportClient.h"
#include "Engine.h"

////////////////////////////////////////////////////////////////////////////////
// 단일 편집 시점을 보관하고 월드의 렌더 상태 전달과 프레임 그리기를 조율한다.
////////////////////////////////////////////////////////////////////////////////
class UEditorEngine : public UEngine
{
public:
	/** 초기화된 World와 양수 화면 크기로 렌더 상태 전달과 편집 그리기를 수행한다. 게임 플레이와 입력 평가는 하지 않으며 실패는 false다. */
	[[nodiscard]] bool Tick(float deltaSeconds, const FInputState& input, FRenderer& renderer, int width, int height) override;

	/** 루프가 프레임 처리 밖에서 편집 World를 해제한다. 초기화 전과 반복 호출도 허용한다. */
	void PreExit() override;

private:
	FEditorViewportClient viewportClient; // 단일 편집 시점이다. World나 GPU 자원을 빌려 보관하지 않으며 이 엔진 객체와 수명이 같다.
};
