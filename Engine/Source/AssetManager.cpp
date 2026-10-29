#include "AssetManager.h"

#include "Core/Logging/Log.h"
#include "Core/Misc/FileHelper.h"
#include "Core/Misc/Paths.h"
#include "Image/ImageUtils.h"
#include "Renderer/Font.h"
#include "Renderer/Material.h"
#include "Renderer/ModelData.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Renderer.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/StaticMeshRenderData.h"
#include "Renderer/Texture2D.h"

#include <utility>

namespace
{
	bool ValidateSingleMeshModel(const FModelData& model)
	{
		if (model.meshes.size() != 1 || model.materials.size() != 1 || model.meshes.front().materialIndex != 0)
		{
			WriteLog("AssetManager", ELogLevel::Error, "Static mesh loading requires exactly one mesh and one referenced material.");
			return false;
		}

		FMatrix4 identity;
		identity.SetIdentity();
		TArray<FMatrix4> nodeToModel;
		nodeToModel.reserve(model.nodes.size());
		size_t placementCount = 0;
		for (const FModelNodeData& node : model.nodes)
		{
			if (node.parentIndex < -1 || (node.parentIndex >= 0 && static_cast<size_t>(node.parentIndex) >= nodeToModel.size()))
			{
				WriteLog("AssetManager", ELogLevel::Error, "Model nodes require valid parents before their children.");
				return false;
			}

			FMatrix4 transform = node.localTransform;
			if (node.parentIndex >= 0)
			{
				transform = transform * nodeToModel[static_cast<size_t>(node.parentIndex)];
			}
			nodeToModel.push_back(transform);
			for (const uint32 meshIndex : node.meshIndices)
			{
				if (meshIndex != 0 || ++placementCount != 1)
				{
					WriteLog("AssetManager", ELogLevel::Error, "Static mesh loading requires exactly one node mesh placement.");
					return false;
				}

				// 작은 변환도 임의로 버리지 않는다. 변환 굽기를 지원하기 전에는 정확한 항등만 허용한다.
				for (size_t row = 0; row < 4; ++row)
				{
					for (size_t column = 0; column < 4; ++column)
					{
						if (transform.m[row][column] != identity.m[row][column])
						{
							WriteLog("AssetManager", ELogLevel::Error, "Static mesh loading requires an identity accumulated node transform.");
							return false;
						}
					}
				}
			}
		}
		if (placementCount != 1)
		{
			WriteLog("AssetManager", ELogLevel::Error, "Static mesh loading requires exactly one node mesh placement.");
			return false;
		}

		return true;
	}
}

#pragma region Lifecycle

UAssetManager::UAssetManager(IRenderer& inRenderer)
	: renderer(inRenderer)
{
}

#pragma endregion

#pragma region Textures

TSharedPtr<UTexture2D> UAssetManager::LoadTexture2D(FStringView contentPath)
{
	const FString key(contentPath);
	const auto existing = textures.find(key);
	if (existing != textures.end()) { return existing->second; }

	const std::optional<std::filesystem::path> path = FPaths::ResolveContentPath(contentPath);
	if (!path) { return nullptr; }

	TSharedPtr<UTexture2D> texture = MakeShared<UTexture2D>();
	const std::filesystem::path::string_type extension = path->extension().native();
	if (extension.size() == 4 && extension[0] == L'.' && (extension[1] == L'd' || extension[1] == L'D') && (extension[2] == L'd' || extension[2] == L'D') && (extension[3] == L's' || extension[3] == L'S'))
	{
		TArray<uint8> bytes;
		if (!FFileHelper::ReadBinaryFile(*path, bytes) || !renderer.CreateTexture2DFromDDS(bytes, *texture)) { return nullptr; }
	}
	else
	{
		FImage image;
		if (!FImageUtils::LoadImageFile(*path, image)) { return nullptr; }
		if (!renderer.CreateTexture2D(image, *texture))
		{
			WriteLog("AssetManager", ELogLevel::Error, "Creating a texture failed: {}", contentPath);
			return nullptr;
		}
	}

	textures.emplace(key, texture);

	return texture;
}

#pragma endregion

#pragma region StaticMeshes

TSharedPtr<UStaticMesh> UAssetManager::LoadStaticMesh(FStringView contentPath)
{
	const FString key(contentPath);
	const auto existing = staticMeshes.find(key);
	if (existing != staticMeshes.end()) { return existing->second; }

	const std::optional<std::filesystem::path> path = FPaths::ResolveContentPath(contentPath);
	if (!path) { return nullptr; }

	FModelData model;
	if (!FModelLoader::LoadTextXModel(*path, model)) { return nullptr; }
	if (!ValidateSingleMeshModel(model))
	{
		WriteLog("AssetManager", ELogLevel::Error, "Unsupported static mesh model: {}", contentPath);
		return nullptr;
	}

	TSharedPtr<UMaterial> material = MakeShared<UMaterial>();
	const std::filesystem::path& texturePath = model.materials.front().baseColorTexturePath;
	if (!texturePath.empty())
	{
		const std::optional<FString> textureContentPath = FPaths::MakeContentPath(texturePath);
		if (!textureContentPath) { return nullptr; }

		TSharedPtr<UTexture2D> texture = LoadTexture2D(*textureContentPath);
		if (!texture) { return nullptr; }
		material->SetBaseColorTexture(std::move(texture));
	}

	TUniquePtr<FStaticMeshRenderData> renderData;
	if (!renderer.CreateMesh(model.meshes.front(), renderData) || !renderData)
	{
		WriteLog("AssetManager", ELogLevel::Error, "Creating a static mesh failed: {}", contentPath);
		return nullptr;
	}

	TSharedPtr<UStaticMesh> mesh = MakeShared<UStaticMesh>();
	mesh->SetRenderData(std::move(renderData));
	mesh->SetStaticMaterials({material});
	staticMeshes.emplace(key, mesh);

	return mesh;
}

#pragma endregion

#pragma region Fonts

TSharedPtr<UFont> UAssetManager::LoadFont(FStringView contentPath)
{
	const FString key(contentPath);
	const auto existing = fonts.find(key);
	if (existing != fonts.end()) { return existing->second; }

	const std::optional<std::filesystem::path> path = FPaths::ResolveContentPath(contentPath);
	if (!path) { return nullptr; }

	TArray<uint8> fontData;
	if (!FFileHelper::ReadBinaryFile(*path, fontData)) { return nullptr; }

	TSharedPtr<UFont> font = MakeShared<UFont>();
	if (!renderer.CreateFontResource(fontData, *font))
	{
		WriteLog("AssetManager", ELogLevel::Error, "Creating a font failed: {}", contentPath);
		return nullptr;
	}

	fonts.emplace(key, font);

	return font;
}

#pragma endregion
