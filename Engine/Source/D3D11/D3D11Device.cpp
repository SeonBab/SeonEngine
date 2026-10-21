#include "D3D11Device.h"

#include "Core/Logging/Log.h"

#include <cstddef>
#include <limits>
#include <type_traits>
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
	// TODO(seon): 디버그 레이어 도입 시 설치 여부와 미설치 실패 정책을 정해 Debug 장치 생성에 반영한다.
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

void FD3D11Device::ClearRenderTargetView(ID3D11RenderTargetView* renderTargetView, const float color[4])
{
	context->ClearRenderTargetView(renderTargetView, color);
}

bool FD3D11Device::CreateVertexShader(const TArray<uint8>& bytecode, Microsoft::WRL::ComPtr<ID3D11VertexShader>& outShader)
{
	if (!device || bytecode.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a vertex shader requires an initialized device and nonempty bytecode.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11VertexShader> createdShader;
	const HRESULT result = device->CreateVertexShader(bytecode.data(), bytecode.size(), nullptr, createdShader.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateVertexShader failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outShader = std::move(createdShader);

	return true;
}

bool FD3D11Device::CreatePixelShader(const TArray<uint8>& bytecode, Microsoft::WRL::ComPtr<ID3D11PixelShader>& outShader)
{
	if (!device || bytecode.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a pixel shader requires an initialized device and nonempty bytecode.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11PixelShader> createdShader;
	const HRESULT result = device->CreatePixelShader(bytecode.data(), bytecode.size(), nullptr, createdShader.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreatePixelShader failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outShader = std::move(createdShader);

	return true;
}

void FD3D11Device::SetVertexShader(ID3D11VertexShader* shader)
{
	context->VSSetShader(shader, nullptr, 0);
}

void FD3D11Device::SetPixelShader(ID3D11PixelShader* shader)
{
	context->PSSetShader(shader, nullptr, 0);
}

void FD3D11Device::SetRenderTarget(ID3D11RenderTargetView* renderTargetView)
{
	if (renderTargetView == nullptr)
	{
		context->OMSetRenderTargets(0, nullptr, nullptr);
		return;
	}

	context->OMSetRenderTargets(1, &renderTargetView, nullptr);
}

void FD3D11Device::SetViewport(int width, int height)
{
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void FD3D11Device::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	context->IASetPrimitiveTopology(topology);
}

void FD3D11Device::Draw(uint32 vertexCount, uint32 startVertexLocation)
{
	context->Draw(vertexCount, startVertexLocation);
}

void FD3D11Device::DrawIndexed(uint32 indexCount, uint32 startIndexLocation, int32 baseVertexLocation)
{
	context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

bool FD3D11Device::CreateVertexBuffer(const TArray<FVertex>& vertices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer)
{
	if (!device || vertices.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a vertex buffer requires an initialized device and nonempty vertices.");
		return false;
	}
	if (vertices.size() > std::numeric_limits<UINT>::max() / sizeof(FVertex))
	{
		WriteLog("D3D11", ELogLevel::Error, "Vertex buffer size exceeds the D3D11 ByteWidth range.");
		return false;
	}

	D3D11_BUFFER_DESC description{};
	description.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(FVertex));
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = vertices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> createdBuffer;
	const HRESULT result = device->CreateBuffer(&description, &initialData, createdBuffer.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateBuffer(vertex buffer) failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outBuffer = std::move(createdBuffer);

	return true;
}

bool FD3D11Device::CreateIndexBuffer(const TArray<uint32>& indices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer)
{
	if (!device || indices.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating an index buffer requires an initialized device and nonempty indices.");
		return false;
	}
	if (indices.size() > std::numeric_limits<UINT>::max() / sizeof(uint32))
	{
		WriteLog("D3D11", ELogLevel::Error, "Index buffer size exceeds the D3D11 ByteWidth range.");
		return false;
	}

	D3D11_BUFFER_DESC description{};
	description.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32));
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = indices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> createdBuffer;
	const HRESULT result = device->CreateBuffer(&description, &initialData, createdBuffer.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateBuffer(index buffer) failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outBuffer = std::move(createdBuffer);

	return true;
}

bool FD3D11Device::CreateInputLayout(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11InputLayout>& outLayout)
{
	if (!device || vertexShaderBytecode.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating an input layout requires an initialized device and nonempty vertex shader bytecode.");
		return false;
	}

	static_assert(std::is_standard_layout_v<FVertex> && std::is_trivially_copyable_v<FVertex>);
	D3D11_INPUT_ELEMENT_DESC elements[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, static_cast<UINT>(offsetof(FVertex, position)), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, static_cast<UINT>(offsetof(FVertex, color)), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, static_cast<UINT>(offsetof(FVertex, normal)), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(offsetof(FVertex, uv)), D3D11_INPUT_PER_VERTEX_DATA, 0}};

	Microsoft::WRL::ComPtr<ID3D11InputLayout> createdLayout;
	const HRESULT result = device->CreateInputLayout(elements, 4, vertexShaderBytecode.data(), vertexShaderBytecode.size(), createdLayout.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateInputLayout failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outLayout = std::move(createdLayout);

	return true;
}

void FD3D11Device::SetInputLayout(ID3D11InputLayout* layout)
{
	context->IASetInputLayout(layout);
}

void FD3D11Device::SetVertexBuffer(ID3D11Buffer* buffer)
{
	SetVertexBuffer(buffer, static_cast<uint32>(sizeof(FVertex)));
}

void FD3D11Device::SetVertexBuffer(ID3D11Buffer* buffer, uint32 stride)
{
	constexpr UINT Offset = 0;
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &Offset);
}

void FD3D11Device::SetIndexBuffer(ID3D11Buffer* buffer)
{
	context->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
}

bool FD3D11Device::CreateConstantBuffer(uint32 byteSize, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer)
{
	if (!device)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a constant buffer requires an initialized device.");
		return false;
	}
	if (byteSize == 0 || byteSize % 16 != 0)
	{
		WriteLog("D3D11", ELogLevel::Error, "Constant buffer size must be a nonzero multiple of 16 bytes (requested {}).", byteSize);
		return false;
	}

	constexpr uint32 MaxConstantBufferByteSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
	if (byteSize > MaxConstantBufferByteSize)
	{
		WriteLog("D3D11", ELogLevel::Error, "Constant buffer size exceeds 64 KiB (requested {}).", byteSize);
		return false;
	}

	D3D11_BUFFER_DESC description{};
	description.ByteWidth = byteSize;
	description.Usage = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Microsoft::WRL::ComPtr<ID3D11Buffer> createdBuffer;
	const HRESULT result = device->CreateBuffer(&description, nullptr, createdBuffer.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateBuffer(constant buffer) failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outBuffer = std::move(createdBuffer);

	return true;
}

bool FD3D11Device::UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data, uint32 byteSize)
{
	if (!device || !context)
	{
		WriteLog("D3D11", ELogLevel::Error, "Updating a constant buffer requires an initialized device and context.");
		return false;
	}
	if (buffer == nullptr || data == nullptr)
	{
		WriteLog("D3D11", ELogLevel::Error, "Updating a constant buffer requires a buffer and CPU data.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> bufferDevice;
	buffer->GetDevice(bufferDevice.GetAddressOf());
	if (bufferDevice.Get() != device.Get())
	{
		WriteLog("D3D11", ELogLevel::Error, "Constant buffer belongs to another D3D11 device.");
		return false;
	}

	D3D11_BUFFER_DESC description{};
	buffer->GetDesc(&description);
	if (description.Usage != D3D11_USAGE_DEFAULT || description.BindFlags != D3D11_BIND_CONSTANT_BUFFER)
	{
		WriteLog("D3D11", ELogLevel::Error, "Updating a constant buffer requires a DEFAULT constant buffer.");
		return false;
	}
	if (byteSize != description.ByteWidth)
	{
		WriteLog("D3D11", ELogLevel::Error, "Constant buffer update size must match the entire buffer (requested {}, buffer {}).", byteSize, description.ByteWidth);
		return false;
	}

	// 상수 버퍼는 전체를 복사하며 이 API는 HRESULT를 반환하지 않는다.
	context->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);

	return true;
}

void FD3D11Device::SetVertexShaderConstantBuffer(uint32 slot, ID3D11Buffer* buffer)
{
	context->VSSetConstantBuffers(slot, 1, &buffer);
}

void FD3D11Device::SetPixelShaderConstantBuffer(uint32 slot, ID3D11Buffer* buffer)
{
	context->PSSetConstantBuffers(slot, 1, &buffer);
}

bool FD3D11Device::CreateTexture2D(uint32 width, uint32 height, const TArray<uint8>& rgbaPixels, Microsoft::WRL::ComPtr<ID3D11Texture2D>& outTexture)
{
	if (!device)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a texture requires an initialized device.");
		return false;
	}
	if (width == 0 || height == 0 || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
	{
		WriteLog("D3D11", ELogLevel::Error, "Texture dimensions are outside the D3D11 2D range.");
		return false;
	}

	// 크기 상한을 먼저 검사하므로 행과 전체 RGBA8 바이트 수가 넘치지 않는다.
	const uint32 rowPitch = width * 4;
	const std::size_t byteSize = static_cast<std::size_t>(rowPitch) * height;
	if (rgbaPixels.size() != byteSize)
	{
		WriteLog("D3D11", ELogLevel::Error, "Texture RGBA8 data size does not match its dimensions.");
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

	Microsoft::WRL::ComPtr<ID3D11Texture2D> createdTexture;
	const HRESULT result = device->CreateTexture2D(&description, &initialData, createdTexture.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateTexture2D failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outTexture = std::move(createdTexture);

	return true;
}

bool FD3D11Device::CreateShaderResourceView(ID3D11Texture2D* texture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& outView)
{
	if (!device)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a shader resource view requires an initialized device.");
		return false;
	}
	if (texture == nullptr)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a shader resource view requires a texture.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> textureDevice;
	texture->GetDevice(textureDevice.GetAddressOf());
	if (textureDevice.Get() != device.Get())
	{
		WriteLog("D3D11", ELogLevel::Error, "Shader resource texture belongs to another D3D11 device.");
		return false;
	}

	// nullptr 설명은 텍스처 형식과 전체 범위에 맞는 기본 뷰를 요청한다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> createdView;
	const HRESULT result = device->CreateShaderResourceView(texture, nullptr, createdView.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateShaderResourceView failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outView = std::move(createdView);

	return true;
}

bool FD3D11Device::CreateSamplerState(const D3D11_SAMPLER_DESC& description, Microsoft::WRL::ComPtr<ID3D11SamplerState>& outState)
{
	if (!device)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a sampler state requires an initialized device.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> createdState;
	const HRESULT result = device->CreateSamplerState(&description, createdState.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateSamplerState failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outState = std::move(createdState);

	return true;
}

void FD3D11Device::SetPixelShaderResourceView(ID3D11ShaderResourceView* view)
{
	context->PSSetShaderResources(0, 1, &view);
}

void FD3D11Device::SetPixelShaderSamplerState(ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}

bool FD3D11Device::CreateBlendState(const D3D11_BLEND_DESC& description, Microsoft::WRL::ComPtr<ID3D11BlendState>& outState)
{
	if (!device)
	{
		WriteLog("D3D11", ELogLevel::Error, "Creating a blend state requires an initialized device.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11BlendState> createdState;
	const HRESULT result = device->CreateBlendState(&description, createdState.GetAddressOf());
	if (FAILED(result))
	{
		WriteLog("D3D11", ELogLevel::Error, "CreateBlendState failed (HRESULT 0x{:08X})", static_cast<unsigned long>(result));
		return false;
	}

	outState = std::move(createdState);

	return true;
}

void FD3D11Device::SetBlendState(ID3D11BlendState* state)
{
	context->OMSetBlendState(state, nullptr, 0xffffffff);
}
