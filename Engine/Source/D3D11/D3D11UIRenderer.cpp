#include "D3D11UIRenderer.h"

#include "Core/Logging/Log.h"
#include "Core/Math/Matrix4.h"
#include "D3D11/D3D11Device.h"
#include "D3D11/D3D11TextureRenderData.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureAlphaRepresentation.h"
#include "UI/UIBatchData.h"

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

namespace
{
	// UIPixelShader의 PS b0와 같은 16바이트 배치다.
	struct FUITextureConstants
	{
		// 0은 Straight, 1은 Premultiplied 텍스처 입력을 뜻한다.
		uint32 sourcePremultiplied = 0;
		// HLSL 상수 버퍼 크기를 맞추며 전체 업로드 시 항상 0으로 초기화한다.
		uint32 padding[3]{};
	};

	static_assert(std::is_standard_layout_v<FUITextureConstants> && std::is_trivially_copyable_v<FUITextureConstants>);
	static_assert(sizeof(FUITextureConstants) == 16);
	static_assert(offsetof(FUITextureConstants, sourcePremultiplied) == 0);
	static_assert(offsetof(FUITextureConstants, padding) == 4);
}

bool FD3D11UIRenderer::Initialize(FD3D11Device& graphics, const TArray<uint8>& vertexShaderBytecode, const TArray<uint8>& pixelShaderBytecode)
{
	if (bInitialized)
	{
		WriteLog("D3D11", ELogLevel::Error, "UI renderer is already initialized.");
		return false;
	}
	if (!graphics.GetDevice() || !graphics.GetContext())
	{
		WriteLog("D3D11", ELogLevel::Error, "UI renderer initialization requires a prepared device and context.");
		return false;
	}
	if (vertexShaderBytecode.empty() || pixelShaderBytecode.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "UI renderer initialization requires nonempty vertex and pixel shader bytecode.");
		return false;
	}

	if (!graphics.CreateVertexShader(vertexShaderBytecode, vertexShader))
	{
		Shutdown();
		return false;
	}
	if (!graphics.CreatePixelShader(pixelShaderBytecode, pixelShader))
	{
		Shutdown();
		return false;
	}

	static_assert(std::is_standard_layout_v<FUIVertex> && std::is_trivially_copyable_v<FUIVertex>);
	const D3D11_INPUT_ELEMENT_DESC elements[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(offsetof(FUIVertex, position)), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(offsetof(FUIVertex, uv)), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, static_cast<UINT>(offsetof(FUIVertex, color)), D3D11_INPUT_PER_VERTEX_DATA, 0}};
	if (!graphics.CreateInputLayout(elements, vertexShaderBytecode, inputLayout))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI vertex shader, pixel shader and input layout created.");

	static_assert(sizeof(FMatrix4) == 64);
	FMatrix4 identity;
	identity.SetIdentity();
	constexpr uint32 TransformBufferByteSize = static_cast<uint32>(sizeof(FMatrix4));
	if (!graphics.CreateConstantBuffer(TransformBufferByteSize, transformConstantBuffer))
	{
		Shutdown();
		return false;
	}
	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &identity, TransformBufferByteSize))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI transform constant buffer initialized with an identity matrix ({} bytes).", TransformBufferByteSize);

	const FUITextureConstants initialConstants{};
	constexpr uint32 TextureBufferByteSize = static_cast<uint32>(sizeof(FUITextureConstants));
	if (!graphics.CreateConstantBuffer(TextureBufferByteSize, textureConstantBuffer))
	{
		Shutdown();
		return false;
	}
	if (!graphics.UpdateConstantBuffer(textureConstantBuffer.Get(), &initialConstants, TextureBufferByteSize))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI texture constant buffer initialized for Straight alpha ({} bytes).", TextureBufferByteSize);

	D3D11_BLEND_DESC blendDescription{};
	D3D11_RENDER_TARGET_BLEND_DESC& target = blendDescription.RenderTarget[0];
	target.BlendEnable = TRUE;
	// Straight RGB는 원본 알파를 곱하고 배경 RGB는 1-alpha를 곱해 더한다.
	target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	target.BlendOp = D3D11_BLEND_OP_ADD;
	// 알파는 원본 알파 + 배경 알파 * (1-원본 알파)로 합성한다.
	target.SrcBlendAlpha = D3D11_BLEND_ONE;
	target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (!graphics.CreateBlendState(blendDescription, straightBlendState))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI Straight alpha blend state created.");

	// Premultiplied RGB에는 알파가 이미 반영돼 있으므로 다시 곱하지 않는다.
	// 설명 구조체만 바꾸며 앞에서 생성한 Straight 상태 객체는 유지한다.
	target.SrcBlend = D3D11_BLEND_ONE;
	if (!graphics.CreateBlendState(blendDescription, premultipliedBlendState))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI Premultiplied alpha blend state created.");

	D3D11_DEPTH_STENCIL_DESC depthDescription{};
	// UI를 월드 깊이로 가리지 않고 기존 깊이 버퍼에도 쓰지 않는다.
	depthDescription.DepthEnable = FALSE;
	depthDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDescription.DepthFunc = D3D11_COMPARISON_LESS;
	depthDescription.StencilEnable = FALSE;
	depthDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDescription.BackFace = depthDescription.FrontFace;
	if (!graphics.CreateDepthStencilState(depthDescription, depthStencilState))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI depth state created with depth testing and writes disabled.");

	D3D11_SAMPLER_DESC samplerDescription{};
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.MaxAnisotropy = 1;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	if (!graphics.CreateSamplerState(samplerDescription, pointClampSamplerState))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI PointClamp sampler state created.");

	// Clamp 설정은 유지하고 주변 텍셀과 밉을 보간하는 별도 샘플러를 만든다.
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	if (!graphics.CreateSamplerState(samplerDescription, linearClampSamplerState))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI LinearClamp sampler state created.");
	// 모든 고정 자원을 준비한 경우에만 성공 상태로 전환한다. GPU 바인딩은 하지 않는다.
	bInitialized = true;
	return true;
}

