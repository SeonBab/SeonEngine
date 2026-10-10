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

bool FD3D11Device::CreateVertexBuffer(const TArray<FPositionColorVertex>& vertices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outVertexBuffer)
{
	if (vertices.empty()) { return false; }
	if (!Direct3DDevice)
	{
		SE_LOG(LogD3D11RHI, Error, "Creating a vertex buffer requires an initialized D3D11 device");

		return false;
	}
	if (vertices.size() > std::numeric_limits<uint32>::max() / sizeof(FPositionColorVertex))
	{
		SE_LOG(LogD3D11RHI, Error, "Vertex buffer size exceeds the D3D11 ByteWidth range");

		return false;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<uint32>(vertices.size() * sizeof(FPositionColorVertex));
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
	constexpr uint32 stride = sizeof(FPositionColorVertex);
	constexpr uint32 offset = 0;

	Direct3DDeviceIMContext->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
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

	D3D11_INPUT_ELEMENT_DESC inputElements[2]{};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[0].AlignedByteOffset = static_cast<uint32>(offsetof(FPositionColorVertex, position));
	inputElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElements[1].SemanticName = "COLOR";
	inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElements[1].AlignedByteOffset = static_cast<uint32>(offsetof(FPositionColorVertex, color));
	inputElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> newInputLayout;
	const HRESULT result = Direct3DDevice->CreateInputLayout(
		inputElements, 2, vertexShaderBytecode.data(), vertexShaderBytecode.size(), newInputLayout.GetAddressOf());
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

void FD3D11Device::SetRenderTargetView(ID3D11RenderTargetView* renderTargetView)
{
	ID3D11RenderTargetView* targets[] = {renderTargetView};
	Direct3DDeviceIMContext->OMSetRenderTargets(1, targets, nullptr);
}

void FD3D11Device::SetViewport(const D3D11_VIEWPORT& viewport)
{
	Direct3DDeviceIMContext->RSSetViewports(1, &viewport);
}
