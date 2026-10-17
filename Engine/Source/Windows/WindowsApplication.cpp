#include "WindowsApplication.h"

#include "Logging/Log.h"
#include "WindowsHWrapper.h"

namespace
{
	constexpr wchar_t WindowClassName[] = L"SeonEngineWindow";

	LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			default:
				return DefWindowProcW(window, message, wParam, lParam);
		}
	}

	bool RegisterWindowClass(HINSTANCE instance, const wchar_t* className)
	{
		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
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

	if (!RegisterWindowClass(instance, WindowClassName))
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

	window = CreateWindowExW(windowExStyle, WindowClassName, description.title.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance, nullptr);
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

bool FWindowsApplication::PumpMessages(int& outExitCode)
{
	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
		{
			outExitCode = static_cast<int>(message.wParam);
			return false;
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
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