void FD3D11UIRenderer::Shutdown()
{
	uiVertexBuffer.Reset();
	uiIndexBuffer.Reset();
	uiVertexBufferByteCapacity = 0;
	uiIndexBufferByteCapacity = 0;
	linearClampSamplerState.Reset();
	pointClampSamplerState.Reset();
	depthStencilState.Reset();

	premultipliedBlendState.Reset();
	straightBlendState.Reset();

	textureConstantBuffer.Reset();
	transformConstantBuffer.Reset();

	inputLayout.Reset();
	pixelShader.Reset();
	vertexShader.Reset();

	bInitialized = false;
}

bool FD3D11UIRenderer::UploadBatchData(FD3D11Device& graphics, const FUIBatchData& batchData)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Uploading UI geometry requires an initialized renderer.");
		return false;
	}
	if (batchData.vertices.empty() && batchData.indices.empty()) { return true; }
	if (batchData.vertices.empty())
	{
		WriteLog("Renderer", ELogLevel::Error, "UI indices require a nonempty vertex array.");
		return false;
	}
	if (batchData.indices.empty())
	{
		WriteLog("Renderer", ELogLevel::Error, "UI vertices require a nonempty index array.");
		return false;
	}

	static_assert(std::is_trivially_copyable_v<FUIVertex>);
	const size_t maxByteSize = std::numeric_limits<uint32>::max();
	if (batchData.vertices.size() > maxByteSize / sizeof(FUIVertex))
	{
		WriteLog("Renderer", ELogLevel::Error, "UI vertex data exceeds the supported buffer byte size.");
		return false;
	}
	if (batchData.indices.size() > maxByteSize / sizeof(uint32))
	{
		WriteLog("Renderer", ELogLevel::Error, "UI index data exceeds the supported buffer byte size.");
		return false;
	}

	// 곱셈과 32비트 변환은 범위를 먼저 확인한 뒤 수행한다.
	const uint32 vertexByteSize = static_cast<uint32>(batchData.vertices.size() * sizeof(FUIVertex));
	const uint32 indexByteSize = static_cast<uint32>(batchData.indices.size() * sizeof(uint32));
	if (!EnsureUIBufferCapacity(graphics, vertexByteSize, indexByteSize)) { return false; }
	if (!graphics.UpdateDynamicBuffer(uiVertexBuffer.Get(), batchData.vertices.data(), vertexByteSize)) { return false; }
	if (!graphics.UpdateDynamicBuffer(uiIndexBuffer.Get(), batchData.indices.data(), indexByteSize)) { return false; }
	return true;
}

