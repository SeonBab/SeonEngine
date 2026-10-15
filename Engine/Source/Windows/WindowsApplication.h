#pragma once

#include "WindowsHWrapper.h"

#include <string>

/** 창 제목과 요청할 클라이언트 영역의 픽셀 크기다. 크기는 유효한 양수로 지정한다. */
struct FWindowDescription
{
	std::wstring title = L"SeonEngine";
	int width = 1280;
	int height = 720;
};

/** 단일 창의 수명을 관리한다. 같은 스레드에서 사용하고 종료 전에 Shutdown을 명시적으로 호출한다. */
class FWindowsApplication
{
public:
	FWindowsApplication() = default;

	FWindowsApplication(const FWindowsApplication&) = delete;
	FWindowsApplication& operator=(const FWindowsApplication&) = delete;

	/** 창을 준비·표시한다. 이미 자원이 있으면 거부하며 준비 실패 시 확보한 자원의 정리를 시도한다. */
	bool Initialize(const FWindowDescription& description);

	/** 스레드 큐를 처리한다. WM_QUIT이면 종료 코드를 기록하고 false, 빈 큐는 true다. */
	bool PumpMessages(int& outExitCode);

	/** 남은 창·클래스를 정리하고 성공 여부를 반환한다. 정리 전·후 반복 호출을 허용한다. */
	bool Shutdown();

private:
	HINSTANCE instance = nullptr;
	HWND window = nullptr;
	bool bClassRegistered = false;
};

/** 초기화·메시지 대기·정리를 조합한다. 정상 종료 코드는 WM_QUIT, API/정리 실패는 1이다. */
int RunWindowsApplication(const FWindowDescription& description);
