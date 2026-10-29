#include "D3D11Renderer.h"

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Log.h"
#include "Core/Math/Matrix4.h"
#include "Core/Misc/Paths.h"
#include "Core/Templates/SharedPtr.h"
#include "D3D11/D3D11FontRenderData.h"
#include "D3D11/D3D11ImGuiRendererBackend.h"
#include "D3D11/D3D11Shader.h"
#include "D3D11/D3D11StaticMeshRenderData.h"
#include "D3D11/D3D11TextureRenderData.h"
#include "Image/Image.h"
#include "Renderer/Font.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/StaticMeshData.h"
#include "Renderer/Texture.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureRenderTarget2D.h"
#include "UI/UIBatchData.h"
#include "UI/UIDrawElementList.h"
#include "UI/UIElementBatcher.h"

#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SpriteFont.h>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace
{
	struct FTransformConstants
	{
		FMatrix4 localToClip;
		FMatrix4 normalToWorld;
	};

	static_assert(sizeof(FTransformConstants) == 128);

	// StaticMeshPixelShader의 PS b1과 같은 16바이트 배치다. GPU 플래그는 C++ bool 대신 uint32를 쓴다.
	struct FAlphaMaskConstants
	{
		uint32 bAlphaMasked = 0;
		float alphaCutoff = 0.5f;
		uint32 padding[2]{};
	};

	static_assert(std::is_standard_layout_v<FAlphaMaskConstants> && std::is_trivially_copyable_v<FAlphaMaskConstants>);
	static_assert(sizeof(FAlphaMaskConstants) == 16);
	static_assert(offsetof(FAlphaMaskConstants, bAlphaMasked) == 0);
	static_assert(offsetof(FAlphaMaskConstants, alphaCutoff) == 4);
	static_assert(offsetof(FAlphaMaskConstants, padding) == 8);

	struct FDirectionalLightConstants
	{
		FVector3 direction;
		float directionPadding = 0.0f;
		FVector3 color; // 색에 세기를 곱한 RGB
		float colorPadding = 0.0f;
	};

	static_assert(sizeof(FDirectionalLightConstants) == 32);
	static_assert(offsetof(FDirectionalLightConstants, color) == 16);

	struct FSkyLightConstants
	{
		FVector3 color; // SkyLight에서 받은 세기 반영 균일 주변광 RGB
		float padding = 0.0f;
	};

	static_assert(sizeof(FSkyLightConstants) == 16);
	static_assert(offsetof(FSkyLightConstants, color) == 0);
	static_assert(offsetof(FSkyLightConstants, padding) == 12);

	struct FSpecularConstants
	{
		FMatrix4 localToWorld;
		FVector3 viewLocation{};
		float viewLocationPadding = 0.0f;
		// 스케일 없는 WorldToView의 세 번째 열을 반전한 월드 방향이다. 직교 시 표면에서 카메라를 향한다.
		FVector3 orthographicViewDirection{0.0f, 0.0f, -1.0f};
		// HLSL uint와 맞춘 32비트 플래그다. 0은 원근, 1은 직교다.
		uint32 bOrthographic = 0;
		FVector3 specularColor{};
		float shininess = 32.0f;
	};

	static_assert(sizeof(FSpecularConstants) == 112);
	static_assert(offsetof(FSpecularConstants, localToWorld) == 0);
	static_assert(offsetof(FSpecularConstants, viewLocation) == 64);
	static_assert(offsetof(FSpecularConstants, viewLocationPadding) == 76);
	static_assert(offsetof(FSpecularConstants, orthographicViewDirection) == 80);
	static_assert(offsetof(FSpecularConstants, bOrthographic) == 92);
	static_assert(offsetof(FSpecularConstants, specularColor) == 96);
	static_assert(offsetof(FSpecularConstants, shininess) == 108);

	bool ValidateTriangleMesh(size_t vertexCount, const TArray<uint32>& indices)
	{
		// TODO(seon): 인덱스 없는 삼각형 목록 지원 시 정점 개수 검증과 GPU 생성과 Draw 경로를 함께 추가한다.
		if (vertexCount == 0 || indices.empty() || indices.size() % 3 != 0)
		{
			WriteLog("Renderer", ELogLevel::Error, "A mesh requires vertices and a nonempty triangle-list index array.");
			return false;
		}
		for (const uint32 vertexIndex : indices)
		{
			if (vertexIndex >= vertexCount)
			{
				WriteLog("Renderer", ELogLevel::Error, "Mesh index {} is outside the vertex array ({} vertices).", vertexIndex, vertexCount);
				return false;
			}
		}

		return true;
	}
}

#pragma region Lifecycle

FD3D11Renderer::FD3D11Renderer() = default;

FD3D11Renderer::~FD3D11Renderer() = default;

