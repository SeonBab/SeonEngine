#include "D3D11SwapChain.h"

#include "Core/Logging/Log.h"

#include <utility>

#pragma region Lifecycle

bool FD3D11SwapChain::Initialize(ID3D11Device* device, HWND window)
{
	if (swapChain || renderTargetView)
	{
		WriteLog("D3D11", ELogLevel::Error, "D3D11 swap chain is already initialized.");
		return false;
	}
	if (device == nullptr || window == nullptr)
	{
		WriteLog("D3D11", ELogLevel::Error, "Swap chain requires a device and a window.");
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	HRESULT result = device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "QueryInterface(IDXGIDevice) failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	result = dxgiDevice->GetAdapter(adapter.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "GetAdapter failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
	result = adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "GetParent(IDXGIFactory2) failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 description{};
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = 2;
	description.Scaling = DXGI_SCALING_STRETCH;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> createdSwapChain;
	result = factory->CreateSwapChainForHwnd(device, window, &description, nullptr, nullptr, createdSwapChain.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateSwapChainForHwnd failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	result = factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "MakeWindowAssociation failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> createdRenderTargetView;
	if (!CreateBackBufferRenderTargetView(device, createdSwapChain.Get(), createdRenderTargetView))
	{
		return false;
	}

	swapChain = std::move(createdSwapChain);
	renderTargetView = std::move(createdRenderTargetView);
	WriteLog("D3D11", ELogLevel::Info, "D3D11 windowed swap chain created (RGBA8, 2 buffers).");
	WriteLog("D3D11", ELogLevel::Info, "D3D11 back buffer render target view created.");

	return true;
}

void FD3D11SwapChain::Shutdown()
{
	renderTargetView.Reset();
	swapChain.Reset();
}

#pragma endregion

#pragma region Presentation

bool FD3D11SwapChain::Present()
{
	const HRESULT result = swapChain->Present(1, 0);
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "Present failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	return true;
}

bool FD3D11SwapChain::Resize(ID3D11Device* device, int width, int height)
{
	if (!swapChain || device == nullptr || width <= 0 || height <= 0)
	{
		WriteLog("D3D11", ELogLevel::Error, "Resize requires a swap chain, a device, and positive dimensions.");
		return false;
	}

	// 호출자가 백 버퍼 바인딩을 해제한 뒤 들어오므로 여기서는 소유 RTV 참조를 해제한다.
	renderTargetView.Reset();
	const HRESULT result = swapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	if (!CreateBackBufferRenderTargetView(device, swapChain.Get(), renderTargetView))
	{
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "D3D11 back buffer resized: {} x {}", width, height);

	return true;
}

bool FD3D11SwapChain::CreateBackBufferRenderTargetView(ID3D11Device* device, IDXGISwapChain1* sourceSwapChain, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& outView)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT result = sourceSwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "GetBuffer failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> createdRenderTargetView;
	result = device->CreateRenderTargetView(backBuffer.Get(), nullptr, createdRenderTargetView.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateRenderTargetView failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outView = std::move(createdRenderTargetView);

	return true;
}

#pragma endregion
