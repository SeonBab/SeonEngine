#include "Renderer/Renderer.h"

#include "Logging/LogMacros.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogRenderer);
}

bool FRenderer::Init(void* WindowHandle, uint32 SizeX, uint32 SizeY)
{
	Device.InitD3DDevice();

	if (!Viewport.Init(WindowHandle, SizeX, SizeY))
	{
		Device.Shutdown();
		return false;
	}

	return true;
}

void FRenderer::Shutdown()
{
	// 출력 참조를 해제하기 전에 컨텍스트가 보유한 바인딩을 끊는다.
	ID3D11DeviceContext* DeviceContext = Device.GetDeviceContext();
	if (DeviceContext)
	{
		DeviceContext->ClearState();
	}

	Viewport.Shutdown();
	Device.Shutdown();
}

void FRenderer::Resize(uint32 SizeX, uint32 SizeY)
{
	Viewport.Resize(SizeX, SizeY);
}

void FRenderer::RenderFrame()
{
	constexpr float ClearColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};
	Device.ClearRenderTargetView(Viewport.GetBackBufferRenderTargetView(), ClearColor);

	if (!Viewport.Present())
	{
		SE_LOG(LogRenderer, Fatal, "Presenting the viewport failed");
	}
}
