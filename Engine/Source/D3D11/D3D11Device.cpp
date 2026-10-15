#include "D3D11Device.h"

#include "Logging/Log.h"

#include <utility>

bool FD3D11Device::Initialize()
{
	if (device || context)
	{
		WriteLog("D3D11", ELogLevel::Error, "D3D11 device is already initialized.");
		return false;
	}

	const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
	Microsoft::WRL::ComPtr<ID3D11Device> createdDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> createdContext;
	const HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, createdDevice.GetAddressOf(), nullptr, createdContext.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "D3D11CreateDevice failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	device = std::move(createdDevice);
	context = std::move(createdContext);
	WriteLog("D3D11", ELogLevel::Info, "D3D11 hardware device created (feature level 11_0).");
	return true;
}

void FD3D11Device::Shutdown()
{
	context.Reset();
	device.Reset();
}