uint32 FD3D11UIRenderer::CalculateUIBufferCapacity(uint32 currentCapacity, uint32 requiredBytes)
{
	if (requiredBytes <= currentCapacity) { return currentCapacity; }

	// 64비트에서 두 배를 계산하여 32비트 용량 곱셈이 넘치는 것을 막는다.
	uint64 nextCapacity = static_cast<uint64>(currentCapacity) * 2;
	if (nextCapacity < requiredBytes)
	{
		nextCapacity = requiredBytes;
	}
	if (nextCapacity > std::numeric_limits<uint32>::max())
	{
		nextCapacity = std::numeric_limits<uint32>::max();
	}
	return static_cast<uint32>(nextCapacity);
}

bool FD3D11UIRenderer::EnsureUIBufferCapacity(FD3D11Device& graphics, uint32 requiredVertexBytes, uint32 requiredIndexBytes)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Preparing UI buffers requires an initialized renderer.");
		return false;
	}

	const bool bGrowVertexBuffer = requiredVertexBytes > uiVertexBufferByteCapacity;
	const bool bGrowIndexBuffer = requiredIndexBytes > uiIndexBufferByteCapacity;
	Microsoft::WRL::ComPtr<ID3D11Buffer> newVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> newIndexBuffer;
	uint32 newVertexCapacity = uiVertexBufferByteCapacity;
	uint32 newIndexCapacity = uiIndexBufferByteCapacity;

	if (bGrowVertexBuffer)
	{
		newVertexCapacity = CalculateUIBufferCapacity(uiVertexBufferByteCapacity, requiredVertexBytes);
		if (!graphics.CreateDynamicBuffer(newVertexCapacity, D3D11_BIND_VERTEX_BUFFER, newVertexBuffer)) { return false; }
	}
	if (bGrowIndexBuffer)
	{
		newIndexCapacity = CalculateUIBufferCapacity(uiIndexBufferByteCapacity, requiredIndexBytes);
		if (!graphics.CreateDynamicBuffer(newIndexCapacity, D3D11_BIND_INDEX_BUFFER, newIndexBuffer)) { return false; }
	}

	// 인덱스 버퍼 준비가 실패해도 기존 정점 버퍼가 먼저 교체되지 않도록 둘 다 준비한 후 반영한다.
	if (bGrowVertexBuffer)
	{
		uiVertexBuffer = std::move(newVertexBuffer);
		uiVertexBufferByteCapacity = newVertexCapacity;
	}
	if (bGrowIndexBuffer)
	{
		uiIndexBuffer = std::move(newIndexBuffer);
		uiIndexBufferByteCapacity = newIndexCapacity;
	}
	return true;
}

