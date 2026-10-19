#include "Renderer.h"

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Log.h"
#include "Core/Math/Matrix4.h"
#include "Core/Misc/Paths.h"
#include "D3D11/D3D11Shader.h"
#include "Image/Image.h"
#include "Renderer/StaticMeshData.h"
#include "Renderer/Texture2D.h"
#include "UI/UIDrawElementList.h"
#include "UI/UIImageDrawElement.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <utility>

namespace
{
	bool ValidateTriangleMesh(size_t vertexCount, const TArray<uint32>& indices)
	{
		// TODO(seon): 인덱스 없는 삼각형 목록 지원 시 정점 개수 검증과 GPU 생성·Draw 경로를 함께 추가한다.
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

bool FRenderer::Initialize(HWND windowHandle, int width, int height)
{
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
	TArray<uint8> vertexShaderBytecode;
	TArray<uint8> pixelShaderBytecode;
	if (!ReadShaderBytecode(shaderDir / L"TriangleVertexShader.cso", vertexShaderBytecode) ||
		!ReadShaderBytecode(shaderDir / L"TrianglePixelShader.cso", pixelShaderBytecode))
	{
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Vertex shader bytecode loaded ({} bytes).", vertexShaderBytecode.size());
	WriteLog("D3D11", ELogLevel::Info, "Pixel shader bytecode loaded ({} bytes).", pixelShaderBytecode.size());
	if (!graphics.Initialize())
	{
		return false;
	}
	if (!graphics.CreateVertexShader(vertexShaderBytecode, vertexShader) ||
		!graphics.CreatePixelShader(pixelShaderBytecode, pixelShader))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Vertex and pixel shaders created.");
	if (!graphics.CreateInputLayout(vertexShaderBytecode, inputLayout))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Vertex input layout created.");

	static_assert(sizeof(FMatrix4) == 64);
	constexpr uint32 TransformBufferByteSize = static_cast<uint32>(sizeof(FMatrix4));
	FMatrix4 initialLocalToWorld;
	initialLocalToWorld.SetIdentity();
	FMatrix4 initialWorldToClip;
	initialWorldToClip.SetIdentity();
	const FMatrix4 localToClip = initialLocalToWorld * initialWorldToClip;
	if (!graphics.CreateConstantBuffer(TransformBufferByteSize, transformConstantBuffer) ||
		!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &localToClip, TransformBufferByteSize))
	{
		Shutdown();
		return false;
	}

	WriteLog("D3D11", ELogLevel::Info, "Transform constant buffer initialized.");

	FMatrix4 initialImageTransform;
	initialImageTransform.SetIdentity();
	if (!graphics.CreateConstantBuffer(TransformBufferByteSize, imageTransformConstantBuffer) ||
		!graphics.UpdateConstantBuffer(imageTransformConstantBuffer.Get(), &initialImageTransform, TransformBufferByteSize))
	{
		Shutdown();
		return false;
	}

	D3D11_BLEND_DESC blendDescription{};
	D3D11_RENDER_TARGET_BLEND_DESC& target = blendDescription.RenderTarget[0];
	target.BlendEnable = TRUE;
	target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	target.BlendOp = D3D11_BLEND_OP_ADD;
	target.SrcBlendAlpha = D3D11_BLEND_ONE;
	target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (!graphics.CreateBlendState(blendDescription, uiBlendState))
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

	bufferWidth = width;
	bufferHeight = height;
	localToWorld = initialLocalToWorld;
	worldToClip = initialWorldToClip;
	bInitialized = true;

	const FStaticMeshData imageQuadData =
		{
			{{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {}, {0.0f, 0.0f}},
				{{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {}, {1.0f, 0.0f}},
				{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {}, {1.0f, 1.0f}},
				{{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {}, {0.0f, 1.0f}}},
			{0, 1, 2, 0, 2, 3}};
	if (!CreateMesh(imageQuadData, imageQuad))
	{
		Shutdown();
		return false;
	}

	return true;
}

bool FRenderer::CreateMesh(const FStaticMeshData& meshData, FStaticMeshRenderData& outMesh, const FImage* baseColorImage)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Creating a mesh requires an initialized renderer.");
		return false;
	}
	if (!ValidateTriangleMesh(meshData.vertices.size(), meshData.indices)) { return false; }

	FStaticMeshRenderData newMesh;
	if (!graphics.CreateVertexBuffer(meshData.vertices, newMesh.vertexBuffer) ||
		!graphics.CreateIndexBuffer(meshData.indices, newMesh.indexBuffer))
	{
		return false;
	}

	if (baseColorImage && !CreateTexture2D(*baseColorImage, newMesh.baseColorTexture)) { return false; }

	// 버퍼와 지정한 이미지가 모두 준비된 뒤에만 기존 메시를 교체한다.
	newMesh.indexCount = static_cast<uint32>(meshData.indices.size());
	newMesh.addressU = meshData.addressU;
	newMesh.addressV = meshData.addressV;
	outMesh = std::move(newMesh);

	return true;
}

bool FRenderer::CreateTexture2D(const FImage& image, UTexture2D& outTexture)
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
	UTexture2D newTexture;
	if (!graphics.CreateTexture2D(static_cast<uint32>(image.sizeX), static_cast<uint32>(image.sizeY), image.rawData, texture)) { return false; }
	if (!graphics.CreateShaderResourceView(texture.Get(), newTexture.textureView)) { return false; }

	outTexture = std::move(newTexture);

	return true;
}

bool FRenderer::SetLocalToWorld(const FMatrix4& matrix)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting LocalToWorld requires an initialized renderer.");
		return false;
	}

	const FMatrix4 localToClip = matrix * worldToClip;
	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &localToClip, static_cast<uint32>(sizeof(localToClip))))
	{
		return false;
	}
	localToWorld = matrix;

	return true;
}

