#include "WindowsApplication.h"

#include "Core/Log.h"
#include "Input/Key.h"

#include <cstdio>

namespace
{
	constexpr wchar_t WindowClassName[] = L"SeonEngineWindow";

	/** 다른 Windows 호출이 오류 번호를 바꾸기 전에 받은 값으로 출력한다. */
	void LogWindowsError(const char* operation, DWORD errorCode)
	{
		char message[160] = {};
		std::snprintf(message, sizeof(message), "%s failed. Windows error: %lu", operation, static_cast<unsigned long>(errorCode));
		Log::Write(LogLevel::Error, "WindowsApplication", message);
	}

	static_assert(static_cast<int>(Key::Z) - static_cast<int>(Key::A) == 'Z' - 'A', "A부터 Z까지 Key 값이 연속해야 한다.");
	static_assert(static_cast<int>(Key::Number9) - static_cast<int>(Key::Number0) == '9' - '0', "Number0부터 Number9까지 Key 값이 연속해야 한다.");

	/** 지원하지 않는 가상 키는 Key::Unknown으로 바꾼다. 왼쪽과 오른쪽 수정 키는 구분하지 않는다. */
	Key TranslateKey(WPARAM virtualKey)
	{
		// Windows의 문자와 숫자 가상 키 코드는 ASCII 대문자와 숫자 코드와 같다.
		if (virtualKey >= 'A' && virtualKey <= 'Z')
		{
			return static_cast<Key>(static_cast<int>(Key::A) + static_cast<int>(virtualKey - 'A'));
		}

		if (virtualKey >= '0' && virtualKey <= '9')
		{
			return static_cast<Key>(static_cast<int>(Key::Number0) + static_cast<int>(virtualKey - '0'));
		}

		switch (virtualKey)
		{
		case VK_SPACE:
			return Key::Space;
		case VK_RETURN:
			return Key::Enter;
		case VK_ESCAPE:
			return Key::Escape;
		case VK_TAB:
			return Key::Tab;
		case VK_SHIFT:
			return Key::Shift;
		case VK_CONTROL:
			return Key::Control;
		case VK_MENU:
			return Key::Alt;
		case VK_UP:
			return Key::Up;
		case VK_DOWN:
			return Key::Down;
		case VK_LEFT:
			return Key::Left;
		case VK_RIGHT:
			return Key::Right;
		default:
			return Key::Unknown;
		}
	}

	/**
	 * 키 메시지와 포커스 상실을 입력 상태에 기록한다. 그 밖의 메시지는 무시한다.
	 * Alt와 함께 누른 키는 WM_SYSKEYDOWN으로 들어오므로 일반 키 메시지와 같이 기록한다.
	 * 창 프로시저는 Windows가 호출하므로 예외가 그 밖으로 나가지 않게 noexcept로 둔다.
	 * 이벤트 목록 할당에 실패하면 std::terminate로 프로그램을 종료한다.
	 */
	void RecordInputMessage(InputState& inputState, UINT message, WPARAM wParam) noexcept
	{
		switch (message)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			inputState.PressKey(TranslateKey(wParam));
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			inputState.ReleaseKey(TranslateKey(wParam));
			break;
		case WM_KILLFOCUS:
			inputState.ReleaseAllKeys();
			break;
		default:
			break;
		}
	}
}

WindowsApplication::~WindowsApplication()
{
	Shutdown();
}

bool WindowsApplication::Initialize()
{
	if (closeRequested) { return false; }
	return Create();
}

bool WindowsApplication::Shutdown()
{
	closeRequested = true;
	if (window != nullptr)
	{
		if (!DestroyWindow(window))
		{
			LogWindowsError("DestroyWindow", GetLastError());
			return false;
		}
		// 초기화 중 객체 연결에 실패한 창도 파괴 성공 후 핸들을 비운다.
		window = nullptr;
	}

	if (isWindowClassRegistered)
	{
		if (!UnregisterClassW(WindowClassName, instance))
		{
			LogWindowsError("UnregisterClassW", GetLastError());
			return false;
		}
		isWindowClassRegistered = false;
	}

	return true;
}

