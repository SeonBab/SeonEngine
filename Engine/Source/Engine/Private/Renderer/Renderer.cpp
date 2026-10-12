#include "Renderer/Renderer.h"

#include "Containers/Array.h"
#include "Containers/SeonString.h"
#include "CoreTypes.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "Math/Matrix4.h"
#include "Platform/Windows/WindowsString.h"
#include "Renderer/MeshRenderData.h"
#include "SceneView.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <utility>

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogRenderer);

	// 유효한 내부 UTF-8 파일 이름을 받는다. 경로 생성만 하며 존재 여부는 읽기 함수에서 확인한다.
	[[nodiscard]] std::optional<std::filesystem::path> GetShaderBytecodePath(const FString& shaderFileName)
	{
		const auto baseDir = FPlatformProcess::BaseDir();
		if (!baseDir)
		{
			return std::nullopt;
		}

		const FString fullPath = *baseDir + "Shaders/" + shaderFileName;
		const auto widePath = FWindowsString::UTF8ToWide(fullPath);
		if (!widePath)
		{
			return std::nullopt;
		}

		return std::filesystem::path(*widePath);
	}

	// 주어진 파일을 바이너리로 전부 읽고 성공 시에만 출력을 교체한다.
	[[nodiscard]] bool ReadShaderBytecode(const std::filesystem::path& path, TArray<uint8>& outBytecode)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			SE_LOG(LogRenderer, Error, "Opening shader bytecode file failed");

			return false;
		}
		const std::streampos endPosition = file.tellg();
		if (endPosition <= std::streampos(0))
		{
			SE_LOG(LogRenderer, Error, "Shader bytecode file is empty or its size could not be read");

			return false;
		}
		const auto byteCount = static_cast<std::uintmax_t>(static_cast<std::streamoff>(endPosition));
		TArray<uint8> newBytecode;
		if (byteCount > newBytecode.max_size() || byteCount > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
		{
			SE_LOG(LogRenderer, Error, "Shader bytecode file exceeds the supported read size");

			return false;
		}
		newBytecode.resize(static_cast<size_t>(byteCount));
		file.seekg(0, std::ios::beg);
		if (!file.read(reinterpret_cast<char*>(newBytecode.data()), static_cast<std::streamsize>(byteCount)))
		{
			SE_LOG(LogRenderer, Error, "Reading the complete shader bytecode file failed");

			return false;
		}
		outBytecode = std::move(newBytecode);

		return true;
	}
}

