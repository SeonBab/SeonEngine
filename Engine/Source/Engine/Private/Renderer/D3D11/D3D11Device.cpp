#include "Renderer/D3D11/D3D11Device.h"

#include "Containers/Array.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "Platform/Windows/WindowsString.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <utility>

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogD3D11RHI);

	////////////////////////////////////////////////////////////////////////////////
	// 첫 삼각형의 POSITION 입력에 맞춘 위치 정점이다. 범용 메시 정점 형식이 아니다.
	// 셰이더가 변환 없이 사용하므로 위치는 클립 공간 좌표로 준비한다.
	////////////////////////////////////////////////////////////////////////////////
	struct FTriangleVertex
	{
		float Position[3];
	};

	// 고정 내부 파일 이름과 초기 .cso 배치에 맞춘 처리다. 셰이더 라이브러리를 도입하면 교체한다.
	std::optional<FString> GetShaderBytecodePath(const FString& ShaderFileName)
	{
		const std::optional<FString> BaseDir = FPlatformProcess::BaseDir();
		if (!BaseDir)
		{
			return std::nullopt;
		}

		return *BaseDir + "Shaders/" + ShaderFileName;
	}

	// 유효한 내부 UTF-8 경로만 받는다. 기존 변환은 깨진 UTF-8을 대체하므로 경로 검증을 대신하지 않는다.
	bool LoadShaderBytecode(TArray<uint8>& OutCode, const FString& FilePath)
	{
		const std::optional<std::wstring> WidePath = FWindowsString::UTF8ToWide(FilePath);
		if (!WidePath)
		{
			return false;
		}

		// 바이트를 그대로 읽고, 다음 단계에서 크기를 조회할 수 있도록 파일 끝에서 시작한다.
		std::ifstream File(std::filesystem::path(*WidePath), std::ios::binary | std::ios::ate);
		if (!File)
		{
			return false;
		}

		const std::streamoff FileSize = File.tellg();
		if (FileSize <= 0)
		{
			return false;
		}

		// 배열과 읽기 함수의 크기 인수 모두에서 표현 가능한 크기만 준비한다.
		TArray<uint8> Code;
		const uint64 CodeSize = static_cast<uint64>(FileSize);
		if (CodeSize > Code.max_size() || CodeSize > static_cast<uint64>(std::numeric_limits<std::streamsize>::max()))
		{
			return false;
		}

		// 이후 읽기가 실패해도 호출자의 결과를 유지할 수 있도록 지역 배열만 준비한다.
		Code.resize(static_cast<size_t>(CodeSize));

		File.seekg(0, std::ios::beg);
		if (!File)
		{
			return false;
		}

		// 짧은 읽기나 입력 오류가 발생하면 호출자가 갖고 있던 결과를 그대로 유지한다.
		if (!File.read(reinterpret_cast<char*>(Code.data()), static_cast<std::streamsize>(CodeSize)))
		{
			return false;
		}

		OutCode = std::move(Code);
		return true;
	}
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
	const HRESULT BackBufferResult = SwapChain->GetBuffer(0, IID_PPV_ARGS(BackBufferResource.GetAddressOf()));

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

	const std::optional<FString> VertexShaderPath = GetShaderBytecodePath("TriangleVertexShader.cso");
	if (!VertexShaderPath)
	{
		Shutdown();
		return false;
	}

	// 초기화 때만 읽는다. GPU 객체에는 파일 경로가 아니라 컴파일된 바이트코드를 전달한다.
	TArray<uint8> VertexShaderCode;
	if (!LoadShaderBytecode(VertexShaderCode, *VertexShaderPath))
	{
		Shutdown();
		return false;
	}

	const HRESULT VertexShaderResult = Direct3DDevice->CreateVertexShader(
		VertexShaderCode.data(),
		VertexShaderCode.size(),
		nullptr, // 현재 셰이더는 동적 클래스 연결을 사용하지 않는다.
		VertexShader.GetAddressOf());
	if (FAILED(VertexShaderResult))
	{
		Shutdown();
		return false;
	}

	const std::optional<FString> PixelShaderPath = GetShaderBytecodePath("TrianglePixelShader.cso");
	if (!PixelShaderPath)
	{
		Shutdown();
		return false;
	}

	TArray<uint8> PixelShaderCode;
	if (!LoadShaderBytecode(PixelShaderCode, *PixelShaderPath))
	{
		Shutdown();
		return false;
	}

	const HRESULT PixelShaderResult = Direct3DDevice->CreatePixelShader(
		PixelShaderCode.data(),
		PixelShaderCode.size(),
		nullptr, // 현재 셰이더는 동적 클래스 연결을 사용하지 않는다.
		PixelShader.GetAddressOf());
	if (FAILED(PixelShaderResult))
	{
		Shutdown();
		return false;
	}

	// 변환 없는 셰이더에서 사용할 클립 공간 위치다.
	constexpr FTriangleVertex Vertices[] = {
		{{-0.5f, -0.5f, 0.0f}},
		{{0.0f, 0.5f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}},
	};

	// 고정된 세 정점은 생성 시 한 번 전달하고 이후 갱신하지 않는다.
	D3D11_BUFFER_DESC BufferDesc{};
	BufferDesc.ByteWidth = sizeof(Vertices);
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// 로컬 정점 배열은 버퍼 생성 호출이 끝날 때까지 살아 있어야 한다.
	D3D11_SUBRESOURCE_DATA InitialData{};
	InitialData.pSysMem = Vertices;

	const HRESULT VertexBufferResult = Direct3DDevice->CreateBuffer(
		&BufferDesc,
		&InitialData,
		VertexBuffer.GetAddressOf());
	if (FAILED(VertexBufferResult))
	{
		Shutdown();
		return false;
	}

	// 현재 정점은 첫 멤버에 위치만 담는다.
	const D3D11_INPUT_ELEMENT_DESC InputElements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// 정점 메모리 설명을 셰이더 입력 서명과 연결한다. 초기화 때 읽은 코드를 재사용한다.
	const HRESULT InputLayoutResult = Direct3DDevice->CreateInputLayout(
		InputElements,
		1,
		VertexShaderCode.data(),
		VertexShaderCode.size(),
		InputLayout.GetAddressOf());
	if (FAILED(InputLayoutResult))
	{
		Shutdown();
		return false;
	}

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

	InputLayout.Reset();
	VertexBuffer.Reset();
	PixelShader.Reset();
	VertexShader.Reset();
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
	const HRESULT BackBufferResult = SwapChain->GetBuffer(0, IID_PPV_ARGS(BackBufferResource.GetAddressOf()));
	if (FAILED(BackBufferResult))
	{
		SE_LOG(LogD3D11RHI, Fatal, "GetBuffer after ResizeBuffers failed (HRESULT 0x{:08X})", static_cast<uint32>(BackBufferResult));
	}

	const HRESULT RenderTargetViewResult = Direct3DDevice->CreateRenderTargetView(BackBufferResource.Get(), nullptr, BackBufferRenderTargetView.GetAddressOf());
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
	// Resize의 ClearState와 flip 방식 Present 뒤에도 그리기 대상이 연결되도록 매 프레임 설정한다.
	ID3D11RenderTargetView* RTArray[] = {BackBufferRenderTargetView.Get()};
	Direct3DDeviceIMContext->OMSetRenderTargets(1, RTArray, nullptr);

	// Resize에서 적용한 출력 크기를 사용한다. ClearState 뒤에도 그릴 영역이 복원되도록 매 프레임 설정한다.
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<float>(SizeX);
	Viewport.Height = static_cast<float>(SizeY);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	Direct3DDeviceIMContext->RSSetViewports(1, &Viewport);

	// Resize의 ClearState 뒤에도 사용할 셰이더가 복원되도록 매 프레임 지정한다.
	Direct3DDeviceIMContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	Direct3DDeviceIMContext->PSSetShader(PixelShader.Get(), nullptr, 0);

	// Resize의 ClearState는 입력 상태도 지우므로 정점 해석 규칙을 매 프레임 복원한다.
	Direct3DDeviceIMContext->IASetInputLayout(InputLayout.Get());

	// 입력 설명의 슬롯 0에 연결한다. Resize의 ClearState 뒤에도 버퍼와 간격을 복원한다.
	ID3D11Buffer* Buffers[] = {VertexBuffer.Get()};
	const uint32 Stride = sizeof(FTriangleVertex);
	const uint32 Offset = 0;
	Direct3DDeviceIMContext->IASetVertexBuffers(0, 1, Buffers, &Stride, &Offset);

	// 세 정점을 독립된 삼각형으로 묶는다. ClearState 뒤에도 해석 방식이 복원되도록 지정한다.
	Direct3DDeviceIMContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 기본 창 배경과 첫 출력 결과를 구분하기 위한 마젠타다.
	constexpr float ClearColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};
	// 지우기는 뷰 전체에 직접 작용하므로 Draw용 출력 바인딩 / viewport 설정이 필요 없다.
	Direct3DDeviceIMContext->ClearRenderTargetView(BackBufferRenderTargetView.Get(), ClearColor);

	// 배경을 지운 뒤 그려야 삼각형이 남고, 이후 Present에서 이 프레임을 제출한다.
	Direct3DDeviceIMContext->Draw(3, 0);

	// 첫 출력은 수직 동기화를 요청하고 추가 표시 옵션은 사용하지 않는다.
	const HRESULT Result = SwapChain->Present(1, 0);
	// 가려짐 같은 성공 상태는 실패로 취급하지 않는다. 장치 손실 복구는 아직 지원하지 않는다.
	if (FAILED(Result))
	{
		SE_LOG(LogD3D11RHI, Fatal, "Present failed (HRESULT 0x{:08X})", static_cast<uint32>(Result));
	}
}
