#include "Renderer/D3D11/D3D11Device.h"

#include "Logging/LogMacros.h"

#include <cstddef>
#include <limits>
#include <utility>

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

void FD3D11Device::ClearDepthStencilView(ID3D11DepthStencilView* depthStencilView, float depth)
{
	Direct3DDeviceIMContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, depth, 0);
}

bool FD3D11Device::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& description, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& outState)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a depth stencil state requires an initialized D3D11 device");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> newState;
	const HRESULT result = Direct3DDevice->CreateDepthStencilState(&description, newState.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a depth stencil state failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}

	outState = std::move(newState);

	return true;
}

bool FD3D11Device::CreateRasterizerState(const D3D11_RASTERIZER_DESC& description, Microsoft::WRL::ComPtr<ID3D11RasterizerState>& outState)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a rasterizer state requires an initialized D3D11 device");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> newState;
	const HRESULT result = Direct3DDevice->CreateRasterizerState(&description, newState.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a rasterizer state failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}

	outState = std::move(newState);

	return true;
}

bool FD3D11Device::CreateSamplerState(const D3D11_SAMPLER_DESC& description, Microsoft::WRL::ComPtr<ID3D11SamplerState>& outState)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a sampler state requires an initialized D3D11 device");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> newState;
	const HRESULT result = Direct3DDevice->CreateSamplerState(&description, newState.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a sampler state failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outState = std::move(newState);

	return true;
}

bool FD3D11Device::CreateConstantBuffer(uint32 byteSize, Microsoft::WRL::ComPtr<ID3D11Buffer>& outConstantBuffer)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a constant buffer requires an initialized D3D11 device");

		return false;
	}
	if (byteSize == 0 || byteSize % 16 != 0)
	{
		SE_LOG(LogD3D11RHI, Error, "Constant buffer size must be a nonzero multiple of 16 bytes (requested {})", byteSize);

		return false;
	}

	// 범위 지정 없는 기본 상수 버퍼 사용을 위해 64KiB 이내로 제한한다.
	constexpr uint32 MaxConstantBufferByteSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
	if (byteSize > MaxConstantBufferByteSize)
	{
		SE_LOG(LogD3D11RHI, Error, "Constant buffer size exceeds the supported 64 KiB range (requested {})", byteSize);

		return false;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = byteSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Microsoft::WRL::ComPtr<ID3D11Buffer> newConstantBuffer;
	const HRESULT bufferResult = Direct3DDevice->CreateBuffer(&bufferDesc, nullptr, newConstantBuffer.GetAddressOf());
	if (FAILED(bufferResult))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a constant buffer failed (HRESULT 0x{:08X})", static_cast<uint32>(bufferResult));

		return false;
	}
	outConstantBuffer = std::move(newConstantBuffer);

	return true;
}

