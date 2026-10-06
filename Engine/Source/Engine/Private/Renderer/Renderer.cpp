#include "Renderer/Renderer.h"

bool FRenderer::Init(void* WindowHandle, uint32 SizeX, uint32 SizeY)
{
	return Device.Init(WindowHandle, SizeX, SizeY);
}

void FRenderer::Shutdown()
{
	Device.Shutdown();
}

void FRenderer::Resize(uint32 SizeX, uint32 SizeY)
{
	Device.Resize(SizeX, SizeY);
}

void FRenderer::RenderFrame()
{
	Device.RenderFrame();
}
