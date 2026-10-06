#include "Renderer/D3D11/D3D11Device.h"

#include "Logging/LogMacros.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogD3D11RHI);
}

bool FD3D11Device::Init(void* InWindowHandle, uint32 InSizeX, uint32 InSizeY)
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
	// Flip 방식은 단일 샘플과 두 개 이상의 버퍼가 필요하다. 구형 환경 fallback은 제공하지 않는다.
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	constexpr D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	constexpr D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32 DeviceFlags = 0;

#if SE_BUILD_DEBUG
	// 디버그 레이어가 설치되지 않았다면 생성 실패를 반환하며 레이어 없이 자동 재시도하지 않는다.
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
		// 호출자는 초기화 실패 후 Shutdown을 호출하지 않으므로 부분 자원은 여기서 정리한다.
		Shutdown();
		return false;
	}

	// 생성된 장치에 대응하는 adapter / factory를 사용해 다른 생성 경로와 섞이지 않게 한다.
	Microsoft::WRL::ComPtr<IDXGIDevice> DXGIDevice;
	const HRESULT DXGIDeviceResult = Direct3DDevice->QueryInterface(IID_PPV_ARGS(DXGIDevice.GetAddressOf()));
	if (FAILED(DXGIDeviceResult))
	{
		DXGIDevice.Reset();
		Shutdown();
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
	const HRESULT AdapterResult = DXGIDevice->GetAdapter(DXGIAdapter.GetAddressOf());
	if (FAILED(AdapterResult))
	{
		DXGIAdapter.Reset();
		DXGIDevice.Reset();
		Shutdown();
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory> Factory;
	const HRESULT FactoryResult = DXGIAdapter->GetParent(IID_PPV_ARGS(Factory.GetAddressOf()));
	if (FAILED(FactoryResult))
	{
		Factory.Reset();
		DXGIAdapter.Reset();
		DXGIDevice.Reset();
		Shutdown();
		return false;
	}

	// 기존 출력 설정을 유지하여 생성 호출 분리와 출력 방식 변경을 구분한다.
	const HRESULT SwapChainResult = Factory->CreateSwapChain(Direct3DDevice.Get(), &SwapChainDesc, SwapChain.GetAddressOf());
	// 생성 경로를 찾는 데만 쓴 참조는 성공 / 실패 모두 장치 정리 전에 내려놓는다.
	Factory.Reset();
	DXGIAdapter.Reset();
	DXGIDevice.Reset();
	if (FAILED(SwapChainResult))
	{
		Shutdown();
		return false;
	}

	// 스왑체인이 만든 버퍼의 참조를 받는다. 별도의 텍스처를 생성하는 단계가 아니다.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBufferResource;
	const HRESULT BackBufferResult = SwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(BackBufferResource.GetAddressOf()));

	if (FAILED(BackBufferResult))
	{
		BackBufferResource.Reset();
		Shutdown();
		return false;
	}

	const HRESULT RenderTargetViewResult = Direct3DDevice->CreateRenderTargetView(
		BackBufferResource.Get(),
		nullptr, // 형식이 확정된 단일 2D 백버퍼이므로 기본 뷰 설정을 사용한다.
		BackBufferRenderTargetView.GetAddressOf());

	if (FAILED(RenderTargetViewResult))
	{
		// 지역 백버퍼 참조를 먼저 내려놓아 스왑체인 정리 / Flush 동안 남지 않게 한다.
		BackBufferResource.Reset();
		Shutdown();
		return false;
	}

	// 출력 뷰와 스왑체인이 자원을 유지하므로 초기화에만 쓴 지역 참조는 더 필요하지 않다.
	BackBufferResource.Reset();

	SizeX = InSizeX;
	SizeY = InSizeY;
	return true;
}

void FD3D11Device::Shutdown()
{
	// 컨텍스트가 자원 참조를 붙잡을 수 있어 보유 참조를 해제하기 전에 바인딩을 끊는다.
	if (Direct3DDeviceIMContext)
	{
		Direct3DDeviceIMContext->ClearState();
	}

	BackBufferRenderTargetView.Reset();
	SwapChain.Reset();

	if (Direct3DDeviceIMContext)
	{
		// 해제 관련 처리를 제출하지만 GPU 실행 완료를 기다리지는 않는다.
		Direct3DDeviceIMContext->Flush();
	}

	Direct3DDeviceIMContext.Reset();
	Direct3DDevice.Reset();

	SizeX = 0;
	SizeY = 0;
}

void FD3D11Device::Resize(uint32 InSizeX, uint32 InSizeY)
{
	if (SizeX == InSizeX && SizeY == InSizeY) { return; }

	// 뷰와 컨텍스트 바인딩도 백버퍼 참조를 유지하므로 크기 변경 전에 모두 해제한다.
	Direct3DDeviceIMContext->ClearState();
	BackBufferRenderTargetView.Reset();

	// 버퍼 수와 형식은 유지한다. Flags는 현재 생성 설정과 같은 0이며 자동 유지 인수가 아니다.
	const HRESULT Result = SwapChain->ResizeBuffers(0, InSizeX, InSizeY, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(Result))
	{
		SE_LOG(LogD3D11RHI, Fatal, "ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<uint32>(Result));
	}

	// 이전 뷰는 교체된 버퍼에 사용할 수 없으므로 새 백버퍼를 확보하고 뷰를 다시 만든다.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBufferResource;
	const HRESULT BackBufferResult = SwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(BackBufferResource.GetAddressOf()));
	if (FAILED(BackBufferResult))
	{
		SE_LOG(LogD3D11RHI, Fatal, "GetBuffer after ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<uint32>(BackBufferResult));
	}

	const HRESULT RenderTargetViewResult = Direct3DDevice->CreateRenderTargetView(
		BackBufferResource.Get(),
		nullptr,
		BackBufferRenderTargetView.GetAddressOf());
	if (FAILED(RenderTargetViewResult))
	{
		SE_LOG(LogD3D11RHI, Fatal, "CreateRenderTargetView after ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<uint32>(RenderTargetViewResult));
	}

	BackBufferResource.Reset();

	// 필수 출력 자원을 모두 준비한 뒤에만 적용 크기를 갱신한다.
	SizeX = InSizeX;
	SizeY = InSizeY;
}

void FD3D11Device::RenderFrame()
{
	// 기본 창 배경과 첫 출력 결과를 구분하기 위한 마젠타다.
	constexpr float ClearColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};
	// 지우기는 뷰 전체에 직접 작용하므로 Draw용 출력 바인딩 / viewport 설정이 필요 없다.
	Direct3DDeviceIMContext->ClearRenderTargetView(BackBufferRenderTargetView.Get(), ClearColor);

	// 첫 출력은 수직 동기화를 요청하고 추가 표시 옵션은 사용하지 않는다.
	const HRESULT Result = SwapChain->Present(1, 0);
	// 가려짐 같은 성공 상태는 실패로 취급하지 않는다. 장치 손실 복구는 아직 지원하지 않는다.
	if (FAILED(Result))
	{
		SE_LOG(LogD3D11RHI, Fatal, "Present failed (HRESULT 0x{:08X})", static_cast<uint32>(Result));
	}
}