bool FD3D11Renderer::Initialize(void* nativeWindowHandle, int32 width, int32 height)
{
	const HWND windowHandle = static_cast<HWND>(nativeWindowHandle);
	if (bInitialized || graphics.GetDevice())
	{
		WriteLog("Renderer", ELogLevel::Error, "Renderer is already initialized.");
		return false;
	}
	if (windowHandle == nullptr || width <= 0 || height <= 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Renderer initialization requires a window and positive dimensions.");
		return false;
	}

	const std::optional<std::filesystem::path> baseDir = FPaths::BaseDir();
	if (!baseDir)
	{
		return false;
	}

	const std::filesystem::path shaderDir = *baseDir / L"Shaders";
	TArray<uint8> staticMeshVertexShaderBytecode;
	TArray<uint8> staticMeshPixelShaderBytecode;
	TArray<uint8> uiVertexShaderBytecode;
	TArray<uint8> uiPixelShaderBytecode;
	if (!ReadShaderBytecode(shaderDir / L"StaticMeshVertexShader.cso", staticMeshVertexShaderBytecode) ||
		!ReadShaderBytecode(shaderDir / L"StaticMeshPixelShader.cso", staticMeshPixelShaderBytecode))
	{
		return false;
	}

	if (!ReadShaderBytecode(shaderDir / L"UIVertexShader.cso", uiVertexShaderBytecode))
	{
		return false;
	}

	if (!ReadShaderBytecode(shaderDir / L"UIPixelShader.cso", uiPixelShaderBytecode))
	{
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "UI pixel shader bytecode loaded ({} bytes).", uiPixelShaderBytecode.size());
	WriteLog("D3D11", ELogLevel::Info, "UI vertex shader bytecode loaded ({} bytes).", uiVertexShaderBytecode.size());
	WriteLog("D3D11", ELogLevel::Info, "Vertex shader bytecode loaded ({} bytes).", staticMeshVertexShaderBytecode.size());
	WriteLog("D3D11", ELogLevel::Info, "Static mesh pixel shader bytecode loaded ({} bytes).", staticMeshPixelShaderBytecode.size());
	if (!graphics.Initialize())
	{
		return false;
	}
	if (!graphics.CreateVertexShader(staticMeshVertexShaderBytecode, staticMeshVertexShader) ||
		!graphics.CreatePixelShader(staticMeshPixelShaderBytecode, staticMeshPixelShader))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Vertex and pixel shaders created.");
	if (!graphics.CreateInputLayout(staticMeshVertexShaderBytecode, inputLayout))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Vertex input layout created.");

	if (!uiRenderer.Initialize(graphics, uiVertexShaderBytecode, uiPixelShaderBytecode))
	{
		Shutdown();
		return false;
	}

	static_assert(sizeof(FMatrix4) == 64);
	constexpr uint32 TransformBufferByteSize = static_cast<uint32>(sizeof(FTransformConstants));
	FMatrix4 initialLocalToWorld;
	initialLocalToWorld.SetIdentity();
	FMatrix4 initialWorldToClip;
	initialWorldToClip.SetIdentity();
	FMatrix4 identity;
	identity.SetIdentity();

	const FTransformConstants initialWorldConstants{initialLocalToWorld * initialWorldToClip, identity};
	if (!graphics.CreateConstantBuffer(TransformBufferByteSize, transformConstantBuffer) ||
		!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &initialWorldConstants, TransformBufferByteSize))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Transform constant buffer initialized.");

	constexpr uint32 DirectionalLightBufferByteSize = static_cast<uint32>(sizeof(FDirectionalLightConstants));

	const FDirectionalLightConstants initialLightConstants{};

	if (!graphics.CreateConstantBuffer(DirectionalLightBufferByteSize, directionalLightConstantBuffer) ||
		!graphics.UpdateConstantBuffer(directionalLightConstantBuffer.Get(), &initialLightConstants, DirectionalLightBufferByteSize))
	{
		Shutdown();
		return false;
	}

	constexpr uint32 SkyLightBufferByteSize = static_cast<uint32>(sizeof(FSkyLightConstants));
	const FSkyLightConstants initialSkyLightConstants{};
	if (!graphics.CreateConstantBuffer(SkyLightBufferByteSize, skyLightConstantBuffer) ||
		!graphics.UpdateConstantBuffer(skyLightConstantBuffer.Get(), &initialSkyLightConstants, SkyLightBufferByteSize))
	{
		Shutdown();
		return false;
	}

	constexpr uint32 SpecularBufferByteSize = static_cast<uint32>(sizeof(FSpecularConstants));
	FSpecularConstants initialSpecularConstants;
	initialSpecularConstants.localToWorld.SetIdentity();
	if (!graphics.CreateConstantBuffer(SpecularBufferByteSize, specularConstantBuffer) ||
		!graphics.UpdateConstantBuffer(specularConstantBuffer.Get(), &initialSpecularConstants, SpecularBufferByteSize))
	{
		Shutdown();
		return false;
	}

	constexpr uint32 AlphaMaskBufferByteSize = static_cast<uint32>(sizeof(FAlphaMaskConstants));
	const FAlphaMaskConstants initialAlphaMaskConstants{};
	if (!graphics.CreateConstantBuffer(AlphaMaskBufferByteSize, alphaMaskConstantBuffer) ||
		!graphics.UpdateConstantBuffer(alphaMaskConstantBuffer.Get(), &initialAlphaMaskConstants, AlphaMaskBufferByteSize))
	{
		Shutdown();
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthDescription{};
	depthDescription.DepthEnable = TRUE;
	depthDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDescription.DepthFunc = D3D11_COMPARISON_LESS;
	depthDescription.StencilEnable = FALSE;
	depthDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDescription.BackFace = depthDescription.FrontFace;

	if (!graphics.CreateDepthStencilState(depthDescription, worldDepthStencilState))
	{
		Shutdown();
		return false;
	}

	D3D11_SAMPLER_DESC samplerDescription{};
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.MaxAnisotropy = 1;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	for (size_t u = 0; u < samplerStates.size(); ++u)
	{
		for (size_t v = 0; v < samplerStates[u].size(); ++v)
		{
			samplerDescription.AddressU = u == 0 ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDescription.AddressV = v == 0 ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
			if (!graphics.CreateSamplerState(samplerDescription, samplerStates[u][v]))
			{
				Shutdown();
				return false;
			}
		}
	}

	WriteLog("D3D11", ELogLevel::Info, "Four point samplers for U/V clamp and wrap created.");
	const TArray<uint8> whitePixel{255, 255, 255, 255};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> whiteTexture;
	if (!graphics.CreateTexture2D(1, 1, whitePixel, whiteTexture) ||
		!graphics.CreateShaderResourceView(whiteTexture.Get(), whiteTextureView))
	{
		Shutdown();
		return false;
	}
	if (!swapChain.Initialize(graphics.GetDevice(), windowHandle))
	{
		Shutdown();
		return false;
	}

	if (!graphics.CreateDepthStencilBuffer(static_cast<uint32>(width), static_cast<uint32>(height), depthStencilTexture, depthStencilView))
	{
		Shutdown();
		return false;
	}

	bufferWidth = width;
	bufferHeight = height;
	outputWidth = width;
	outputHeight = height;
	localToWorld = initialLocalToWorld;
	worldToClip = initialWorldToClip;
	bInitialized = true;

	return true;
}

