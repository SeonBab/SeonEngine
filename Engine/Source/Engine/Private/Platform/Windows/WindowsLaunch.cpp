#include "Launch/EngineMain.h"
#include "Platform/Windows/WindowsHWrapper.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Renderer/Renderer.h"

// 창과 기본 렌더러는 공통 루프가 반환할 때까지 유지한다. 실제 자원 초기화는 루프에서 수행한다.
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	FWindowsWindow window;
	FRenderer renderer;
	return EngineMain(window, renderer);
}
