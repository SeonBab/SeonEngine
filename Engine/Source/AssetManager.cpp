#include "AssetManager.h"

#include "Core/Log.h"
#include "GLTFLoader.h"
#include "Image.h"
#include "ImageLoader.h"
#include "Mesh/StaticMesh.h"
#include "Paths.h"
#include "Renderer/Material.h"
#include "Texture/Texture.h"

#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager()
{
	assert(instance == nullptr && "AssetManager는 한 번에 하나만 존재해야 한다.");
	instance = this;
}

AssetManager::~AssetManager()
{
	if (instance == this) { instance = nullptr; }
}

AssetManager& AssetManager::Get()
{
	assert(instance != nullptr && "AssetManager가 없을 때 Get을 호출했다.");
	return *instance;
}

std::shared_ptr<const Model> AssetManager::LoadModel(std::string_view relativePath)
{
	std::filesystem::path absolutePath;
	if (!Paths::GetAbsolutePath(relativePath, absolutePath))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to resolve model path: " + std::string(relativePath));
		return nullptr;
	}

	absolutePath = absolutePath.lexically_normal();
	absolutePath.make_preferred();

	const auto cachedModel = models.find(absolutePath);
	if (cachedModel != models.end()) { return cachedModel->second; }

	GLTFModel gltfModel;
	if (!GLTFLoader::LoadModel(absolutePath, gltfModel))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to load model: " + std::string(relativePath));
		return nullptr;
	}

	// UTF-8 경로를 정규화하고 구분자를 /로 통일해 메시 경로의 앞부분으로 쓴다.
	const std::filesystem::path assetPath = std::filesystem::path(std::u8string(relativePath.begin(), relativePath.end())).lexically_normal();
	const std::u8string assetPathUtf8 = assetPath.generic_u8string();
	const std::string modelPath(assetPathUtf8.begin(), assetPathUtf8.end());

	Model model;
	model.meshes.reserve(gltfModel.meshes.size());
	for (std::size_t i = 0; i < gltfModel.meshes.size(); ++i)
	{
		StaticMesh& source = gltfModel.meshes[i];

		// 경로는 만든 뒤 바꿀 수 없으므로, 이름과 "파일경로#번호" 경로로 메시를 새로 만든다.
		StaticMesh meshData(source.GetName(), modelPath + "#" + std::to_string(i));

		// 정점과 인덱스는 복사하지 않고 옮겨 담는다.
		meshData.vertices = std::move(source.vertices);
		meshData.indices = std::move(source.indices);

		model.meshes.push_back(std::make_shared<const StaticMesh>(std::move(meshData)));
	}

	// glTF 재질마다 Material을 만든다. 텍스처 경로의 기준은 모델 파일이 있는 폴더다.
	const std::filesystem::path modelDirectory = assetPath.parent_path();
	std::vector<std::shared_ptr<const Material>> materials;
	materials.reserve(gltfModel.materials.size());
	for (const GLTFModelMaterial& source : gltfModel.materials)
	{
		std::shared_ptr<const Material> material = CreateMaterial(source, modelDirectory);
		if (material == nullptr)
		{
			Log::Write(LogLevel::Error, "AssetManager", "Failed to create material for model: " + std::string(relativePath));
			return nullptr;
		}

		materials.push_back(std::move(material));
	}

	// 메시마다 재질을 정한다. 재질 번호는 로더가 범위를 확인했다.
	// 재질이 없는 메시는 glTF 기본 재질을 쓰며, Material의 기본값(흰색, 금속성 1, 거칠기 1, 텍스처 없음)이 이와 같다.
	// 기본 재질은 필요할 때 한 번만 만들어 공유한다.
	std::shared_ptr<const Material> defaultMaterial;
	model.meshMaterials.reserve(gltfModel.meshMaterials.size());
	for (const std::optional<std::size_t>& materialIndex : gltfModel.meshMaterials)
	{
		if (materialIndex.has_value())
		{
			model.meshMaterials.push_back(materials[materialIndex.value()]);
			continue;
		}

		if (defaultMaterial == nullptr) { defaultMaterial = std::make_shared<const Material>(); }
		model.meshMaterials.push_back(defaultMaterial);
	}

	model.sceneNodes = std::move(gltfModel.sceneNodes);

	std::shared_ptr<const Model> result = std::make_shared<const Model>(std::move(model));
	models.emplace(std::move(absolutePath), result);
	return result;
}