TUniquePtr<IImGuiRendererBackend> FD3D11Renderer::CreateImGuiRendererBackend()
{
	if (!bInitialized) { return nullptr; }
	return MakeUnique<FD3D11ImGuiRendererBackend>(graphics.GetDevice(), graphics.GetContext());
}

void FD3D11Renderer::Shutdown()
{
	if (graphics.GetContext())
	{
		graphics.SetBlendState(nullptr);
		graphics.SetDepthStencilState(nullptr);
		graphics.SetPixelShaderResourceView(nullptr);
		graphics.SetPixelShaderSamplerState(nullptr);
		graphics.SetIndexBuffer(nullptr);
		graphics.SetVertexShaderConstantBuffer(0, nullptr);
		graphics.SetVertexShaderConstantBuffer(1, nullptr);
		graphics.SetVertexShaderConstantBuffer(2, nullptr);
		graphics.SetVertexShaderConstantBuffer(3, nullptr);
		graphics.SetPixelShaderConstantBuffer(0, nullptr);
		graphics.SetVertexBuffer(nullptr);
		graphics.SetInputLayout(nullptr);
		graphics.SetRenderTarget(nullptr);
		graphics.SetPixelShader(nullptr);
		graphics.SetVertexShader(nullptr);
	}

	uiRenderer.Shutdown();
	activeRenderTargetResource.Reset();
	captureDepthStencilView.Reset();
	captureDepthStencilTexture.Reset();
	depthStencilView.Reset();
	depthStencilTexture.Reset();
	worldDepthStencilState.Reset();
	swapChain.Shutdown();
	for (std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>, 2>& samplersForU : samplerStates)
	{
		for (Microsoft::WRL::ComPtr<ID3D11SamplerState>& sampler : samplersForU)
		{
			sampler.Reset();
		}
	}
	whiteTextureView.Reset();
	transformConstantBuffer.Reset();
	directionalLightConstantBuffer.Reset();
	skyLightConstantBuffer.Reset();
	specularConstantBuffer.Reset();
	alphaMaskConstantBuffer.Reset();
	inputLayout.Reset();
	staticMeshPixelShader.Reset();
	staticMeshVertexShader.Reset();
	graphics.Shutdown();
	localToWorld = FMatrix4{};
	worldToClip = FMatrix4{};
	savedLocalToWorld = FMatrix4{};
	savedWorldToClip = FMatrix4{};
	captureDepthWidth = 0;
	captureDepthHeight = 0;
	bufferWidth = 0;
	bufferHeight = 0;
	outputWidth = 0;
	outputHeight = 0;
	bInitialized = false;
	bFrameActive = false;
	bRenderTargetActive = false;
}

