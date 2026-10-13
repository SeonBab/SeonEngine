#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Platform/GenericPlatform/GenericApplication.h"
#include "Platform/Windows/WindowsHWrapper.h"

class FWindowsWindow;

namespace SE::Private
{
	struct FWindowsWindowProc;
}

////////////////////////////////////////////////////////////////////////////////
// Windows 애플리케이션의 메시지 처리 주체다. Win32 타입은 Private 플랫폼 계층에 한정한다.
////////////////////////////////////////////////////////////////////////////////
class FWindowsApplication final : public FGenericApplication
{
	friend struct SE::Private::FWindowsWindowProc;

public:
	FWindowsApplication() = default;
	~FWindowsApplication() override = default;

	TSharedPtr<FGenericWindow> MakeWindow() override;
	void InitializeWindow(const TSharedPtr<FGenericWindow>& window) override;

	void PumpMessages(float TimeDelta) override;

private:
	/** Windows가 호출하는 창 메시지 진입점이다. */
	static LRESULT CALLBACK AppWndProc(HWND hwnd, uint32 message, WPARAM wParam, LPARAM lParam);

	/** HWND로 등록된 창을 찾아 Windows 메시지를 처리하고 OS에 전달할 결과를 반환한다. */
	LRESULT ProcessMessage(HWND hwnd, uint32 message, WPARAM wParam, LPARAM lParam);

	/** 등록된 창 객체를 공유 소유한다. OS 창은 명시적으로 종료한다. */
	TArray<TSharedPtr<FWindowsWindow>> windows;
};
