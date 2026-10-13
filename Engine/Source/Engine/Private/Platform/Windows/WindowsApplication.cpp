#include "Platform/Windows/WindowsApplication.h"

#include "Platform/GenericPlatform/GenericApplicationMessageHandler.h"
#include "Platform/Windows/WindowsHWrapper.h"
#include "Platform/Windows/WindowsWindow.h"

#include <algorithm>

namespace
{
	/** 등록된 창 중 OS 핸들이 같은 객체를 반환한다. 빈 핸들 / 미등록 창은 빈 포인터다. */
	TSharedPtr<FWindowsWindow> FindWindowByHWND(const TArray<TSharedPtr<FWindowsWindow>>& windowsToSearch, HWND handleToFind)
	{
		if (handleToFind == nullptr) { return nullptr; }

		for (const TSharedPtr<FWindowsWindow>& window : windowsToSearch)
		{
			if (window->GetOSWindowHandle() == handleToFind)
			{
				return window;
			}
		}

		return nullptr;
	}
}

TSharedPtr<FGenericWindow> FWindowsApplication::MakeWindow()
{
	return MakeShared<FWindowsWindow>(*this);
}

void FWindowsApplication::InitializeWindow(const TSharedPtr<FGenericWindow>& window)
{
	// TODO(seon): 어설션 도입 시 필수 창 인수와 초기화 실패의 처리 정책을 적용한다.
	if (!window) { return; }

	TSharedPtr<FWindowsWindow> nativeWindow = std::dynamic_pointer_cast<FWindowsWindow>(window);
	if (!nativeWindow) { return; }

	if (std::find(windows.begin(), windows.end(), nativeWindow) != windows.end()) { return; }

	windows.push_back(nativeWindow);
	if (!nativeWindow->Initialize())
	{
		std::erase(windows, nativeWindow);
		return;
	}
}

void FWindowsApplication::PumpMessages([[maybe_unused]] float TimeDelta)
{
	MSG message{};
	// 창 필터 없이 이 스레드의 창 메시지와 스레드 메시지를 꺼낸다.
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

LRESULT FWindowsApplication::ProcessMessage(HWND hwnd, uint32 message, WPARAM wParam, LPARAM lParam)
{
	TSharedPtr<FWindowsWindow> window = FindWindowByHWND(windows, hwnd);
	if (!window)
	{
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	if (message == WM_KEYDOWN || message == WM_KEYUP)
	{
		// 호출 중 수신자가 교체돼도 현재 수신자의 객체 수명을 유지한다.
		TSharedPtr<FGenericApplicationMessageHandler> handler = messageHandler;
		const int32 keyCode = static_cast<int32>(wParam);
		const uint32 characterCode = MapVirtualKeyW(static_cast<uint32>(wParam), MAPVK_VK_TO_CHAR);

		if (message == WM_KEYDOWN)
		{
			const bool bIsRepeat = (lParam & (1LL << 30)) != 0;
			handler->OnKeyDown(keyCode, characterCode, bIsRepeat);
		}
		else
		{
			handler->OnKeyUp(keyCode, characterCode, false);
		}

		// 일반 키 메시지의 OS 반환값은 수신자의 이벤트 처리 여부와 별개다.
		return 0;
	}

	if (message == WM_SIZE)
	{
		window->UpdateClientSize(static_cast<uint32>(LOWORD(lParam)), static_cast<uint32>(HIWORD(lParam)), wParam == SIZE_MINIMIZED);
		return 0;
	}

	if (message == WM_CLOSE)
	{
		window->RequestClose();
		return 0;
	}

	return DefWindowProcW(hwnd, message, wParam, lParam);
}