bool FD3D11Device::UpdateConstantBuffer(ID3D11Buffer* constantBuffer, const void* data, uint32 byteSize)
{
	if (!Direct3DDeviceIMContext)
	{
		SE_LOG(LogD3D11RHI, Error, "Updating a constant buffer requires an initialized D3D11 context");

		return false;
	}
	if (!constantBuffer || !data)
	{
		SE_LOG(LogD3D11RHI, Error, "Updating a constant buffer requires a buffer and CPU data");

		return false;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	constantBuffer->GetDesc(&bufferDesc);
	if (bufferDesc.Usage != D3D11_USAGE_DEFAULT || bufferDesc.BindFlags != D3D11_BIND_CONSTANT_BUFFER)
	{
		SE_LOG(LogD3D11RHI, Error, "Updating a constant buffer requires a DEFAULT constant buffer");

		return false;
	}
	if (byteSize != bufferDesc.ByteWidth)
	{
		SE_LOG(LogD3D11RHI, Error, "Constant buffer update size must match the entire buffer (requested {}, buffer {})", byteSize, bufferDesc.ByteWidth);

		return false;
	}

	// 기본 상수 버퍼 갱신은 전체 복사이며 API는 HRESULT를 반환하지 않는다.
	Direct3DDeviceIMContext->UpdateSubresource(constantBuffer, 0, nullptr, data, 0, 0);

	return true;
}

void FD3D11Device::SetVertexShaderConstantBuffer(ID3D11Buffer* constantBuffer)
{
	ID3D11Buffer* buffers[] = {constantBuffer};

	Direct3DDeviceIMContext->VSSetConstantBuffers(0, 1, buffers);
}

bool FD3D11Device::CreateVertexBuffer(const TArray<FStaticMeshVertex>& vertices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outVertexBuffer)
{
	if (vertices.empty()) { return false; }
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex buffer requires an initialized D3D11 device");

		return false;
	}
	if (vertices.size() > std::numeric_limits<uint32>::max() / sizeof(FStaticMeshVertex))
	{
		SE_LOG(LogD3D11RHI, Error, "Vertex buffer size exceeds the D3D11 ByteWidth range");

		return false;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<uint32>(vertices.size() * sizeof(FStaticMeshVertex));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = vertices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> newVertexBuffer;
	const HRESULT bufferResult = Direct3DDevice->CreateBuffer(&bufferDesc, &initialData, newVertexBuffer.GetAddressOf());
	if (FAILED(bufferResult))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex buffer failed (HRESULT 0x{:08X})", static_cast<uint32>(bufferResult));

		return false;
	}
	outVertexBuffer = std::move(newVertexBuffer);

	return true;
}

void FD3D11Device::SetVertexBuffer(ID3D11Buffer* vertexBuffer)
{
	ID3D11Buffer* buffers[] = {vertexBuffer};
	constexpr uint32 stride = sizeof(FStaticMeshVertex);
	constexpr uint32 offset = 0;

	Direct3DDeviceIMContext->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
}

