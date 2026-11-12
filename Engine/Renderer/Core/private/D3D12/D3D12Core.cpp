#include "D3D12Core.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


namespace Hakko
{

D3D12Core::D3D12Core()
{
    Init();
}

D3D12Core::~D3D12Core()
{
    Finalize();
}


D3D12Core& D3D12Core::getInstance()
{
    static D3D12Core instance;

    return instance;
}


void D3D12Core::Init()
{

	CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));


	GetHardwareAdapter(factory.Get(), &adapter, 1);


	D3D12CreateDevice(adapter.Get(),
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&device));

}

void D3D12Core::Finalize()
{
    directQueue.Reset();
    device.Reset();
	adapter.Reset();
    factory.Reset();
}


void D3D12Core::GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter)));
				++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter.Get() == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter.Detach();
}


IDXGIFactory4 *D3D12Core::GetFactory()
{
    return factory.Get();
}

IDXGIAdapter1 *D3D12Core::GetAdapter()
{
    return adapter.Get();
}

ID3D12Device *D3D12Core::GetDevice()
{
    return device.Get();
}

ID3D12CommandQueue *D3D12Core::GetDirectQueue()
{
    if (directQueue == nullptr)
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&directQueue));
    }

    return directQueue.Get();
}

}
