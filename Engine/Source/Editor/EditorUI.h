#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"
#include "Windows/WindowsHWrapper.h"

struct ImGuiContext;
class IImGuiRendererBackend;
class UTextureRenderTarget2D;

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
	 * 같은 스레드에서 GPU·Win32 backend와 ImGui 프레임을 시작하고 이전 context 선택을 복원한다.
	 * 미초기화와 중복 시작은 false다. 위젯 구성·GPU 제출·출력 선택은 하지 않는다.
	 * 성공 후 Build, 표시할 장면이 있으면 BuildViewportImage, EndFrame 순서로 호출한다.
	 * 중간 실패로 제출을 포기하면 Shutdown으로 열린 프레임을 정리한다.
	 */
	[[nodiscard]] bool BeginFrame();

	/**
	 * 시작된 프레임에서 확인용 창과 Viewport 패널을 구성하고 이전 context 선택을 복원한다. GPU 제출은 하지 않는다.
	 * 패널의 남은 표시 공간을 정수 크기로 내려 반환한다. 초기 크기는 640×360이며 이후 사용자 크기와 저장된 설정을 따른다.
	 * 출력은 먼저 0으로 비운다. 접힘·공간 부족·int32로 표현할 수 없는 크기는 둘 다 0이며 UI 구성 자체는 성공한다.
	 * 미초기화나 프레임 밖에서는 false다. 현재 Win32의 배율 1인 프레임버퍼를 전제로 UI 좌표 크기를 타깃 픽셀 크기로 사용한다.
	 */
	[[nodiscard]] bool Build(int32& outViewportWidth, int32& outViewportHeight);

	/**
	 * 같은 프레임의 Build로 측정한 크기에 맞춰 그린 타겟을 빌려 Viewport 패널에 이미지를 추가한다. GPU 제출은 하지 않는다.
	 * 호출 전 타겟 출력을 끝내고 EndFrame 반환까지 타겟과 GPU 데이터를 유지하며 교체하지 않는다. 이미지 참조는 보관하지 않는다.
	 * 미초기화·프레임 밖·빈 크기·이미지 참조 변환 실패는 false다. 접힌 패널은 이미지 없이 true다.
	 * 이전 ImGui context 선택을 복원한다. 같은 프레임에 한 번 호출하며 크기는 타겟 픽셀 크기와 동일한 UI 단위를 사용한다.
	 */
	[[nodiscard]] bool BuildViewportImage(const UTextureRenderTarget2D& renderTarget);

	/**
	 * 같은 스레드에서 시작된 UI 프레임을 끝내고 현재 연결된 유효한 출력 대상에 제출한다. 기존 ImGui context 선택은 복원한다.
	 * 호출자는 같은 장치의 출력 대상을 준비하고 이후 GPU 작업에 필요한 상태를 보장한다. 출력 대상 선택, 지우기와 Present는 하지 않는다.
	 * 미초기화나 프레임 밖에서는 false다. GPU 제출 실패 때도 UI 프레임은 이미 끝났으며 다시 시작할 수 있다.
	 * true는 호출 흐름 완료이며 GPU 작업 완료나 화면 표시 성공을 보장하지 않는다.
	 */
	[[nodiscard]] bool EndFrame();

	/** 빌린 창과 Renderer가 살아 있을 때 열린 UI 프레임을 제출 없이 끝낸 뒤 GPU backend·Win32·context 순서로 해제한다. 빈 상태와 반복 호출을 허용한다. */
	void Shutdown();

private:
	ImGuiContext* context = nullptr; // CreateContext로 획득하고 DestroyContext로 해제하는 단독 소유다.
	// context를 빌리므로 context보다 먼저 정리·파괴한다. 구체 backend가 자신의 부분 초기화 상태를 관리한다.
	TUniquePtr<IImGuiRendererBackend> rendererBackend;
	bool bWin32Initialized = false; // 준비 실패 시 미초기화 Win32 backend의 종료 호출을 막는다.
	bool bFrameStarted = false;     // BeginFrame 성공부터 EndFrame 또는 Shutdown까지 true이며 중복 시작을 막는다.
};