bool FD3D11Device::CreateTexture2D(uint32 width, uint32 height, const TArray<uint8>& rgbaPixels, Microsoft::WRL::ComPtr<ID3D11Texture2D>& outTexture)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a texture requires an initialized D3D11 device");

		return false;
	}
	if (width == 0 || height == 0 || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
	{
		SE_LOG(LogD3D11RHI, Error, "Texture dimensions are outside the D3D11 2D range");

		return false;
	}

	// 크기 상한 검사 후 계산하므로 행 바이트 수와 전체 RGBA8 크기가 넘치지 않는다.
	const uint32 rowPitch = width * 4;
	const size_t byteSize = static_cast<size_t>(rowPitch) * height;
	if (rgbaPixels.size() != byteSize)
	{
		SE_LOG(LogD3D11RHI, Error, "Texture RGBA8 data size does not match its dimensions");

		return false;
	}

	D3D11_TEXTURE2D_DESC description{};
	description.Width = width;
	description.Height = height;
	description.MipLevels = 1;
	description.ArraySize = 1;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = rgbaPixels.data();
	initialData.SysMemPitch = rowPitch;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> newTexture;
	const HRESULT result = Direct3DDevice->CreateTexture2D(&description, &initialData, newTexture.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a texture failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outTexture = std::move(newTexture);

	return true;
}

bool FD3D11Device::CreateShaderResourceView(ID3D11Texture2D* texture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& outView)
{
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a shader resource view requires an initialized D3D11 device");

		return false;
	}
	if (!texture)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a shader resource view requires a texture");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> textureDevice;
	texture->GetDevice(textureDevice.GetAddressOf());
	if (textureDevice.Get() != Direct3DDevice.Get())
	{
		SE_LOG(LogD3D11RHI, Error, "The shader resource texture belongs to another D3D11 device");

		return false;
	}

	// nullptr 설명은 텍스처 형식과 전체 범위에 맞는 기본 뷰를 요청한다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> newView;
	const HRESULT result = Direct3DDevice->CreateShaderResourceView(texture, nullptr, newView.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a shader resource view failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outView = std::move(newView);

	return true;
}

bool FD3D11Device::CreateInputLayout(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11InputLayout>& outInputLayout)
{
	if (vertexShaderBytecode.empty())
	{
		SE_LOG(LogD3D11RHI, Error, "Creating an input layout requires vertex shader bytecode");

		return false;
	}
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating an input layout requires an initialized D3D11 device");

		return false;
	}

	D3D11_INPUT_ELEMENT_DESC inputElements[3]{};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[0].AlignedByteOffset = static_cast<uint32>(offsetof(FStaticMeshVertex, position));
	inputElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElements[1].SemanticName = "COLOR";
	inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElements[1].AlignedByteOffset = static_cast<uint32>(offsetof(FStaticMeshVertex, color));
	inputElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	// UV 두 성분을 정점 셰이더의 TEXCOORD0 입력에 연결한다.
	inputElements[2].SemanticName = "TEXCOORD";
	inputElements[2].SemanticIndex = 0;
	inputElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElements[2].AlignedByteOffset = static_cast<uint32>(offsetof(FStaticMeshVertex, uv));
	inputElements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> newInputLayout;
	const HRESULT result = Direct3DDevice->CreateInputLayout(
		inputElements, 3, vertexShaderBytecode.data(), vertexShaderBytecode.size(), newInputLayout.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating an input layout failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outInputLayout = std::move(newInputLayout);

	return true;
}

void FD3D11Device::SetInputLayout(ID3D11InputLayout* inputLayout)
{
	Direct3DDeviceIMContext->IASetInputLayout(inputLayout);
}

bool FD3D11Device::CreateVertexShader(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11VertexShader>& outVertexShader)
{
	if (vertexShaderBytecode.empty())
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex shader requires vertex shader bytecode");

		return false;
	}
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex shader requires an initialized D3D11 device");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11VertexShader> newVertexShader;
	const HRESULT result = Direct3DDevice->CreateVertexShader(
		vertexShaderBytecode.data(), vertexShaderBytecode.size(), nullptr, newVertexShader.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex shader failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outVertexShader = std::move(newVertexShader);

	return true;
}

void FD3D11Device::SetVertexShader(ID3D11VertexShader* vertexShader)
{
	Direct3DDeviceIMContext->VSSetShader(vertexShader, nullptr, 0);
}

bool FD3D11Device::CreatePixelShader(const TArray<uint8>& pixelShaderBytecode, Microsoft::WRL::ComPtr<ID3D11PixelShader>& outPixelShader)
{
	if (pixelShaderBytecode.empty())
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a pixel shader requires pixel shader bytecode");

		return false;
	}
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a pixel shader requires an initialized D3D11 device");

		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11PixelShader> newPixelShader;
	const HRESULT result = Direct3DDevice->CreatePixelShader(
		pixelShaderBytecode.data(), pixelShaderBytecode.size(), nullptr, newPixelShader.GetAddressOf());
	if (FAILED(result))
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a pixel shader failed (HRESULT 0x{:08X})", static_cast<uint32>(result));

		return false;
	}
	outPixelShader = std::move(newPixelShader);

	return true;
}

void FD3D11Device::SetPixelShader(ID3D11PixelShader* pixelShader)
{
	Direct3DDeviceIMContext->PSSetShader(pixelShader, nullptr, 0);
}

void FD3D11Device::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	Direct3DDeviceIMContext->IASetPrimitiveTopology(topology);
}

void FD3D11Device::Draw(uint32 vertexCount, uint32 startVertexLocation)
{
	Direct3DDeviceIMContext->Draw(vertexCount, startVertexLocation);
}

void FD3D11Device::SetRenderTargetView(ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView)
{
	ID3D11RenderTargetView* targets[] = {renderTargetView};
	Direct3DDeviceIMContext->OMSetRenderTargets(1, targets, depthStencilView);
}

void FD3D11Device::SetDepthStencilState(ID3D11DepthStencilState* depthStencilState, uint32 stencilReference)
{
	Direct3DDeviceIMContext->OMSetDepthStencilState(depthStencilState, stencilReference);
}

void FD3D11Device::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
	Direct3DDeviceIMContext->RSSetState(rasterizerState);
}

void FD3D11Device::SetViewport(const D3D11_VIEWPORT& viewport)
{
	Direct3DDeviceIMContext->RSSetViewports(1, &viewport);
}
