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

namespace SE::Private
{
	// Window.h에서 FWindow의 friend로 지정한 구조체다. 헤더의 이름과 같은 타입이 되도록 익명 namespace가 아닌 여기에 정의한다
	struct FWindowsWindowProc
	{
		// 창 클래스에 등록하는 창 프로시저. Windows가 이 창에 보내는 모든 메시지가 여기로 온다
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			// 창 생성 초기에 객체를 연결한다. CREATESTRUCTW에서 CreateWindowExW에 넘긴 FWindow를 꺼내,
			// 이후 메시지에서 찾을 수 있도록 창마다 있는 사용자 칸에 적어 둔다
			if (message == WM_NCCREATE)
			{
				const CREATESTRUCTW* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lParam);
				FWindow*             window       = static_cast<FWindow*>(createStruct->lpCreateParams);
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
				window->nativeHandle = hwnd;
			}

			// 창의 마지막 메시지. 이 뒤로 hwnd는 무효이므로, 객체가 사라진 창을 가리키지 않도록 연결을 끊는다
			if (message == WM_NCDESTROY)
			{
				FWindow* window = reinterpret_cast<FWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
				if (window != nullptr)
				{
					window->nativeHandle = nullptr;
				}
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			}

			// WM_NCCREATE도 기본 처리로 넘겨야 한다. 제목 설정 등 창 생성에 필요한 일을 기본 처리가 한다
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
	};
}

bool FWindow::Initialize()
{
	WNDCLASSEXW windowClass   = {};
	windowClass.cbSize        = sizeof(windowClass);
	windowClass.lpszClassName = WindowClassName;
	windowClass.lpfnWndProc   = SE::Private::FWindowsWindowProc::WindowProc;
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
		this);                              // 창 프로시저가 WM_NCCREATE에서 받아 이 객체와 창을 연결한다
	// nativeHandle은 반환 전에 창 프로시저가 WM_NCCREATE에서 이미 넣었다
	if (hwnd == nullptr)
	{
		return false;
	}

	// 첫 호출은 실행한 쪽이 넘긴 시작 정보(바로가기의 "최소화로 실행" 등)가 있으면 그것을 따른다
	ShowWindow(hwnd, SW_SHOW);
	return true;
}

void FWindow::Shutdown()
{
	// 창 클래스는 그 클래스로 만든 창이 모두 사라진 뒤에만 해제할 수 있어 창을 먼저 없앤다.
	// nativeHandle은 반환 전에 창 프로시저가 WM_NCDESTROY에서 비운다
	DestroyWindow(static_cast<HWND>(nativeHandle));

	UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
}
