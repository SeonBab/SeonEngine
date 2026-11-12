#pragma once

#include "D3D12Includes.h"

namespace Hakko
{

using Microsoft::WRL::ComPtr;
using namespace DirectX;


class D3D12Core
{

public:
    static D3D12Core& getInstance();

private:
	D3D12Core();
    D3D12Core(const D3D12Core&) = delete;
    D3D12Core& operator=(const D3D12Core&) = delete;

public:
	~D3D12Core();

    
private:
	void Init();
	void Finalize();
	

private:
	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> directQueue;


	void GetHardwareAdapter(
		IDXGIFactory1* pFactory,
		IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter);

public:
    IDXGIFactory4* GetFactory();
    IDXGIAdapter1* GetAdapter();
    ID3D12Device* GetDevice();
    ID3D12CommandQueue* GetDirectQueue();

};

}