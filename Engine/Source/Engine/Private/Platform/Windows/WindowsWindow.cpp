#include "Platform/Window.h"

#include "Platform/Windows/WindowsHeaders.h"

// Window.h에 선언한 플랫폼 창의 Windows 구현이다. Win32 호출은 이 파일에만 둔다.

namespace
{
	// 창 클래스 등록, 창 생성, 해제에 같은 이름을 써야 한다
	constexpr const wchar_t* WindowClassName = L"SeonEngineWindow";
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

	if (RegisterClassExW(&windowClass) == 0)
	{
		return false;
	}

	// TODO(seon): 창 생성
	return true;
}

void FWindow::Shutdown()
{
	// TODO(seon): 창 파괴
	UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
}
