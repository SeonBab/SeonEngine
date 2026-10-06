#include "Platform/Windows/WindowsWindow.h"

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHWrapper.h"

// 공통 기반 클래스를 사용하는 쪽에 Win32 타입과 호출을 노출하지 않는다.

namespace
{
	// 창 클래스 등록, 창 생성, 해제에 같은 이름을 써야 한다
	constexpr const wchar_t* WindowClassName = L"SeonEngineWindow";
	constexpr const wchar_t* WindowTitle = L"SeonEngine";

	// 창 크기를 계산할 때와 창을 만들 때 같은 스타일을 써야 한다. 다르면 클라이언트 영역 크기가 틀어진다
	constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
	// 메인 창 하나라 특별한 동작(작업 표시줄 숨김, 항상 위 등)이 필요 없다
	constexpr DWORD WindowExStyle = 0;

	// 클라이언트 영역(테두리와 제목 표시줄을 뺀, 그림이 그려지는 영역)의 크기다
	constexpr int32 WindowWidth = 1280;
	constexpr int32 WindowHeight = 720;
}

namespace SE::Private
{
	// WindowsWindow.h의 friend와 같은 타입이어야 하므로 익명 namespace가 아닌 여기에 정의한다.
	struct FWindowsWindowProc
	{
		// 창 클래스에 등록하는 창 프로시저. Windows가 이 창에 보내는 모든 메시지가 여기로 온다
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			// CreateWindowExW가 반환하기 전에도 메시지가 오므로 WM_NCCREATE에서 객체를 연결한다.
			// CREATESTRUCTW에서 CreateWindowExW에 넘긴 FWindowsWindow를 꺼내,
			// 이후 메시지에서 찾을 수 있도록 창마다 있는 사용자 칸에 적어 둔다
			if (message == WM_NCCREATE)
			{
				const CREATESTRUCTW* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lParam);
				FWindowsWindow* window = static_cast<FWindowsWindow*>(createStruct->lpCreateParams);
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
				window->nativeHandle = hwnd;
				// 여기서 끝내지 않고 아래 HandleMessage를 거쳐 기본 처리로 넘긴다. 제목 설정 등 창 생성에 필요한 일을 기본 처리가 한다
			}

			// 창의 마지막 메시지. 이 뒤로 hwnd는 무효이므로, 객체가 사라진 창을 가리키지 않도록 연결을 끊는다
			if (message == WM_NCDESTROY)
			{
				FWindowsWindow* window = reinterpret_cast<FWindowsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
				if (window != nullptr)
				{
					window->nativeHandle = nullptr;
					window->clientWidth = 0;
					window->clientHeight = 0;
					window->bMinimized = false;
				}
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			}

			FWindowsWindow* window = reinterpret_cast<FWindowsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
			// WM_NCCREATE보다 먼저 오는 WM_GETMINMAXINFO처럼 연결 전에 온 메시지와, 연결을 끊은 WM_NCDESTROY는 기본 처리로 넘긴다
			if (window == nullptr)
			{
				return DefWindowProcW(hwnd, message, wParam, lParam);
			}
			return HandleMessage(*window, hwnd, message, wParam, lParam);
		}

		// 창과 연결된 객체의 메시지를 처리한다. 직접 처리하지 않는 메시지는 기본 처리로 넘긴다.
		static LRESULT HandleMessage(FWindowsWindow& window, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			if (message == WM_SIZE)
			{
				// 생성 중에도 올 수 있으므로 렌더러를 호출하지 않고 최신 상태만 저장한다.
				window.clientWidth = static_cast<uint32>(LOWORD(lParam));
				window.clientHeight = static_cast<uint32>(HIWORD(lParam));
				window.bMinimized = (wParam == SIZE_MINIMIZED);
				return 0;
			}

			// 창을 바로 없애지 않고 요청만 기록한다. 기본 처리에 넘기면 그 자리에서 창이 파괴되므로,
			// 실제 파괴는 엔진 종료 순서에서 Shutdown이 한다
			if (message == WM_CLOSE)
			{
				window.bCloseRequested = true;
				return 0;
			}

			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
	};
}

bool FWindowsWindow::Initialize()
{
	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.lpszClassName = WindowClassName;
	windowClass.lpfnWndProc = SE::Private::FWindowsWindowProc::WindowProc;
	// 창 프로시저가 든 모듈이 실행 파일이라 실행 파일의 인스턴스를 쓴다. 엔진을 DLL로 나누면 다시 정한다
	windowClass.hInstance = GetModuleHandleW(nullptr);
	// 비워 두면 클라이언트 영역에 들어온 커서가 직전 모양(테두리의 크기 조절 화살표 등) 그대로 남는다
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	if (RegisterClassExW(&windowClass) == 0)
	{
		// TODO(seon): 로그를 만들면 실패 원인(GetLastError)을 남긴다
		return false;
	}
	bClassRegistered = true;

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
		// TODO(seon): 로그를 만들면 실패 원인(GetLastError)을 남긴다. 해제가 오류 값을 덮어쓸 수 있어 해제보다 먼저 읽는다
		// 창 생성이 실패해도 창 클래스 등록은 남는다. 해제해서 Initialize 전 상태로 되돌린다
		UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
		bClassRegistered = false;
		return false;
	}

	// 첫 호출은 실행한 쪽이 넘긴 시작 정보(바로가기의 "최소화로 실행" 등)가 있으면 그것을 따른다
	ShowWindow(hwnd, SW_SHOW);
	return true;
}

void FWindowsWindow::Shutdown()
{
	// 남아 있는 것만 정리한다. Initialize가 실패한 뒤나 두 번째 호출에서는 할 일이 없다
	// TODO(seon): 로그를 만들면 DestroyWindow / UnregisterClassW의 실패를 남긴다. 실패해도 재시도나 복구는 하지 않는다

	// 창 클래스는 그 클래스로 만든 창이 모두 사라진 뒤에만 해제할 수 있어 창을 먼저 없앤다.
	// nativeHandle은 반환 전에 창 프로시저가 WM_NCDESTROY에서 비운다
	if (nativeHandle != nullptr)
	{
		DestroyWindow(static_cast<HWND>(nativeHandle));
	}

	if (bClassRegistered)
	{
		UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
		bClassRegistered = false;
	}
}

// TODO(seon): 메시지 큐는 창이 아니라 스레드에 하나 있다. 창이 둘 이상 필요해지면 창 밖으로 옮긴다
//             옮기기 쉽도록 멤버 변수를 쓰지 않는다
void FWindowsWindow::PumpMessages()
{
	MSG message = {};
	// 창 필터를 nullptr로 둬야 창에 속하지 않은 스레드 메시지까지 꺼낸다
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		// 키 입력 메시지에서 문자 메시지(WM_CHAR)를 만들어 큐에 넣는다
		TranslateMessage(&message);
		// 메시지가 가리키는 창의 창 프로시저를 부른다
		DispatchMessageW(&message);
	}
}
