#include "ModelLoader.h"

#include "Logging/Log.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace
{
	bool Fail(FStringView message)
	{
		WriteLog("Model", ELogLevel::Error, "{}", message);

		return false;
	}

	bool IsFinite(const aiVector3D& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}

	bool ImportMaterials(const aiScene& scene, const std::filesystem::path& modelDirectory, TArray<FModelMaterialData>& materials)
	{
		materials.reserve(scene.mNumMaterials);
		for (unsigned int i = 0; i < scene.mNumMaterials; ++i)
		{
			if (scene.mMaterials[i] == nullptr) { return Fail("A material is missing."); }
			const aiMaterial& source = *scene.mMaterials[i];
			FModelMaterialData material;
			aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
			if (source.Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS) { (void)source.Get(AI_MATKEY_COLOR_DIFFUSE, color); }
			material.baseColor[0] = color.r;
			material.baseColor[1] = color.g;
			material.baseColor[2] = color.b;
			material.baseColor[3] = color.a;
			for (const float component : material.baseColor)
			{
				if (!std::isfinite(component)) { return Fail("A material contains a non-finite color."); }
			}

			const aiTextureType type = source.GetTextureCount(aiTextureType_BASE_COLOR) != 0 ? aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;
			const unsigned int textureCount = source.GetTextureCount(type);
			if (textureCount > 1) { return Fail("Multiple base color texture layers are not supported."); }
			if (textureCount == 1)
			{
				aiString texture;
				aiTextureMapping mapping = aiTextureMapping_UV;
				unsigned int uvIndex = 0;
				if (source.GetTexture(type, 0, &texture, &mapping, &uvIndex) != AI_SUCCESS || texture.length == 0)
				{
					return Fail("A base color texture has no valid path.");
				}
				if (mapping != aiTextureMapping_UV || uvIndex != 0) { return Fail("Base color textures must use UV channel zero."); }
				if (scene.GetEmbeddedTexture(texture.C_Str()) != nullptr || texture.C_Str()[0] == '*')
				{
					return Fail("Embedded textures are not supported.");
				}
				const std::u8string_view textureName(reinterpret_cast<const char8_t*>(texture.C_Str()), texture.length);
				if (textureName.find(u8'\0') != std::u8string_view::npos) { return Fail("A texture path contains an embedded null character."); }
				const std::filesystem::path texturePath(textureName);
				// 텍스처의 상대 경로는 실행 폴더가 아닌 모델 파일의 폴더를 기준으로 해석한다.
				material.baseColorTexturePath = (texturePath.is_absolute() ? texturePath : modelDirectory / texturePath).lexically_normal();
				aiUVTransform uvTransform;
				if (source.Get(AI_MATKEY_UVTRANSFORM(type, 0), uvTransform) == AI_SUCCESS)
				{
					if (uvTransform.mTranslation.x != 0.0f || uvTransform.mTranslation.y != 0.0f || uvTransform.mScaling.x != 1.0f || uvTransform.mScaling.y != 1.0f || uvTransform.mRotation != 0.0f)
					{
						WriteLog("Model", ELogLevel::Warning, "Material {} has a UV transform that is not imported.", i);
					}
				}
			}
			for (unsigned int textureType = 1; textureType <= static_cast<unsigned int>(AI_TEXTURE_TYPE_MAX); ++textureType)
			{
				const auto extraType = static_cast<aiTextureType>(textureType);
				if (extraType != aiTextureType_BASE_COLOR && extraType != aiTextureType_DIFFUSE && source.GetTextureCount(extraType) != 0)
				{
					WriteLog("Model", ELogLevel::Warning, "Material {} has texture type {} that is not imported.", i, textureType);
				}
			}
			materials.push_back(std::move(material));
		}

		return true;
	}

	bool ImportMeshes(const aiScene& scene, const TArray<FModelMaterialData>& materials, TArray<FModelMeshData>& meshes)
	{
		meshes.reserve(scene.mNumMeshes);
		for (unsigned int i = 0; i < scene.mNumMeshes; ++i)
		{
			if (scene.mMeshes[i] == nullptr) { return Fail("A mesh is missing."); }
			const aiMesh& source = *scene.mMeshes[i];
			if (source.HasBones() || source.mNumAnimMeshes != 0) { return Fail("Bones and morph targets are not supported."); }
			if (!source.HasPositions() || !source.HasFaces() || !source.HasNormals()) { return Fail("Every mesh must have vertices, faces and normals."); }
			FModelMeshData mesh;
			if (scene.mNumMaterials != 0)
			{
				if (source.mMaterialIndex >= scene.mNumMaterials) { return Fail("A mesh material index is out of range."); }
				mesh.materialIndex = static_cast<int32>(source.mMaterialIndex);
			}
			const bool bHasUV = source.HasTextureCoords(0);
			if (bHasUV && source.mNumUVComponents[0] != 2) { return Fail("UV channel zero must have two components."); }
			// 텍스처가 없으면 기본 UV (0, 0)를 허용하지만, 텍스처가 있으면 UV가 필요하다.
			if (!bHasUV && mesh.materialIndex >= 0 && !materials[mesh.materialIndex].baseColorTexturePath.empty())
			{
				return Fail("A textured mesh is missing UV channel zero.");
			}
			if (source.GetNumUVChannels() > 1 || source.GetNumColorChannels() != 0)
			{
				WriteLog("Model", ELogLevel::Warning, "Mesh {} has additional UV channels or vertex colors that are not imported.", i);
			}
			mesh.vertices.reserve(source.mNumVertices);
			for (unsigned int vertexIndex = 0; vertexIndex < source.mNumVertices; ++vertexIndex)
			{
				const aiVector3D& position = source.mVertices[vertexIndex];
				const aiVector3D& normal = source.mNormals[vertexIndex];
				if (!IsFinite(position) || !IsFinite(normal) || (bHasUV && !IsFinite(source.mTextureCoords[0][vertexIndex])))
				{
					return Fail("A mesh contains non-finite vertex data.");
				}
				FModelVertex vertex;
				vertex.position = {position.x, position.y, position.z};
				vertex.normal = {normal.x, normal.y, normal.z};
				if (bHasUV) { vertex.uv = {source.mTextureCoords[0][vertexIndex].x, source.mTextureCoords[0][vertexIndex].y}; }
				mesh.vertices.push_back(vertex);
			}
			if (source.mNumFaces > std::numeric_limits<size_t>::max() / 3) { return Fail("The mesh has too many faces."); }
			mesh.indices.reserve(static_cast<size_t>(source.mNumFaces) * 3);
			for (unsigned int faceIndex = 0; faceIndex < source.mNumFaces; ++faceIndex)
			{
				const aiFace& face = source.mFaces[faceIndex];
				if (face.mNumIndices != 3 || face.mIndices == nullptr) { return Fail("Only triangle faces are supported."); }
				for (unsigned int corner = 0; corner < 3; ++corner)
				{
					if (face.mIndices[corner] >= source.mNumVertices) { return Fail("A vertex index is out of range."); }
					mesh.indices.push_back(face.mIndices[corner]);
				}
			}
			meshes.push_back(std::move(mesh));
		}

		return true;
	}

	bool ImportNodes(const aiNode& root, size_t meshCount, TArray<FModelNodeData>& nodes)
	{
		TArray<std::pair<const aiNode*, int32>> pending{{&root, -1}};
		std::unordered_set<const aiNode*> visited;
		while (!pending.empty())
		{
			const auto [source, parentIndex] = pending.back();
			pending.pop_back();
			if (source == nullptr || !visited.insert(source).second) { return Fail("The node hierarchy is invalid."); }
			if (nodes.size() >= static_cast<size_t>(std::numeric_limits<int32>::max())) { return Fail("The model has too many nodes."); }
			FModelNodeData node;
			node.parentIndex = parentIndex;
			// Assimp의 열 벡터 행렬을 엔진의 행 벡터 행렬로 전치한다.
			for (unsigned int row = 0; row < 4; ++row)
			{
				for (unsigned int column = 0; column < 4; ++column)
				{
					const float value = source->mTransformation[column][row];
					if (!std::isfinite(value)) { return Fail("A node contains a non-finite transform."); }
					node.localTransform.m[row][column] = value;
				}
			}
			node.meshIndices.reserve(source->mNumMeshes);
			for (unsigned int i = 0; i < source->mNumMeshes; ++i)
			{
				if (source->mMeshes == nullptr || source->mMeshes[i] >= meshCount) { return Fail("A node mesh index is out of range."); }
				node.meshIndices.push_back(source->mMeshes[i]);
			}
			const int32 nodeIndex = static_cast<int32>(nodes.size());
			nodes.push_back(std::move(node));
			// 스택에서 꺼낼 때 원본 자식 순서가 유지되도록 역순으로 넣는다.
			for (unsigned int i = source->mNumChildren; i > 0; --i)
			{
				if (source->mChildren == nullptr || source->mChildren[i - 1] == nullptr || source->mChildren[i - 1]->mParent != source)
				{
					return Fail("A child node has an invalid parent.");
				}
				pending.emplace_back(source->mChildren[i - 1], nodeIndex);
			}
		}

		return true;
	}
}

