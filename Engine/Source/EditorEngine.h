#pragma once

#include "Editor/EditorUI.h"
#include "EditorViewportClient.h"
#include "Engine.h"

////////////////////////////////////////////////////////////////////////////////
// 단일 편집 시점을 보관하고 월드의 렌더 상태 전달과 프레임 그리기를 조율한다.
////////////////////////////////////////////////////////////////////////////////
class UEditorEngine : public UEngine
{
#pragma region EditorUI

public:
	/** 기존 창을 빌려 편집 UI를 준비한다. 루프는 World 초기화 뒤 호출하고 실패 시 PreExit로 정리한다. */
	[[nodiscard]] bool InitializeEditorUI(HWND windowHandle);

	/** 초기화한 창의 메시지를 소유 UI로 전달한다. 같은 스레드에서 호출하며 true일 때만 outResult가 유효하다. */
	[[nodiscard]] bool HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

private:
	FEditorUI editorUI; // 이 에디터가 단독 소유한다. 빌린 창이 파괴되기 전에 PreExit에서 정리한다.

#pragma endregion

#pragma region Execution

public:
	/** 초기화된 World와 양수 화면 크기로 렌더 상태 전달과 편집 그리기를 수행한다. 게임 플레이와 입력 평가는 하지 않으며 실패는 false다. */
	[[nodiscard]] bool Tick(float deltaSeconds, const FInputState& input, FRenderer& renderer, int width, int height) override;

	/** 창이 살아 있을 때 UI와 World 순서로 해제한다. 초기화 전, 부분 실패 후, 반복 호출도 허용한다. */
	void PreExit() override;

private:
	FEditorViewportClient viewportClient; // 단일 편집 시점이다. World나 GPU 자원을 빌려 보관하지 않으며 이 엔진 객체와 수명이 같다.

#pragma endregion
};
