#include "Renderer/D3D11/D3D11Device.h"

#include "Logging/LogMacros.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogD3D11RHI);
}

void FD3D11Device::InitD3DDevice()
{
	if (Direct3DDevice) { return; }

	constexpr D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	constexpr D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32 DeviceFlags = 0;

#if SE_BUILD_DEBUG
	// 디버그 레이어가 설치되지 않았다면 치명 종료하며 레이어 없이 자동 재시도하지 않는다.
	DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL ActualFeatureLevel{};
	// 기본 하드웨어 adapter를 사용하며 기능 수준은 11.0 하나만 요구한다.
	// 출력 주소는 첫 초기화의 빈 ComPtr에만 사용한다. 기존 참조를 자동 해제하지 않는다.
	const HRESULT DeviceResult = D3D11CreateDevice(
		nullptr,
		DriverType,
		nullptr,
		DeviceFlags,
		&FeatureLevel,
		1,
		D3D11_SDK_VERSION,
		Direct3DDevice.GetAddressOf(),
		&ActualFeatureLevel,
		Direct3DDeviceIMContext.GetAddressOf());

	if (FAILED(DeviceResult))
	{
		// 생성 실패는 정상 반환하지 않으며 진단 전에 확보한 부분 자원을 정리한다.
		Shutdown();
		SE_LOG(LogD3D11RHI, Fatal, "D3D11CreateDevice failed (HRESULT 0x{:08X})", static_cast<uint32>(DeviceResult));
	}
}

void FD3D11Device::Shutdown()
{
	// 컨텍스트가 자원 참조를 붙잡을 수 있어 보유 참조를 해제하기 전에 바인딩을 끊는다.
	if (Direct3DDeviceIMContext)
	{
		Direct3DDeviceIMContext->ClearState();
	}

	if (Direct3DDeviceIMContext)
	{
		// 해제 관련 처리를 제출하지만 GPU 실행 완료를 기다리지는 않는다.
		Direct3DDeviceIMContext->Flush();
	}

	Direct3DDeviceIMContext.Reset();
	Direct3DDevice.Reset();
}

ID3D11DeviceContext* FD3D11Device::GetDeviceContext() const
{
	return Direct3DDeviceIMContext.Get();
}

void FD3D11Device::ClearRenderTargetView(ID3D11RenderTargetView* InRenderTargetView, const float InClearColor[4])
{
	Direct3DDeviceIMContext->ClearRenderTargetView(InRenderTargetView, InClearColor);
}