bool FModelLoader::LoadModelFile(const std::filesystem::path& path, FModelData& outModel)
{
	if (path.empty() || path.native().find(std::filesystem::path::value_type{}) != std::filesystem::path::string_type::npos)
	{
		return Fail("The model path must be nonempty and contain no embedded null characters.");
	}

	std::error_code error;
	const std::filesystem::path fullPath = std::filesystem::absolute(path, error).lexically_normal();
	if (error || !std::filesystem::is_regular_file(fullPath, error) || error) { return Fail("The model path is not a readable regular file."); }
	const std::u8string utf8Path = fullPath.u8string();
	Assimp::Importer importer;
	// ConvertToLeftHanded가 UV와 면 방향도 변환하므로 별도로 뒤집지 않는다.
	constexpr unsigned int Flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_ConvertToLeftHanded;
	const aiScene* scene = importer.ReadFile(reinterpret_cast<const char*>(utf8Path.c_str()), Flags);
	if (scene == nullptr) { return Fail(importer.GetErrorString()); }
	if (scene->mRootNode == nullptr || scene->mNumMeshes == 0) { return Fail("The model has no root node or meshes."); }
	if (scene->mNumAnimations != 0 || scene->mNumTextures != 0) { return Fail("Animations and embedded textures are not supported."); }
	if (scene->mNumMaterials > static_cast<unsigned int>(std::numeric_limits<int32>::max())) { return Fail("The model has too many materials."); }
	if (scene->mNumCameras != 0 || scene->mNumLights != 0)
	{
		WriteLog("Model", ELogLevel::Warning, "Cameras and lights are not imported.");
	}

	FModelData model;
	if (!ImportMaterials(*scene, fullPath.parent_path(), model.materials)) { return false; }
	if (!ImportMeshes(*scene, model.materials, model.meshes)) { return false; }
	if (!ImportNodes(*scene->mRootNode, model.meshes.size(), model.nodes)) { return false; }

	// 완성된 배열만 넘겨 실패 중간 결과나 Assimp의 소유 포인터를 남기지 않는다.
	outModel = std::move(model);

	return true;
}