#pragma endregion

#pragma region ResourceCreation

bool FD3D11Renderer::CreateMesh(const FStaticMeshData& meshData, TUniquePtr<FStaticMeshRenderData>& outMesh)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a mesh requires an initialized renderer.");
		return false;
	}
	if (!ValidateTriangleMesh(meshData.vertices.size(), meshData.indices)) { return false; }

	TUniquePtr<FD3D11StaticMeshRenderData> newMesh = MakeUnique<FD3D11StaticMeshRenderData>();
	if (!graphics.CreateVertexBuffer(meshData.vertices, newMesh->vertexBuffer) ||
		!graphics.CreateIndexBuffer(meshData.indices, newMesh->indexBuffer))
	{
		return false;
	}

	// 두 형상 버퍼가 모두 준비된 뒤에만 기존 메시를 교체한다.
	newMesh->indexCount = static_cast<uint32>(meshData.indices.size());
	outMesh = std::move(newMesh);

	return true;
}

bool FD3D11Renderer::CreateTexture2D(const FImage& image, UTexture2D& outTexture)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a texture requires an initialized renderer.");
		return false;
	}
	if (image.sizeX <= 0 || image.sizeY <= 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "A texture image requires positive dimensions.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	TUniquePtr<FD3D11TextureRenderData> newTexture = MakeUnique<FD3D11TextureRenderData>(image.sizeX, image.sizeY);
	if (!graphics.CreateTexture2D(static_cast<uint32>(image.sizeX), static_cast<uint32>(image.sizeY), image.rawData, texture)) { return false; }
	if (!graphics.CreateShaderResourceView(texture.Get(), newTexture->textureView)) { return false; }
	if (!graphics.GenerateMips(newTexture->textureView.Get())) { return false; }

	outTexture.SetRenderData(std::move(newTexture));

	return true;
}