bool FD3D11UIRenderer::PrepareDraw(FD3D11Device& graphics, int32 outputWidth, int32 outputHeight)
{
	if (!bInitialized)
	{
		WriteLog("D3D11", ELogLevel::Error, "Preparing UI drawing requires an initialized UI renderer.");
		return false;
	}
	if (!uiVertexBuffer || !uiIndexBuffer)
	{
		WriteLog("D3D11", ELogLevel::Error, "Preparing UI drawing requires uploaded vertex and index buffers.");
		return false;
	}
	if (outputWidth <= 0)
	{
		WriteLog("D3D11", ELogLevel::Error, "Preparing UI drawing requires a positive output width ({}).", outputWidth);
		return false;
	}
	if (outputHeight <= 0)
	{
		WriteLog("D3D11", ELogLevel::Error, "Preparing UI drawing requires a positive output height ({}).", outputHeight);
		return false;
	}

	// 행 벡터 (x,y,0,1)에 곱한다. 픽셀의 아래쪽 증가 Y를 클립의 위쪽 증가 Y로 바꾼다.
	FMatrix4 pixelToClip;
	pixelToClip.SetIdentity();
	pixelToClip.m[0][0] = 2.0f / static_cast<float>(outputWidth);
	pixelToClip.m[1][1] = -2.0f / static_cast<float>(outputHeight);
	pixelToClip.m[3][0] = -1.0f;
	pixelToClip.m[3][1] = 1.0f;
	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &pixelToClip, static_cast<uint32>(sizeof(FMatrix4))))
	{
		return false;
	}

	// 검사를 모두 통과한 뒤 공통 UI 상태와 업로드된 정점/인덱스 버퍼를 연결한다. 텍스처와 배치별 상태는 호출자가 이어서 연결한다.
	graphics.SetDepthStencilState(depthStencilState.Get());
	graphics.SetVertexShader(vertexShader.Get());
	graphics.SetPixelShader(pixelShader.Get());
	graphics.SetInputLayout(inputLayout.Get());
	graphics.SetVertexShaderConstantBuffer(0, transformConstantBuffer.Get());
	graphics.SetPixelShaderConstantBuffer(0, textureConstantBuffer.Get());
	graphics.SetVertexBuffer(uiVertexBuffer.Get(), static_cast<uint32>(sizeof(FUIVertex)));
	graphics.SetIndexBuffer(uiIndexBuffer.Get());
	graphics.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

bool FD3D11UIRenderer::SetBatchState(FD3D11Device& graphics, ETextureAlphaRepresentation alphaRepresentation, EUISampling sampling)
{
	if (!bInitialized)
	{
		WriteLog("D3D11", ELogLevel::Error, "Setting UI batch state requires an initialized UI renderer.");
		return false;
	}

	FUITextureConstants constants{};
	switch (alphaRepresentation)
	{
		case ETextureAlphaRepresentation::Straight:
			constants.sourcePremultiplied = 0;
			break;
		case ETextureAlphaRepresentation::Premultiplied:
			constants.sourcePremultiplied = 1;
			break;
		default:
			WriteLog("D3D11", ELogLevel::Error, "UI batch state requires a known texture alpha representation.");
			return false;
	}

	// 갱신 전에 지원 모드와 사용할 객체를 결정한다. 포인터는 이 호출 동안만 빌린다.
	ID3D11SamplerState* selectedSampler = nullptr;
	switch (sampling)
	{
		case EUISampling::PointClamp:
			selectedSampler = pointClampSamplerState.Get();
			break;
		case EUISampling::LinearClamp:
			selectedSampler = linearClampSamplerState.Get();
			break;
		default:
			WriteLog("D3D11", ELogLevel::Error, "UI batch state requires a supported sampling mode.");
			return false;
	}

	if (!graphics.UpdateConstantBuffer(textureConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(constants))))
	{
		return false;
	}

	// 모든 실패 검사를 마친 뒤에만 연결해 거부된 요청이 이전 상태를 덮지 않게 한다.
	graphics.SetBlendState(alphaRepresentation == ETextureAlphaRepresentation::Premultiplied ? premultipliedBlendState.Get() : straightBlendState.Get());
	graphics.SetPixelShaderSamplerState(selectedSampler);
	return true;
}

bool FD3D11UIRenderer::SubmitBatch(FD3D11Device& graphics, const FUIRenderBatch& batch)
{
	// DrawUI가 모든 자원을 먼저 검증했다. 제출이 끝날 때까지 자원과 메타데이터를 교체하지 않는다.
	const FD3D11TextureRenderData* textureData = static_cast<const FD3D11TextureRenderData*>(batch.texture->GetRenderData());
	if (!SetBatchState(graphics, textureData->GetAlphaRepresentation(), batch.sampling)) { return false; }
	graphics.SetPixelShaderResourceView(textureData->GetShaderResourceView());
	// 인덱스는 vertices 전체의 절대 번호이므로 배치별 baseVertex를 더하지 않는다.
	graphics.DrawIndexed(batch.indexCount, batch.firstIndex, 0);
	return true;
}