bool FRenderer::Init(void* WindowHandle, uint32 SizeX, uint32 SizeY)
{
	Device.InitD3DDevice();

	if (!Viewport.Init(WindowHandle, SizeX, SizeY))
	{
		Device.Shutdown();
		return false;
	}

	const auto vertexShaderPath = GetShaderBytecodePath("PositionColorVertexShader.cso");
	if (!vertexShaderPath)
	{
		Shutdown();
		return false;
	}

	TArray<uint8> vertexShaderBytecode;
	if (!ReadShaderBytecode(*vertexShaderPath, vertexShaderBytecode))
	{
		Shutdown();
		return false;
	}

	if (!Device.CreateVertexShader(vertexShaderBytecode, vertexShader))
	{
		Shutdown();
		return false;
	}

	if (!Device.CreateInputLayout(vertexShaderBytecode, inputLayout))
	{
		Shutdown();
		return false;
	}

	const auto pixelShaderPath = GetShaderBytecodePath("VertexColorPixelShader.cso");
	if (!pixelShaderPath)
	{
		Shutdown();
		return false;
	}

	TArray<uint8> pixelShaderBytecode;
	if (!ReadShaderBytecode(*pixelShaderPath, pixelShaderBytecode))
	{
		Shutdown();
		return false;
	}

	if (!Device.CreatePixelShader(pixelShaderBytecode, pixelShader))
	{
		Shutdown();
		return false;
	}

	if (!Device.CreateConstantBuffer(sizeof(FMatrix4), transformConstantBuffer))
	{
		Shutdown();
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC description{};
	description.DepthEnable = TRUE;
	description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	description.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	description.StencilEnable = FALSE;
	description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	description.BackFace = description.FrontFace;
	if (!Device.CreateDepthStencilState(description, depthStencilState))
	{
		Shutdown();
		return false;
	}

	D3D11_RASTERIZER_DESC rasterizerDescription{};
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_BACK;
	rasterizerDescription.FrontCounterClockwise = TRUE;
	rasterizerDescription.DepthClipEnable = TRUE;
	if (!Device.CreateRasterizerState(rasterizerDescription, rasterizerState))
	{
		Shutdown();
		return false;
	}

	return true;
}

void FRenderer::Shutdown()
{
	// 출력 참조를 해제하기 전에 컨텍스트가 보유한 바인딩을 끊는다.
	ID3D11DeviceContext* DeviceContext = Device.GetDeviceContext();
	if (DeviceContext)
	{
		DeviceContext->ClearState();
	}

	rasterizerState.Reset();
	depthStencilState.Reset();
	transformConstantBuffer.Reset();
	inputLayout.Reset();
	pixelShader.Reset();
	vertexShader.Reset();

	Viewport.Shutdown();
	Device.Shutdown();
}

void FRenderer::Resize(uint32 SizeX, uint32 SizeY)
{
	Viewport.Resize(SizeX, SizeY);
}

void FRenderer::RenderFrame(const TArray<FMeshRenderData>& renderData, const FSceneView* sceneView)
{
	Device.SetInputLayout(inputLayout.Get());
	Device.SetVertexShader(vertexShader.Get());
	Device.SetPixelShader(pixelShader.Get());
	Device.SetRenderTargetView(Viewport.GetBackBufferRenderTargetView(), Viewport.GetDepthStencilView());
	Device.SetDepthStencilState(depthStencilState.Get(), 0);
	Device.SetRasterizerState(rasterizerState.Get());

	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(Viewport.GetSizeX());
	viewport.Height = static_cast<float>(Viewport.GetSizeY());
	viewport.MaxDepth = 1.0f;
	Device.SetViewport(viewport);

	constexpr float ClearColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};
	Device.ClearRenderTargetView(Viewport.GetBackBufferRenderTargetView(), ClearColor);
	Device.ClearDepthStencilView(Viewport.GetDepthStencilView(), 0.0f);

	if (sceneView)
	{
		const FMatrix4 worldToClip = sceneView->viewMatrices.GetWorldToView() * sceneView->viewMatrices.GetViewToClip();

		for (const FMeshRenderData& mesh : renderData)
		{
			if (mesh.vertices.empty())
			{
				continue;
			}

			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			if (!Device.CreateVertexBuffer(mesh.vertices, vertexBuffer))
			{
				continue;
			}
			const FMatrix4 localToClip = mesh.localToWorld * worldToClip;
			if (!Device.UpdateConstantBuffer(transformConstantBuffer.Get(), &localToClip, sizeof(FMatrix4)))
			{
				continue;
			}
			Device.SetVertexShaderConstantBuffer(transformConstantBuffer.Get());
			Device.SetVertexBuffer(vertexBuffer.Get());
			D3D11_PRIMITIVE_TOPOLOGY topology;
			switch (mesh.primitiveType)
			{
				case EPrimitiveType::PT_TriangleList:
					topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					break;
				case EPrimitiveType::PT_TriangleStrip:
					topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
					break;
				default:
					SE_LOG(LogRenderer, Error, "Unsupported primitive type");
					continue;
			}
			Device.SetPrimitiveTopology(topology);
			Device.Draw(static_cast<uint32>(mesh.vertices.size()), 0);
		}
	}

	if (!Viewport.Present())
	{
		SE_LOG(LogRenderer, Fatal, "Presenting the viewport failed");
	}
}
