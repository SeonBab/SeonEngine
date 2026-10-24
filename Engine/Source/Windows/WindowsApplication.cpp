#include "WindowsApplication.h"

#include "Core/Logging/Log.h"
#include "Input/InputState.h"
#include "Windows/WindowsInput.h"
#include "WindowsHWrapper.h"

#include <utility>

namespace
{
	constexpr wchar_t WindowClassName[] = L"SeonEngineWindow";

	bool RegisterWindowClass(HINSTANCE instance, const wchar_t* className, WNDPROC windowProc)
	{
		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = windowProc;
		windowClass.hInstance = instance;
		windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		if (windowClass.hCursor == nullptr)
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Windows", ELogLevel::Error, "LoadCursorW failed (error {})", errorCode);
			return false;
		}

		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(COLOR_WINDOW + 1));
		windowClass.lpszClassName = className;

		if (RegisterClassExW(&windowClass) == 0)
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Windows", ELogLevel::Error, "RegisterClassExW failed (error {})", errorCode);
			return false;
		}

		return true;
	}

}

#pragma region Lifecycle

FWindowsApplication::FWindowsApplication(FInputState& input)
	: inputState(input)
{
}

#pragma endregion

#pragma region Messages

void FWindowsApplication::SetMessageHandler(FWindowsMessageHandler handler)
{
	messageHandler = std::move(handler);
}

void FWindowsApplication::ClearMessageHandler()
{
	messageHandler = nullptr;
}

bool FWindowsApplication::PumpMessages(int& outExitCode)
{
	inputState.ResetFrameState();

	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
		{
			outExitCode = static_cast<int>(message.wParam);
			return false;
		}

		if (message.hwnd == window)
		{
			if (message.message == WM_KEYDOWN)
			{
				const bool bIsRepeat = (message.lParam & (static_cast<LPARAM>(1) << 30)) != 0;
				const EKeyEventType type = bIsRepeat ? EKeyEventType::Repeat : EKeyEventType::Pressed;
				inputState.AddKeyEvent({TranslateWindowsKey(message.wParam), type});
			}
			else if (message.message == WM_KEYUP)
			{
				inputState.AddKeyEvent({TranslateWindowsKey(message.wParam), EKeyEventType::Released});
			}
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
	return true;
}

LRESULT CALLBACK FWindowsApplication::WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NCCREATE)
	{
		const CREATESTRUCTW* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
		// CreateWindowExW가 반환하기 전부터 메시지가 오므로 생성 메시지에서 객체를 연결한다.
		FWindowsApplication* application = static_cast<FWindowsApplication*>(create->lpCreateParams);
		if (application == nullptr) { return FALSE; }

		SetLastError(ERROR_SUCCESS);
		const LONG_PTR previous = SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
		if (previous == 0 && GetLastError() != ERROR_SUCCESS) { return FALSE; }

		return TRUE;
	}

	FWindowsApplication* application = reinterpret_cast<FWindowsApplication*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
	LRESULT handlerResult = 0;
	bool bHandled = false;
	if (application != nullptr && application->messageHandler)
	{
		bHandled = application->messageHandler(windowHandle, message, wParam, lParam, handlerResult);
	}

	// 외부 처리자가 true를 반환해도 창 수명과 입력 초기화의 필수 처리는 유지한다.
	switch (message)
	{
		case WM_KILLFOCUS:
		{
			if (application != nullptr)
			{
				application->inputState.Reset();
			}
			return 0;
		}

		case WM_NCDESTROY:
			SetWindowLongPtrW(windowHandle, GWLP_USERDATA, 0);
			return DefWindowProcW(windowHandle, message, wParam, lParam);

		case WM_CLOSE:
			// 창 파괴는 루프 정리에 맡겨 UI와 Renderer가 빌린 창보다 먼저 종료되게 한다.
			PostQuitMessage(0);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			if (bHandled) { return handlerResult; }
			return DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

#pragma endregion

#pragma region Window

bool FWindowsApplication::Initialize(const FWindowDescription& description)
{
	if (window != nullptr || bClassRegistered)
	{
		WriteLog("Windows", ELogLevel::Error, "Windows application still owns window resources.");
		return false;
	}

	instance = GetModuleHandleW(nullptr);
	if (instance == nullptr)
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Windows", ELogLevel::Error, "GetModuleHandleW failed (error {})", errorCode);
		return false;
	}

	if (!RegisterWindowClass(instance, WindowClassName, WindowProc))
	{
		return false;
	}
	bClassRegistered = true;

	const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	const DWORD windowExStyle = 0;
	RECT windowRectangle{0, 0, static_cast<LONG>(description.width), static_cast<LONG>(description.height)};
	if (!AdjustWindowRectEx(&windowRectangle, windowStyle, FALSE, windowExStyle))
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Windows", ELogLevel::Error, "AdjustWindowRectEx failed (error {})", errorCode);
		Shutdown();
		return false;
	}

	const int windowWidth = static_cast<int>(windowRectangle.right - windowRectangle.left);
	const int windowHeight = static_cast<int>(windowRectangle.bottom - windowRectangle.top);
	WriteLog("Windows", ELogLevel::Info, "Requested client area: {} x {}; window size: {} x {}", description.width, description.height, windowWidth, windowHeight);

	window = CreateWindowExW(windowExStyle, WindowClassName, description.title.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance, this);
	if (window == nullptr)
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Windows", ELogLevel::Error, "CreateWindowExW failed (error {})", errorCode);
		Shutdown();
		return false;
	}

	RECT clientRectangle{};
	if (!GetClientRect(window, &clientRectangle))
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Windows", ELogLevel::Error, "GetClientRect failed (error {})", errorCode);
		Shutdown();
		return false;
	}

	const int clientWidth = static_cast<int>(clientRectangle.right - clientRectangle.left);
	const int clientHeight = static_cast<int>(clientRectangle.bottom - clientRectangle.top);
	WriteLog("Windows", ELogLevel::Info, "Actual client area: {} x {}", clientWidth, clientHeight);
	ShowWindow(window, SW_SHOW);
	return true;
}

bool FWindowsApplication::Shutdown()
{
	bool bSuccess = true;
	if (window != nullptr)
	{
		if (IsWindow(window) && !DestroyWindow(window))
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Windows", ELogLevel::Error, "DestroyWindow failed (error {})", errorCode);
			bSuccess = false;
		}
		else
		{
			window = nullptr;
		}
	}

	if (bClassRegistered)
	{
		if (UnregisterClassW(WindowClassName, instance) == 0)
		{
			const DWORD errorCode = GetLastError();
			WriteLog("Windows", ELogLevel::Error, "UnregisterClassW failed (error {})", errorCode);
			bSuccess = false;
		}
		else
		{
			bClassRegistered = false;
		}
	}

	if (window == nullptr && !bClassRegistered)
	{
		instance = nullptr;
	}
	return bSuccess;
}

bool FWindowsApplication::GetClientSize(int& outWidth, int& outHeight) const
{
	RECT clientRectangle{};
	if (!GetClientRect(window, &clientRectangle))
	{
		const DWORD errorCode = GetLastError();
		WriteLog("Windows", ELogLevel::Error, "GetClientRect failed (error {})", errorCode);
		return false;
	}

	outWidth = static_cast<int>(clientRectangle.right - clientRectangle.left);
	outHeight = static_cast<int>(clientRectangle.bottom - clientRectangle.top);

	return true;
}

#pragma endregion
