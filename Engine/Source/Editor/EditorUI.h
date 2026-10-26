#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Windows/WindowsHWrapper.h"

struct ImGuiContext;
class IImGuiRendererBackend;

/** ImGui context와 Win32, 선택된 렌더러의 GPU backend를 소유한다. 같은 스레드에서 사용하며 외부에서 context를 파괴하면 안 된다. */
class FEditorUI
{
public:
	FEditorUI();
	/** GPU, Win32 backend와 소유 context를 정리한다. 명시적 Shutdown 뒤에도 안전하다. */
	~FEditorUI();

	FEditorUI(const FEditorUI&) = delete;
	FEditorUI& operator=(const FEditorUI&) = delete;
	FEditorUI(FEditorUI&&) = delete;
	FEditorUI& operator=(FEditorUI&&) = delete;

	/**
	 * 같은 스레드의 유효한 창을 빌려 context와 Win32, 전달된 GPU backend를 준비한다. 중복 초기화와 기존 current context는 거부한다.
	 * backend의 소유권을 넘겨받으며 nullptr는 거부한다. 원본 Renderer와 창은 이 UI의 Shutdown까지 유지해야 한다.
	 */
	[[nodiscard]] bool Initialize(HWND windowHandle, TUniquePtr<IImGuiRendererBackend> newRendererBackend);

	/** 초기화한 창의 메시지를 같은 스레드에서 전달한다. true일 때만 outResult가 유효하다. */
	[[nodiscard]] bool HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	/**
	 * 초기화 후 같은 스레드에서 UI 한 프레임을 구성하고 현재 연결된 유효한 출력 대상에 제출한다. 기존 ImGui context 선택은 복원한다.
	 * 호출자는 같은 장치의 출력 대상을 준비하고 이후 GPU 작업에 필요한 상태를 보장한다. 출력 대상 선택, 지우기와 Present는 하지 않는다.
	 * 미초기화면 false다. true는 호출 흐름 완료이며 GPU 작업 완료나 화면 표시 성공을 보장하지 않는다.
	 */
	[[nodiscard]] bool Render();

	/** 빌린 창과 Renderer가 살아 있을 때 GPU backend 정리·파괴, Win32 정리, context 파괴 순서로 해제한다. 빈 상태와 반복 호출을 허용한다. */
	void Shutdown();

private:
	ImGuiContext* context = nullptr; // CreateContext로 획득하고 DestroyContext로 해제하는 단독 소유다.
	// context를 빌리므로 context보다 먼저 정리·파괴한다. 구체 backend가 자신의 부분 초기화 상태를 관리한다.
	TUniquePtr<IImGuiRendererBackend> rendererBackend;
	bool bWin32Initialized = false; // 준비 실패 시 미초기화 Win32 backend의 종료 호출을 막는다.
};
