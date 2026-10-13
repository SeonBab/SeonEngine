#include "Framework/Application/SlateApplication.h"
#include "Launch/EngineMain.h"
#include "Platform/GenericPlatform/GenericWindow.h"
#include "Platform/Windows/WindowsApplication.h"
#include "Platform/Windows/WindowsHWrapper.h"
#include "Renderer/Renderer.h"
#include "Templates/SharedPointer.h"

// 애플리케이션 / 창 / 렌더러는 공통 루프가 반환할 때까지 유지한다. 자원 초기화는 루프에서 수행한다.
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	TSharedPtr<FWindowsApplication> application = MakeShared<FWindowsApplication>();
	FSlateApplication::Create(application);

	TSharedPtr<FGenericWindow> window = application->MakeWindow();
	FRenderer renderer;
	const int exitCode = EngineMain(*application, window, renderer);

	FSlateApplication::Shutdown();
	return exitCode;
}