bool FRenderer::SetWorldToClip(const FMatrix4& matrix)
{
	if (!bInitialized)
	{
		WriteLog("Renderer", ELogLevel::Error, "Setting WorldToClip requires an initialized renderer.");
		return false;
	}

	const FMatrix4 localToClip = localToWorld * matrix;
	if (!graphics.UpdateConstantBuffer(transformConstantBuffer.Get(), &localToClip, static_cast<uint32>(sizeof(localToClip))))
	{
		return false;
	}
	worldToClip = matrix;

	return true;
}

bool FRenderer::BeginFrame(int width, int height)
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
		bufferWidth = width;
		bufferHeight = height;
	}

	const float clearColor[] = {0.05f, 0.10f, 0.20f, 1.0f};
	graphics.SetRenderTarget(swapChain.GetBackBufferRenderTargetView());
	graphics.SetViewport(bufferWidth, bufferHeight);
	graphics.ClearRenderTargetView(swapChain.GetBackBufferRenderTargetView(), clearColor);
	bFrameActive = true;

	return true;
}

bool FRenderer::DrawMesh(const FStaticMeshRenderData& mesh)
{
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing a mesh requires an active frame.");
		return false;
	}

	if (!mesh.vertexBuffer || !mesh.indexBuffer || mesh.indexCount == 0)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires a nonempty GPU mesh.");
		return false;
	}
	Microsoft::WRL::ComPtr<ID3D11Device> vertexDevice;
	Microsoft::WRL::ComPtr<ID3D11Device> indexDevice;
	mesh.vertexBuffer->GetDevice(vertexDevice.GetAddressOf());
	mesh.indexBuffer->GetDevice(indexDevice.GetAddressOf());
	if (vertexDevice.Get() != graphics.GetDevice() || indexDevice.Get() != graphics.GetDevice())
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing requires mesh buffers from the current renderer device.");
		return false;
	}

	graphics.SetVertexShader(vertexShader.Get());
	graphics.SetPixelShader(pixelShader.Get());
	graphics.SetInputLayout(inputLayout.Get());
	graphics.SetVertexBuffer(mesh.vertexBuffer.Get());
	const auto& textureView = mesh.baseColorTexture.textureView;
	graphics.SetPixelShaderResourceView(textureView ? textureView.Get() : whiteTextureView.Get());
	const size_t addressU = mesh.addressU == ETextureAddressMode::Wrap ? 1 : 0;
	const size_t addressV = mesh.addressV == ETextureAddressMode::Wrap ? 1 : 0;
	graphics.SetPixelShaderSamplerState(samplerStates[addressU][addressV].Get());
	graphics.SetVertexShaderConstantBuffer(transformConstantBuffer.Get());
	graphics.SetIndexBuffer(mesh.indexBuffer.Get());
	graphics.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics.DrawIndexed(mesh.indexCount, 0, 0);

	return true;
}

bool FRenderer::DrawUI(const FUIDrawElementList& elements)
{
	if (elements.GetImageElements().empty()) { return true; }
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing UI requires an active frame.");
		return false;
	}

	graphics.SetBlendState(uiBlendState.Get());
	for (const FUIImageDrawElement& element : elements.GetImageElements())
	{
		if (!DrawImage(element))
		{
			graphics.SetBlendState(nullptr);
			return false;
		}
	}
	graphics.SetBlendState(nullptr);

	return true;
}

