#include "Launch/EngineMain.h"
#include "Platform/Windows/WindowsHWrapper.h"

// Windows 진입점. 플랫폼별 준비만 하고 EngineMain()으로 넘긴다.
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	return EngineMain();
}
