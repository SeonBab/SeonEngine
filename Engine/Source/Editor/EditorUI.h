#pragma once

#include "Windows/WindowsHWrapper.h"

struct ImGuiContext;

/** ImGui context와 Win32 백엔드를 관리한다. 같은 스레드에서 사용하며 외부에서 소유 context를 파괴하면 안 된다. */
class FEditorUI
{
public:
	FEditorUI() = default;
	/** Win32 백엔드와 소유 context를 정리한다. 명시적 Shutdown 뒤에도 안전하다. */
	~FEditorUI();

	FEditorUI(const FEditorUI&) = delete;
	FEditorUI& operator=(const FEditorUI&) = delete;
	FEditorUI(FEditorUI&&) = delete;
	FEditorUI& operator=(FEditorUI&&) = delete;

	/** 같은 스레드의 유효한 창을 빌려 context와 Win32 백엔드를 준비한다. 중복 초기화/기존 current context는 거부한다. */
	[[nodiscard]] bool Initialize(HWND windowHandle);

	/** 빌린 창이 살아 있을 때 Win32 백엔드와 소유 context를 순서대로 해제한다. 빈 상태/반복 호출을 허용한다. */
	void Shutdown();

private:
	ImGuiContext* context = nullptr; // CreateContext로 획득하고 DestroyContext로 해제하는 단독 소유다.
	bool bWin32Initialized = false;  // 백엔드 준비 실패 시 미초기화 백엔드의 종료 호출을 막는다.
};