bool FRenderer::DrawImage(const FUIImageDrawElement& element)
{
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing an image requires an active frame.");
		return false;
	}
	if (!element.texture || !element.texture->textureView || !std::isfinite(element.position.x) || !std::isfinite(element.position.y) ||
		!std::isfinite(element.size.x) || !std::isfinite(element.size.y) || element.size.x <= 0.0f || element.size.y <= 0.0f)
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing an image requires a texture, finite position and positive finite size.");
		return false;
	}
	Microsoft::WRL::ComPtr<ID3D11Device> textureDevice;
	element.texture->textureView->GetDevice(textureDevice.GetAddressOf());
	if (textureDevice.Get() != graphics.GetDevice())
	{
		WriteLog("Renderer", ELogLevel::Error, "Drawing an image requires a texture from the current renderer device.");
		return false;
	}

	const float width = static_cast<float>(bufferWidth);
	const float height = static_cast<float>(bufferHeight);
	FMatrix4 imageToClip{};
	imageToClip.m[0][0] = 2.0f * (element.size.x / width);
	imageToClip.m[1][1] = -2.0f * (element.size.y / height);
	imageToClip.m[2][2] = 1.0f;
	imageToClip.m[3][0] = 2.0f * (element.position.x / width) - 1.0f;
	imageToClip.m[3][1] = 1.0f - 2.0f * (element.position.y / height);
	imageToClip.m[3][3] = 1.0f;
	if (!std::isfinite(imageToClip.m[0][0]) || !std::isfinite(imageToClip.m[1][1]) ||
		!std::isfinite(imageToClip.m[3][0]) || !std::isfinite(imageToClip.m[3][1]))
	{
		WriteLog("Renderer", ELogLevel::Error, "Image transform exceeds the finite matrix range.");
		return false;
	}
	if (!graphics.UpdateConstantBuffer(imageTransformConstantBuffer.Get(), &imageToClip, static_cast<uint32>(sizeof(imageToClip)))) { return false; }

	graphics.SetVertexShader(vertexShader.Get());
	graphics.SetPixelShader(pixelShader.Get());
	graphics.SetInputLayout(inputLayout.Get());
	graphics.SetVertexBuffer(imageQuad.vertexBuffer.Get());
	graphics.SetPixelShaderResourceView(element.texture->textureView.Get());
	graphics.SetPixelShaderSamplerState(samplerStates[0][0].Get());
	graphics.SetVertexShaderConstantBuffer(imageTransformConstantBuffer.Get());
	graphics.SetIndexBuffer(imageQuad.indexBuffer.Get());
	graphics.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics.DrawIndexed(imageQuad.indexCount, 0, 0);

	return true;
}

bool FRenderer::EndFrame()
{
	if (!bFrameActive)
	{
		WriteLog("Renderer", ELogLevel::Error, "Ending a frame requires an active frame.");
		return false;
	}
	bFrameActive = false;

	return swapChain.Present();
}

void FRenderer::Shutdown()
{
	if (graphics.GetContext())
	{
		graphics.SetBlendState(nullptr);
		graphics.SetPixelShaderResourceView(nullptr);
		graphics.SetPixelShaderSamplerState(nullptr);
		graphics.SetIndexBuffer(nullptr);
		graphics.SetVertexShaderConstantBuffer(nullptr);
		graphics.SetVertexBuffer(nullptr);
		graphics.SetInputLayout(nullptr);
		graphics.SetRenderTarget(nullptr);
		graphics.SetPixelShader(nullptr);
		graphics.SetVertexShader(nullptr);
	}
	swapChain.Shutdown();
	imageQuad = FStaticMeshRenderData{};
	for (auto& samplersForU : samplerStates)
	{
		for (auto& sampler : samplersForU)
		{
			sampler.Reset();
		}
	}
	whiteTextureView.Reset();
	uiBlendState.Reset();
	transformConstantBuffer.Reset();
	imageTransformConstantBuffer.Reset();
	inputLayout.Reset();
	pixelShader.Reset();
	vertexShader.Reset();
	graphics.Shutdown();
	localToWorld = FMatrix4{};
	worldToClip = FMatrix4{};
	bufferWidth = 0;
	bufferHeight = 0;
	bInitialized = false;
	bFrameActive = false;
}