bool FD3D11Renderer::CreateTexture2DFromDDS(std::span<const uint8> ddsBytes, UTexture2D& outTexture)
{
	if (!bInitialized || ddsBytes.empty())
	{
		WriteLog("Renderer", ELogLevel::Error, "DDS texture creation requires an initialized renderer and nonempty bytes.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
	DirectX::DDS_ALPHA_MODE alphaMode = DirectX::DDS_ALPHA_MODE_UNKNOWN;
	// 컨텍스트 없는 오버로드는 DDS의 밉만 사용하며 자동 밉 생성이나 픽셀 변환을 하지 않는다.
	const HRESULT result = DirectX::CreateDDSTextureFromMemory(graphics.GetDevice(), ddsBytes.data(), ddsBytes.size(), resource.GetAddressOf(), view.GetAddressOf(), 0, &alphaMode);
	if (FAILED(result))
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a DDS texture failed (HRESULT 0x{:08X}).", static_cast<unsigned long>(result));
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	if (FAILED(resource.As(&texture)))
	{
		WriteLog("Renderer", ELogLevel::Error, "DDS texture creation requires a 2D resource.");
		return false;
	}
	D3D11_TEXTURE2D_DESC description{};
	texture->GetDesc(&description);
	if (description.ArraySize != 1 || (description.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0 || alphaMode == DirectX::DDS_ALPHA_MODE_CUSTOM)
	{
		WriteLog("Renderer", ELogLevel::Error, "DDS texture arrays, cubes and custom alpha modes are not supported.");
		return false;
	}

	switch (description.Format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			break;
		default:
			WriteLog("Renderer", ELogLevel::Error, "DDS texture creation requires supported RGBA/BGRA/BGRX8 or BC1/BC2/BC3 color data.");
			return false;
	}

	TUniquePtr<FD3D11TextureRenderData> newTexture = MakeUnique<FD3D11TextureRenderData>(static_cast<int32>(description.Width), static_cast<int32>(description.Height));
	newTexture->textureView = std::move(view);
	newTexture->SetAlphaRepresentation(alphaMode == DirectX::DDS_ALPHA_MODE_PREMULTIPLIED ? ETextureAlphaRepresentation::Premultiplied : ETextureAlphaRepresentation::Straight);
	outTexture.SetRenderData(std::move(newTexture));

	return true;
}

bool FD3D11Renderer::CreateFontResource(std::span<const uint8> fontData, UFont& outFont)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a font requires an initialized renderer.");
		return false;
	}
	if (fontData.empty())
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a font requires non-empty font data.");
		return false;
	}

	TUniquePtr<FD3D11FontRenderData> newFont = MakeUnique<FD3D11FontRenderData>();
	newFont->spriteFont = MakeUnique<DirectX::SpriteFont>(graphics.GetDevice(), fontData.data(), fontData.size());

	// GetSpriteSheet가 반환한 COM 참조를 같은 SRV의 엔진 텍스처 데이터로 넘긴다. GPU 픽셀은 복제하지 않는다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> atlasView;
	newFont->spriteFont->GetSpriteSheet(atlasView.GetAddressOf());
	if (!atlasView)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a font requires an atlas shader resource view.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Resource> atlasResource;
	atlasView->GetResource(atlasResource.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11Texture2D> atlasTexture;
	const HRESULT atlasResult = atlasResource.As(&atlasTexture);
	if (FAILED(atlasResult))
	{
		WriteLog("Renderer", ELogLevel::Error, "Font atlas must be a 2D texture (HRESULT 0x{:08X}).", static_cast<unsigned long>(atlasResult));
		return false;
	}

	D3D11_TEXTURE2D_DESC atlasDescription{};
	atlasTexture->GetDesc(&atlasDescription);
	if (atlasDescription.Width == 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Font atlas width must be positive.");
		return false;
	}
	if (atlasDescription.Height == 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Font atlas height must be positive.");
		return false;
	}
	if (atlasDescription.Width > static_cast<UINT>(std::numeric_limits<int32>::max()))
	{
		WriteLog("Renderer", ELogLevel::Error, "Font atlas width exceeds the supported integer range.");
		return false;
	}
	if (atlasDescription.Height > static_cast<UINT>(std::numeric_limits<int32>::max()))
	{
		WriteLog("Renderer", ELogLevel::Error, "Font atlas height exceeds the supported integer range.");
		return false;
	}

	TUniquePtr<FD3D11TextureRenderData> atlasData = MakeUnique<FD3D11TextureRenderData>(static_cast<int32>(atlasDescription.Width), static_cast<int32>(atlasDescription.Height));
	atlasData->textureView = std::move(atlasView);
	// 입력 .spritefont의 준비 계약을 기록한다. 픽셀 변환이나 자동 추론은 하지 않는다.
	atlasData->SetAlphaRepresentation(ETextureAlphaRepresentation::Premultiplied);
	newFont->atlas = MakeUnique<UTexture2D>();
	newFont->atlas->SetRenderData(std::move(atlasData));

	// 폰트와 공통 아틀라스가 모두 준비된 뒤에만 기존 출력을 교체한다.
	outFont.SetRenderData(std::move(newFont));
	return true;
}

bool FD3D11Renderer::CreateRenderTarget2D(int32 width, int32 height, UTextureRenderTarget2D& outTexture)
{
	if (!bInitialized || bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a render target requires an initialized renderer outside an active frame.");
		return false;
	}
	if (width <= 0 || height <= 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "A render target requires positive dimensions.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	TUniquePtr<FD3D11TextureRenderData> newTexture = MakeUnique<FD3D11TextureRenderData>(width, height);
	if (!graphics.CreateRenderTargetTexture2D(static_cast<uint32>(width), static_cast<uint32>(height), texture, newTexture->renderTargetView)) { return false; }
	if (!graphics.CreateShaderResourceView(texture.Get(), newTexture->textureView)) { return false; }

	const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
	graphics.ClearRenderTargetView(newTexture->renderTargetView.Get(), clearColor);

	newTexture->SetAlphaRepresentation(ETextureAlphaRepresentation::Straight);
	// 두 뷰와 실제 크기를 함께 교체해 공유 객체의 주소와 기존 주소 설정을 유지한다.
	outTexture.SetRenderData(std::move(newTexture));

	return true;
}

#pragma endregion

#pragma region ViewTransform

bool FD3D11Renderer::SetLocalToWorld(const FMatrix4& matrix)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting LocalToWorld requires an initialized renderer.");
		return false;
	}

	const FTransformConstants constants{matrix * worldToClip, matrix.Inverse().Transpose()};

	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(constants))))
	{
		return false;
	}
	localToWorld = matrix;

	return true;
}

bool FD3D11Renderer::SetWorldToClip(const FMatrix4& matrix)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting WorldToClip requires an initialized renderer.");
		return false;
	}

	const FTransformConstants constants{localToWorld * matrix, localToWorld.Inverse().Transpose()};
	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(FTransformConstants))))
	{
		return false;
	}
	worldToClip = matrix;

	return true;
}

#pragma endregion

#pragma region Lighting

bool FD3D11Renderer::SetDirectionalLight(FVector3 lightDirection, FVector3 lightColor)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting a directional light requires an initialized renderer.");
		return false;
	}

	const FDirectionalLightConstants constants{.direction = lightDirection, .color = lightColor};

	return graphics.UpdateConstantBuffer(directionalLightConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(constants)));
}

