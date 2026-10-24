#pragma once

#include "WindowsHWrapper.h"

#include <functional>
#include <string>

class FInputState;

/** true면 일반 메시지의 outResult를 사용한다. false면 기본 처리하며 포커스·창 수명의 필수 처리는 항상 유지한다. */
using FWindowsMessageHandler = std::function<bool(HWND window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)>;

/** 창 제목과 요청할 클라이언트 영역의 픽셀 크기다. 크기는 유효한 양수로 지정한다. */
struct FWindowDescription
{
	std::wstring title = L"SeonEngine";
	int width = 1280;
	int height = 720;
};

/** 단일 창의 수명을 관리한다. 같은 스레드에서 사용하고 객체가 살아 있을 때 Shutdown으로 창을 먼저 해제한다. */
class FWindowsApplication
{
public:
	/** 입력 상태를 비소유로 빌린다. 입력 객체는 이 객체보다 오래 살아 있어야 한다. */
	explicit FWindowsApplication(FInputState& input);

	FWindowsApplication(const FWindowsApplication&) = delete;
	FWindowsApplication& operator=(const FWindowsApplication&) = delete;

	/**
	 * 메시지 처리자를 등록하거나 교체한다. 캡처한 참조 대상의 수명은 호출자가 보장하며 처리자 실행 중에는 호출하지 않는다.
	 * WM_NCCREATE를 제외한 생성·파괴 메시지도 전달한다. 생성 중에는 GetWindowHandle 대신 전달된 HWND를 사용한다.
	 */
	void SetMessageHandler(FWindowsMessageHandler handler);

	/** 등록된 메시지 처리자를 해제한다. 빈 상태에서도 호출할 수 있으며 처리자 실행 중에는 호출하지 않는다. */
	void ClearMessageHandler();

	/** 창을 준비·표시한다. 이미 자원이 있으면 거부하며 준비 실패 시 확보한 자원의 정리를 시도한다. */
	bool Initialize(const FWindowDescription& description);

	/** 스레드 큐를 처리한다. WM_QUIT이면 종료 코드를 기록하고 false, 빈 큐는 true다. */
	bool PumpMessages(int& outExitCode);

	/** 남은 창·클래스를 정리하고 성공 여부를 반환한다. 정리 전·후 반복 호출을 허용하며 처리자 실행 중에는 호출하지 않는다. */
	bool Shutdown();

	/** 창을 빌려 조회한다. 초기화 전에는 nullptr이며 파괴 후에는 사용하면 안 된다. */
	HWND GetWindowHandle() const { return window; }

	/** 클라이언트 영역의 픽셀 크기를 조회한다. 실패하면 로그를 남기고 출력 인자를 유지한다. */
	bool GetClientSize(int& outWidth, int& outHeight) const;

private:
	/** 생성 연결 뒤 외부 처리자를 호출한다. 포커스·종료·연결 해제는 외부 처리 여부와 무관하게 수행한다. */
	static LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

	FInputState& inputState;
	FWindowsMessageHandler messageHandler; // 선택적인 메시지 처리자다. 비어 있으면 외부 처리를 호출하지 않는다.
	HINSTANCE instance = nullptr;
	HWND window = nullptr;
	bool bClassRegistered = false;
};
