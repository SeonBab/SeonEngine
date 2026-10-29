#pragma once

#include "Editor/EditorUI.h"
#include "EditorViewportClient.h"
#include "Engine.h"

////////////////////////////////////////////////////////////////////////////////
// 단일 편집 시점을 보관하고 월드의 렌더 상태 전달과 프레임 그리기를 조율한다.
////////////////////////////////////////////////////////////////////////////////
class UEditorEngine : public UEngine
{
#pragma region Lifecycle

public:
	/** UI를 포함한 파생 자원이 남아 있으면 멤버 파괴 전에 종료 처리한다. */
	~UEditorEngine() override;

#pragma endregion

#pragma region EditorUI

public:
	/**
	 * 기존 창과 초기화된 Renderer가 만든 ImGui backend를 UI에 전달한다. UI가 backend를 소유하고 Renderer는 보관하지 않는다.
	 * 루프는 공통 초기화 뒤 같은 스레드에서 호출한다. 중복 요청은 기존 UI를 유지하고 거부한다.
	 * 새 UI 초기화의 bool 실패는 PreExit로 공통 서비스까지 정리한다.
	 */
	[[nodiscard]] bool InitializeEditorUI(HWND windowHandle, IRenderer& renderer);

	/** 초기화한 창의 메시지를 소유 UI로 전달한다. 같은 스레드에서 호출하며 true일 때만 outResult가 유효하다. */
	[[nodiscard]] bool HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

private:
	FEditorUI editorUI; // 이 에디터가 단독 소유한다. 빌린 창이 파괴되기 전에 PreExit에서 정리한다.

	bool bEditorUIInitialized = false; // 중복 초기화 거부와 새 UI 준비 실패의 정리를 구분한다.

#pragma endregion

#pragma region Execution

public:
	/** 공통 서비스와 editorStartupMap 및 렌더 상태를 준비한다. GameInstance·게임 카메라·UI·BeginPlay는 시작하지 않는다. 중복 초기화는 기존 상태를 보존하고 거부하며 새 초기화의 bool 실패는 PreExit로 정리한다. 에디터 UI는 이후 별도로 초기화한다. */
	[[nodiscard]] bool Initialize(IRenderer& renderer) override;

	/**
	 * 초기화된 World, UI와 양수 화면 크기로 렌더 상태를 갱신하고 UI 프레임 시작·패널 구성 뒤 Renderer 프레임을 시작한다.
	 * 자동 캡처 뒤 표시 공간이 있는 편집 장면을 소유 타겟에 그리고 창 출력을 복원한다.
	 * 이후 타겟을 Viewport 이미지로 추가하고 UI 프레임 종료·제출, Renderer 프레임 종료·Present 순서로 진행한다.
	 * 타겟은 UI 제출까지 유지한다. 패널이 접히면 편집 장면과 이미지 추가를 생략한다.
	 * 게임 플레이와 입력 평가는 하지 않으며 실패는 false다.
	 */
	[[nodiscard]] bool Tick(float deltaSeconds, const FInputState& input, IRenderer& renderer, int width, int height) override;

	/** 창이 살아 있을 때 UI → 뷰포트 GPU 타깃 → 기반 World/서비스 순서로 정리한다. 초기화 전·부분 실패·반복 호출을 허용하며 종료 콜백에서 재호출하지 않는다. */
	void PreExit() override;

private:
	FEditorViewportClient viewportClient; // 편집 카메라와 타겟을 소유한다. World를 빌려 보관하지 않으며 이 엔진 객체와 수명이 같다.

#pragma endregion
};