bool FD3D11Renderer::SetSkyLight(FVector3 lightColor)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting a sky light requires an initialized renderer.");
		return false;
	}

	const FSkyLightConstants constants{.color = lightColor};

	return graphics.UpdateConstantBuffer(skyLightConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(constants)));
}

#pragma endregion

#pragma region Frame

bool FD3D11Renderer::BeginFrame(int32 width, int32 height)
{
	if (!bInitialized || bFrameActive || width <= 0 || height <= 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Beginning a frame requires an initialized renderer, positive dimensions and no active frame.");
		return false;
	}
	if (width != bufferWidth || height != bufferHeight)
	{
		graphics.SetRenderTarget(nullptr);

		if (!swapChain.Resize(graphics.GetDevice(), width, height))
		{
			return false;
		}

		if (!graphics.CreateDepthStencilBuffer(static_cast<uint32>(width), static_cast<uint32>(height), depthStencilTexture, depthStencilView))
		{
			return false;
		}

		bufferWidth = width;
		bufferHeight = height;
	}

	const float clearColor[] = {0.05f, 0.10f, 0.20f, 1.0f};

	graphics.SetRenderTarget(swapChain.GetBackBufferRenderTargetView(), depthStencilView.Get());
	graphics.SetViewport(bufferWidth, bufferHeight);
	outputWidth = bufferWidth;
	outputHeight = bufferHeight;
	graphics.SetDepthStencilState(worldDepthStencilState.Get());

	graphics.ClearRenderTargetView(swapChain.GetBackBufferRenderTargetView(), clearColor);
	graphics.ClearDepthStencilView(depthStencilView.Get(), 1.0f);

	bFrameActive = true;

	return true;
}

bool FD3D11Renderer::BeginRenderTarget(const UTextureRenderTarget2D& target)
{
	if (!bInitialized || !bFrameActive || bRenderTargetActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Beginning a render target requires an initialized renderer, an active frame and no active render target.");
		return false;
	}

	const FD3D11TextureRenderData* targetData = dynamic_cast<const FD3D11TextureRenderData*>(target.GetRenderData());
	if (!targetData || !targetData->textureView || !targetData->renderTargetView || target.GetWidth() <= 0 || target.GetHeight() <= 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Beginning a render target requires D3D11 texture views and positive dimensions.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> shaderResourceDevice;
	Microsoft::WRL::ComPtr<ID3D11Device> renderTargetDevice;
	targetData->textureView->GetDevice(shaderResourceDevice.GetAddressOf());
	targetData->renderTargetView->GetDevice(renderTargetDevice.GetAddressOf());
	if (shaderResourceDevice.Get() != graphics.GetDevice() || renderTargetDevice.Get() != graphics.GetDevice())
	{
		WriteLog("Renderer", ELogLevel::Error, "Beginning a render target requires texture views from the current renderer device.");
		return false;
	}

	const int32 targetWidth = target.GetWidth();
	const int32 targetHeight = target.GetHeight();

	if (!captureDepthStencilTexture || !captureDepthStencilView || captureDepthWidth != targetWidth || captureDepthHeight != targetHeight)
	{
		if (!graphics.CreateDepthStencilBuffer(static_cast<uint32>(targetWidth), static_cast<uint32>(targetHeight), captureDepthStencilTexture, captureDepthStencilView))
		{
			return false;
		}

		captureDepthWidth = targetWidth;
		captureDepthHeight = targetHeight;
	}

	savedLocalToWorld = localToWorld;
	savedWorldToClip = worldToClip;

	targetData->renderTargetView->GetResource(activeRenderTargetResource.ReleaseAndGetAddressOf());

	graphics.SetPixelShaderResourceView(nullptr);

	graphics.SetRenderTarget(targetData->renderTargetView.Get(), captureDepthStencilView.Get());
	graphics.SetViewport(targetWidth, targetHeight);
	outputWidth = targetWidth;
	outputHeight = targetHeight;
	graphics.SetDepthStencilState(worldDepthStencilState.Get());
	graphics.SetBlendState(nullptr);

	const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
	graphics.ClearRenderTargetView(targetData->renderTargetView.Get(), clearColor);
	graphics.ClearDepthStencilView(captureDepthStencilView.Get(), 1.0f);

	bRenderTargetActive = true;

	return true;
}

bool FD3D11Renderer::EndRenderTarget()
{
	if (!bInitialized || !bFrameActive || !bRenderTargetActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Ending a render target requires an initialized renderer, an active frame and an active render target.");
		return false;
	}

	const FTransformConstants constants{savedLocalToWorld * savedWorldToClip, savedLocalToWorld.Inverse().Transpose()};

	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &constants, static_cast<uint32>(sizeof(constants))))
	{
		return false;
	}

	graphics.SetRenderTarget(swapChain.GetBackBufferRenderTargetView(), depthStencilView.Get());
	graphics.SetViewport(bufferWidth, bufferHeight);
	outputWidth = bufferWidth;
	outputHeight = bufferHeight;
	graphics.SetDepthStencilState(worldDepthStencilState.Get());
	graphics.SetBlendState(nullptr);

	localToWorld = savedLocalToWorld;
	worldToClip = savedWorldToClip;

	activeRenderTargetResource.Reset();

	bRenderTargetActive = false;

	return true;
}

