#include "Renderer/RendererFactory.h"

#include "Core/Templates/UniquePtr.h"
#include "D3D11/D3D11Renderer.h"

TUniquePtr<IRenderer> CreateRenderer()
{
	return MakeUnique<FD3D11Renderer>();
}
