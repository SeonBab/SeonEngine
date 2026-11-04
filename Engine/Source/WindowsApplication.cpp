#include "WindowsApplication.h"

bool WindowsApplication::Initialize()
{
	return Create();
}

bool WindowsApplication::Create()
{
	HINSTANCE instance = GetModuleHandleW(nullptr);
	constexpr wchar_t WindowClassName[] = L"SeonEngineWindow";

	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = WindowClassName;
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	if (RegisterClassExW(&windowClass) == 0)
	{
		// @TODO 실패 로그
		return false;
	}

	const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	const DWORD windowExStyle = 0;

	const int windowWidth = 1280;
	const int windowHeight = 720;
	window = CreateWindowExW(windowExStyle, WindowClassName, L"SeonEngine", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance, this);
	if (window == nullptr)
	{
		// @TODO 실패 로그
		return false;
	}

	ShowWindow(window, SW_SHOW);

	return true;
}

LRESULT WindowsApplication::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(window, message, wParam, lParam);
}
