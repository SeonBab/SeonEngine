#pragma once

#include "WindowsHWrapper.h"

#include <string>

class FInputState;

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
	/** 입력 상태를 비소유로 빌린다. 입력 객체는 이 객체보다 오래 살아 있어야 한다. */
	explicit FWindowsApplication(FInputState& input);

	FWindowsApplication(const FWindowsApplication&) = delete;
	FWindowsApplication& operator=(const FWindowsApplication&) = delete;

	/** 창을 준비·표시한다. 이미 자원이 있으면 거부하며 준비 실패 시 확보한 자원의 정리를 시도한다. */
	bool Initialize(const FWindowDescription& description);

	/** 스레드 큐를 처리한다. WM_QUIT이면 종료 코드를 기록하고 false, 빈 큐는 true다. */
	bool PumpMessages(int& outExitCode);

	/** 남은 창·클래스를 정리하고 성공 여부를 반환한다. 정리 전·후 반복 호출을 허용한다. */
	bool Shutdown();

	/** 창을 빌려 조회한다. 초기화 전에는 nullptr이며 파괴 후에는 사용하면 안 된다. */
	HWND GetWindowHandle() const { return window; }

	/** 클라이언트 영역의 픽셀 크기를 조회한다. 실패하면 로그를 남기고 출력 인자를 유지한다. */
	bool GetClientSize(int& outWidth, int& outHeight) const;

private:
	FInputState& inputState;
	HINSTANCE instance = nullptr;
	HWND window = nullptr;
	bool bClassRegistered = false;
};
