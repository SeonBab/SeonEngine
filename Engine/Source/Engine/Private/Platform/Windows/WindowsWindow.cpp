#include "Platform/Window.h"

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHeaders.h"

// Window.h에 선언한 플랫폼 창의 Windows 구현이다. Win32 호출은 이 파일에만 둔다.

namespace
{
	// 창 클래스 등록, 창 생성, 해제에 같은 이름을 써야 한다
	constexpr const wchar_t* WindowClassName = L"SeonEngineWindow";
	constexpr const wchar_t* WindowTitle     = L"SeonEngine";

	// 창 크기를 계산할 때와 창을 만들 때 같은 스타일을 써야 한다. 다르면 클라이언트 영역 크기가 틀어진다
	constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
	// 메인 창 하나라 특별한 동작(작업 표시줄 숨김, 항상 위 등)이 필요 없다
	constexpr DWORD WindowExStyle = 0;

	// 클라이언트 영역(테두리와 제목 표시줄을 뺀, 그림이 그려지는 영역)의 크기다
	constexpr int32 WindowWidth  = 1280;
	constexpr int32 WindowHeight = 720;
}

bool FWindow::Initialize()
{
	WNDCLASSEXW windowClass   = {};
	windowClass.cbSize        = sizeof(windowClass);
	windowClass.lpszClassName = WindowClassName;
	// 아직 직접 처리할 메시지가 없으므로 Windows 기본 처리 함수를 그대로 쓴다
	windowClass.lpfnWndProc = DefWindowProcW;
	// 창 프로시저가 든 모듈이 실행 파일이라 실행 파일의 인스턴스를 쓴다. 엔진을 DLL로 나누면 다시 정한다
	windowClass.hInstance = GetModuleHandleW(nullptr);
	// 비워 두면 클라이언트 영역에 들어온 커서가 직전 모양(테두리의 크기 조절 화살표 등) 그대로 남는다
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	if (RegisterClassExW(&windowClass) == 0)
	{
		return false;
	}

	// 원하는 클라이언트 영역에 테두리와 제목 표시줄을 더해 CreateWindowExW에 넘길 바깥 크기를 구한다
	RECT windowRect = {0, 0, WindowWidth, WindowHeight};
	AdjustWindowRectEx(&windowRect, WindowStyle, FALSE, WindowExStyle);

	HWND hwnd = CreateWindowExW(
		WindowExStyle,                      // 확장 스타일
		WindowClassName,                    // 등록한 창 클래스
		WindowTitle,                        // 제목
		WindowStyle,                        // 스타일
		CW_USEDEFAULT,                      // x: Windows가 정함
		CW_USEDEFAULT,                      // y: x가 CW_USEDEFAULT면 무시됨
		windowRect.right - windowRect.left, // 바깥 너비
		windowRect.bottom - windowRect.top, // 바깥 높이
		nullptr,                            // 부모 창 없음
		nullptr,                            // 메뉴 없음
		GetModuleHandleW(nullptr),          // 등록할 때와 같은 인스턴스
		nullptr);                           // 창 프로시저에 넘길 값
	if (hwnd == nullptr)
	{
		return false;
	}

	nativeHandle = hwnd;

	// 첫 호출은 실행한 쪽이 넘긴 시작 정보(바로가기의 "최소화로 실행" 등)가 있으면 그것을 따른다
	ShowWindow(hwnd, SW_SHOW);
	return true;
}

void FWindow::Shutdown()
{
	// 창 클래스는 그 클래스로 만든 창이 모두 사라진 뒤에만 해제할 수 있어 창을 먼저 없앤다
	DestroyWindow(static_cast<HWND>(nativeHandle));
	nativeHandle = nullptr;

	UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
}