bool WindowsApplication::Create()
{
	// ----------
	// 창 클래스 등록
	// ----------
	if (isWindowClassRegistered)
	{
		Log::Write(LogLevel::Error, "WindowsApplication", "Initialize failed: window class is already registered.");
		return false;
	}

	instance = GetModuleHandleW(nullptr);
	if (instance == nullptr)
	{
		LogWindowsError("GetModuleHandleW", GetLastError());
		return false;
	}

	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = WindowClassName;
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	if (RegisterClassExW(&windowClass) == 0)
	{
		LogWindowsError("RegisterClassExW", GetLastError());
		return false;
	}

	isWindowClassRegistered = true;

	// ----------
	// 창 생성
	// ----------
	const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	const DWORD windowExStyle = 0;

	const int windowWidth = 1280;
	const int windowHeight = 720;
	window = CreateWindowExW(windowExStyle, WindowClassName, L"SeonEngine", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance, nullptr);
	if (window == nullptr)
	{
		LogWindowsError("CreateWindowExW", GetLastError());
		return false;
	}

	// ----------
	// 창과 소유 객체 연결 및 표시
	// ----------
	// 창 생성이 끝난 뒤, 이후 메시지에서 소유 객체를 찾을 수 있도록 연결한다.
	SetLastError(ERROR_SUCCESS);
	const LONG_PTR previousData = SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	const DWORD connectionError = GetLastError();
	if (previousData == 0 && connectionError != ERROR_SUCCESS)
	{
		LogWindowsError("SetWindowLongPtrW", connectionError);
		// 연결되지 않은 창이므로 파괴 성공 시 여기서 직접 핸들을 비운다.
		if (DestroyWindow(window))
		{
			window = nullptr;
		}
		else
		{
			// 핸들을 유지해 소멸자에서 다시 정리한다.
			LogWindowsError("DestroyWindow", GetLastError());
		}
		return false;
	}

	ShowWindow(window, SW_SHOW);

	return true;
}

bool WindowsApplication::ProcessMessages(int& outExitCode)
{
	inputState.ClearFrameEvents();

	MSG message = {};

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

HWND WindowsApplication::GetWindowHandle() const
{
	return window;
}

bool WindowsApplication::GetClientSize(int& outWidth, int& outHeight) const
{
	if (window == nullptr) { return false; }

	RECT clientRect = {};
	if (!GetClientRect(window, &clientRect)) { return false; }

	outWidth = clientRect.right - clientRect.left;
	outHeight = clientRect.bottom - clientRect.top;
	return true;
}

bool WindowsApplication::IsMinimized() const
{
	return window != nullptr && IsIconic(window) != FALSE;
}

LRESULT WindowsApplication::WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 창 생성 중 객체 주소를 연결하기 전과 연결을 해제한 뒤의 메시지는 기록할 대상이 없다.
	WindowsApplication* windowsApplication = reinterpret_cast<WindowsApplication*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
	if (windowsApplication != nullptr)
	{
		// 기록 후에도 아래 DefWindowProcW로 넘겨 Alt+F4 같은 시스템 키 처리를 유지한다.
		RecordInputMessage(windowsApplication->inputState, message, wParam);
	}

	if (message == WM_CLOSE && windowsApplication != nullptr)
	{
		// 렌더러가 창을 사용하는 동안은 창을 유지하고 프레임 루프의 종료만 요청한다.
		if (!windowsApplication->closeRequested)
		{
			windowsApplication->closeRequested = true;
			PostQuitMessage(0);
		}
		return 0;
	}

	if (message == WM_DESTROY)
	{
		if (windowsApplication == nullptr || !windowsApplication->closeRequested)
		{
			PostQuitMessage(0);
		}

		return 0;
	}

	if (message == WM_NCDESTROY)
	{
		WindowsApplication* application = reinterpret_cast<WindowsApplication*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
		if (application != nullptr)
		{
			application->window = nullptr;

			SetWindowLongPtrW(windowHandle, GWLP_USERDATA, 0);
		}
	}

	return DefWindowProcW(windowHandle, message, wParam, lParam);
}

const InputState& WindowsApplication::GetInputState() const
{
	return inputState;
}