std::shared_ptr<const Material> AssetManager::CreateMaterial(const GLTFModelMaterial& source, const std::filesystem::path& modelDirectory)
{
	// 계수는 로더가 범위를 확인했고 의미가 같으므로 그대로 옮긴다.
	Material material;
	material.baseColor = source.baseColor;
	material.roughness = source.roughness;
	material.metallic = source.metallic;

	if (!source.baseColorImageUri.empty())
	{
		// uri는 UTF-8이며 모델 파일 폴더 기준이다. 폴더와 합쳐 저장소 루트 기준 경로로 만든다.
		const std::u8string uriUtf8(source.baseColorImageUri.begin(), source.baseColorImageUri.end());
		const std::filesystem::path imagePath = (modelDirectory / std::filesystem::path(uriUtf8)).lexically_normal();
		const std::u8string imagePathUtf8 = imagePath.generic_u8string();
		const std::string relativePath(imagePathUtf8.begin(), imagePathUtf8.end());

		// glTF 색 텍스처의 RGB는 sRGB 값이다. 같은 이미지는 LoadTexture가 캐시해 공유한다.
		material.baseColorTexture = LoadTexture(relativePath, TextureFormat::RGBA8SRGB);
		if (material.baseColorTexture == nullptr) { return nullptr; }
	}

	return std::make_shared<const Material>(std::move(material));
}

std::shared_ptr<const Texture> AssetManager::LoadTexture(std::string_view relativePath, TextureFormat format)
{
	const bool isSupportedFormat = format == TextureFormat::RGBA8UNorm || format == TextureFormat::RGBA8SRGB;
	if (!isSupportedFormat)
	{
		Log::Write(LogLevel::Error, "AssetManager", "Unsupported texture format.");
		return nullptr;
	}

	std::filesystem::path absolutePath;
	if (!Paths::GetAbsolutePath(relativePath, absolutePath))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to resolve texture path: " + std::string(relativePath));
		return nullptr;
	}

	absolutePath = absolutePath.lexically_normal();
	absolutePath.make_preferred();

	const std::pair<std::filesystem::path, TextureFormat> cacheKey(absolutePath, format);
	const auto cachedTexture = textures.find(cacheKey);
	if (cachedTexture != textures.end()) { return cachedTexture->second; }

	Image image;
	if (!ImageLoader::LoadPNG(absolutePath, image)) { return nullptr; }

	const std::u8string relativePathUtf8(relativePath.begin(), relativePath.end());
	const std::filesystem::path requestedPath(relativePathUtf8);
	const std::filesystem::path assetPath = requestedPath.lexically_normal();
	const std::u8string assetNameUtf8 = assetPath.stem().u8string();
	const std::u8string assetPathUtf8 = assetPath.generic_u8string();

	const std::string assetName(assetNameUtf8.begin(), assetNameUtf8.end());
	const std::string assetRelativePath(assetPathUtf8.begin(), assetPathUtf8.end());

	Texture textureData(assetName, assetRelativePath);
	if (!textureData.Initialize(std::move(image), format))
	{
		Log::Write(LogLevel::Error, "AssetManager", "Failed to initialize texture: " + std::string(relativePath));
		return nullptr;
	}

	std::shared_ptr<const Texture> texture;
	texture = std::make_shared<const Texture>(std::move(textureData));

	textures.emplace(cacheKey, texture);
	return texture;
}