bool FD3D11Renderer::DrawScene(const FSceneRenderData& data)
{
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing a scene requires an active frame.");
		return false;
	}

	if (!SetWorldToClip(data.view.worldToView * data.view.viewToClip) ||
		!SetDirectionalLight(data.lightDirection, data.lightColor) ||
		!SetSkyLight(data.skyLightColor))
	{
		return false;
	}

	for (const FMeshDrawItem& item : data.meshes)
	{
		if (!SetLocalToWorld(item.localToWorld) || !DrawMesh(item, data.view)) { return false; }
	}

	return true;
}

bool FD3D11Renderer::DrawMesh(const FMeshDrawItem& item, const FRenderView& view)
{
	const FD3D11StaticMeshRenderData* d3d11Mesh = dynamic_cast<const FD3D11StaticMeshRenderData*>(item.mesh);
	if (!d3d11Mesh)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires D3D11 mesh data.");
		return false;
	}
	if (!d3d11Mesh->vertexBuffer || !d3d11Mesh->indexBuffer || d3d11Mesh->indexCount == 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires a nonempty GPU mesh.");
		return false;
	}
	Microsoft::WRL::ComPtr<ID3D11Device> vertexDevice;
	Microsoft::WRL::ComPtr<ID3D11Device> indexDevice;
	d3d11Mesh->vertexBuffer->GetDevice(vertexDevice.GetAddressOf());
	d3d11Mesh->indexBuffer->GetDevice(indexDevice.GetAddressOf());
	if (vertexDevice.Get() != graphics.GetDevice() || indexDevice.Get() != graphics.GetDevice())
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires mesh buffers from the current renderer device.");
		return false;
	}

	// 요청이 보관한 공유 참조가 그리기 요청이 끝날 때까지 텍스처 객체를 유지한다.
	const TSharedPtr<UTexture>& texture = item.baseColorTexture;
	const FTextureRenderData* textureData = texture ? texture->GetRenderData() : nullptr;
	const FD3D11TextureRenderData* d3d11Texture = dynamic_cast<const FD3D11TextureRenderData*>(textureData);
	if (textureData && !d3d11Texture)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires D3D11 material texture data.");
		return false;
	}
	if (d3d11Texture && d3d11Texture->textureView)
	{
		Microsoft::WRL::ComPtr<ID3D11Device> textureDevice;
		d3d11Texture->textureView->GetDevice(textureDevice.GetAddressOf());
		if (textureDevice.Get() != graphics.GetDevice())
		{
			WriteLog("Renderer", ELogLevel::Error, "Drawing requires a material texture from the current renderer device.");
			return false;
		}

		if (IsActiveRenderTargetResource(d3d11Texture->textureView.Get()))
		{
			WriteLog("Renderer", ELogLevel::Error, "Drawing cannot sample the active render target.");
			return false;
		}
	}

	const FSpecularConstants specularConstants{
		.localToWorld = item.localToWorld,
		.viewLocation = view.location,
		.orthographicViewDirection = {-view.worldToView.m[0][2], -view.worldToView.m[1][2], -view.worldToView.m[2][2]},
		.bOrthographic = view.bOrthographic ? 1u : 0u,
		.specularColor = item.specularColor,
		.shininess = item.shininess};
	if (!graphics.UpdateConstantBuffer(specularConstantBuffer.Get(), &specularConstants, static_cast<uint32>(sizeof(specularConstants))))
	{
		return false;
	}

	const FAlphaMaskConstants alphaMaskConstants{
		.bAlphaMasked = item.bAlphaMasked ? 1u : 0u,
		.alphaCutoff = item.alphaCutoff};
	if (!graphics.UpdateConstantBuffer(alphaMaskConstantBuffer.Get(), &alphaMaskConstants, static_cast<uint32>(sizeof(alphaMaskConstants))))
	{
		return false;
	}

	graphics.SetVertexShader(staticMeshVertexShader.Get());
	graphics.SetPixelShader(staticMeshPixelShader.Get());
	graphics.SetInputLayout(inputLayout.Get());
	graphics.SetVertexBuffer(d3d11Mesh->vertexBuffer.Get());
	graphics.SetPixelShaderResourceView(d3d11Texture && d3d11Texture->textureView ? d3d11Texture->textureView.Get() : whiteTextureView.Get());
	const size_t addressU = texture && texture->GetTextureAddressX() == ETextureAddress::Wrap ? 1 : 0;
	const size_t addressV = texture && texture->GetTextureAddressY() == ETextureAddress::Wrap ? 1 : 0;
	graphics.SetPixelShaderSamplerState(samplerStates[addressU][addressV].Get());
	graphics.SetVertexShaderConstantBuffer(0, transformConstantBuffer.Get());
	graphics.SetVertexShaderConstantBuffer(1, directionalLightConstantBuffer.Get());
	graphics.SetVertexShaderConstantBuffer(2, skyLightConstantBuffer.Get());
	graphics.SetVertexShaderConstantBuffer(3, specularConstantBuffer.Get());
	graphics.SetPixelShaderConstantBuffer(1, alphaMaskConstantBuffer.Get());
	graphics.SetIndexBuffer(d3d11Mesh->indexBuffer.Get());
	graphics.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics.DrawIndexed(d3d11Mesh->indexCount, 0, 0);

	return true;
}

