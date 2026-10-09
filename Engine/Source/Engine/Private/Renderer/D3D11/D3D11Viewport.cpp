#include "Renderer/D3D11/D3D11Viewport.h"

#include "Logging/LogMacros.h"
#include "Renderer/D3D11/D3D11Device.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogD3D11RHI);
}

FD3D11Viewport::FD3D11Viewport(FD3D11Device& InDevice)
	: device(&InDevice)
{
}

FD3D11Viewport::~FD3D11Viewport() = default;

bool FD3D11Viewport::Init(void* InWindowHandle, uint32 InSizeX, uint32 InSizeY)
{
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferDesc.Width = InSizeX;
	SwapChainDesc.BufferDesc.Height = InSizeY;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.OutputWindow = static_cast<HWND>(InWindowHandle);
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ID3D11Device* NativeDevice = device->GetDevice();

	if (!NativeDevice)
	{
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice> DXGIDevice;
	const HRESULT DXGIDeviceResult = NativeDevice->QueryInterface(IID_PPV_ARGS(DXGIDevice.GetAddressOf()));
	if (FAILED(DXGIDeviceResult))
	{
		DXGIDevice.Reset();

		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
	const HRESULT AdapterResult = DXGIDevice->GetAdapter(DXGIAdapter.GetAddressOf());
	if (FAILED(AdapterResult))
	{
		DXGIAdapter.Reset();
		DXGIDevice.Reset();

		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory> Factory;
	const HRESULT FactoryResult = DXGIAdapter->GetParent(IID_PPV_ARGS(Factory.GetAddressOf()));
	if (FAILED(FactoryResult))
	{
		Factory.Reset();
		DXGIAdapter.Reset();
		DXGIDevice.Reset();

		return false;
	}

	const HRESULT SwapChainResult = Factory->CreateSwapChain(NativeDevice, &SwapChainDesc, SwapChain.GetAddressOf());
	Factory.Reset();
	DXGIAdapter.Reset();
	DXGIDevice.Reset();
	if (FAILED(SwapChainResult))
	{
		Shutdown();

		return false;
	}

	if (!CreateBackBufferRenderTargetView())
	{
		Shutdown();

		return false;
	}

	SizeX = InSizeX;
	SizeY = InSizeY;

	return true;
}

void FD3D11Viewport::Shutdown()
{
	BackBufferRenderTargetView.Reset();
	SwapChain.Reset();

	SizeX = 0;
	SizeY = 0;
}

IDXGISwapChain* FD3D11Viewport::GetSwapChain() const
{
	return SwapChain.Get();
}

ID3D11RenderTargetView* FD3D11Viewport::GetBackBufferRenderTargetView() const
{
	return BackBufferRenderTargetView.Get();
}

void FD3D11Viewport::Resize(uint32 InSizeX, uint32 InSizeY)
{
	if (SizeX == InSizeX && SizeY == InSizeY)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = device->GetDeviceContext();
	DeviceContext->ClearState();
	BackBufferRenderTargetView.Reset();

	// 버퍼 수와 형식은 유지한다. Flags는 현재 생성 설정과 같은 0이다.
	const HRESULT Result = SwapChain->ResizeBuffers(0, InSizeX, InSizeY, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(Result))
	{
		SE_LOG(LogD3D11RHI, Fatal, "ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<uint32>(Result));
	}

	if (!CreateBackBufferRenderTargetView())
	{
		SE_LOG(LogD3D11RHI, Fatal, "Recreating the back buffer render target view after ResizeBuffers failed");
	}

	SizeX = InSizeX;
	SizeY = InSizeY;
}

bool FD3D11Viewport::Present()
{
	const HRESULT Result = SwapChain->Present(1, 0);

	if (FAILED(Result))
	{
		SE_LOG(LogD3D11RHI, Error, "Present failed (HRESULT 0x{:08X})", static_cast<uint32>(Result));

		return false;
	}

	return true;
}

bool FD3D11Viewport::CreateBackBufferRenderTargetView()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBufferResource;
	const HRESULT BackBufferResult = SwapChain->GetBuffer(
		0, IID_PPV_ARGS(BackBufferResource.GetAddressOf()));

	if (FAILED(BackBufferResult))
	{
		BackBufferResource.Reset();
		SE_LOG(LogD3D11RHI, Error, "GetBuffer failed (HRESULT 0x{:08X})", static_cast<uint32>(BackBufferResult));

		return false;
	}

	const HRESULT RenderTargetViewResult = device->GetDevice()->CreateRenderTargetView(
		BackBufferResource.Get(),
		nullptr,
		BackBufferRenderTargetView.GetAddressOf());

	BackBufferResource.Reset();

	if (FAILED(RenderTargetViewResult))
	{
		BackBufferRenderTargetView.Reset();
		SE_LOG(LogD3D11RHI, Error, "CreateRenderTargetView failed (HRESULT 0x{:08X})", static_cast<uint32>(RenderTargetViewResult));

		return false;
	}

	return true;
}
