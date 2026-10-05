#include "Launch/EngineMain.h"
#include "Platform/Windows/WindowsHWrapper.h"
#include "Platform/Windows/WindowsWindow.h"

// Windows 창 객체의 수명을 소유하고 공통 진입점에 넘긴다.
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	FWindowsWindow window;
	return EngineMain(window);
}