bool FD3D11Renderer::EndFrame()
{
	if (!bFrameActive || bRenderTargetActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Ending a frame requires an active frame and no active render target.");
		return false;
	}
	bFrameActive = false;

	return swapChain.Present();
}

bool FD3D11Renderer::IsActiveRenderTargetResource(ID3D11ShaderResourceView* view) const
{
	if (!bRenderTargetActive || !view) { return false; }

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	view->GetResource(resource.GetAddressOf());

	return resource.Get() == activeRenderTargetResource.Get();
}

#pragma endregion

#pragma region UI

bool FD3D11Renderer::ValidateUIBatchResources(const FUIBatchData& batchData) const
{
	for (const FUIRenderBatch& batch : batchData.renderBatches)
	{
		if (batch.texture == nullptr)
		{
			WriteLog("Renderer", ELogLevel::Error, "UI batches require a texture.");
			return false;
		}
		const FD3D11TextureRenderData* textureData = dynamic_cast<const FD3D11TextureRenderData*>(batch.texture->GetRenderData());
		if (textureData == nullptr)
		{
			WriteLog("Renderer", ELogLevel::Error, "UI batches require D3D11 texture render data.");
			return false;
		}
		if (!textureData->textureView)
		{
			WriteLog("Renderer", ELogLevel::Error, "UI batches require a prepared shader resource view.");
			return false;
		}
		Microsoft::WRL::ComPtr<ID3D11Device> textureDevice;
		textureData->textureView->GetDevice(textureDevice.GetAddressOf());
		if (textureDevice.Get() != graphics.GetDevice())
		{
			WriteLog("Renderer", ELogLevel::Error, "UI batch texture belongs to another D3D11 device.");
			return false;
		}
		if (IsActiveRenderTargetResource(textureData->textureView.Get()))
		{
			WriteLog("Renderer", ELogLevel::Error, "UI batches cannot sample the active render target.");
			return false;
		}
		// Renderer와 UI 고정 자원은 함께 초기화된다. 여기서는 목록 전체의 지원 모드를 검사한다.
		switch (batch.sampling)
		{
			case EUISampling::PointClamp:
			case EUISampling::LinearClamp:
				break;
			default:
				WriteLog("Renderer", ELogLevel::Error, "UI batch uses an unsupported sampling mode.");
				return false;
		}
	}
	return true;
}

bool FD3D11Renderer::DrawUI(const FUIDrawElementList& elements)
{
	if (elements.GetElements().empty()) { return true; }
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing UI requires an initialized renderer.");
		return false;
	}
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing UI requires an active frame.");
		return false;
	}

	// 이번 호출만의 출력이다. Build 실패 시 이전 호출의 배치를 제출하지 않는다.
	FUIElementBatcher batcher;
	FUIBatchData batchData;
	if (!batcher.Build(elements, batchData)) { return false; }
	// 빈 문자열/공백만 있는 유효한 요청은 GPU 상태 변경이나 버퍼 준비가 필요 없다.
	if (batchData.renderBatches.empty()) { return true; }
	if (!ValidateUIBatchResources(batchData)) { return false; }
	if (!uiRenderer.UploadBatchData(graphics, batchData)) { return false; }
	if (!uiRenderer.PrepareDraw(graphics, outputWidth, outputHeight)) { return false; }

	bool bSubmitted = true;
	for (const FUIRenderBatch& batch : batchData.renderBatches)
	{
		if (!uiRenderer.SubmitBatch(graphics, batch))
		{
			bSubmitted = false;
			break;
		}
	}

	// 성공/실패 모두 같은 종료 경로를 사용한다. 임의의 호출 전 상태를 저장하는 계약은 아니다.
	graphics.SetPixelShaderConstantBuffer(0, nullptr);
	graphics.SetBlendState(nullptr);
	graphics.SetDepthStencilState(worldDepthStencilState.Get());
	return bSubmitted;
}

#pragma endregion
