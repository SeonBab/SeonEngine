#include "GLTFLoader.h"

#include "Core/FileHelper.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector3.h"
#include "GLTFDocument.h"
#include "Mesh/MeshVertex.h"
#include "Mesh/StaticMesh.h"

#include "rapidjson/document.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{
	// glTF는 미터, 엔진은 센티미터 단위다.
	constexpr float MetersToCentimeters = 100.0f;

	/**
	 * 엔진 로컬 좌표의 삼각형 위치와 UV에서 정규화하지 않은 U 방향을 구한다.
	 * normal은 사용하지 않는다. UV가 누락되었는지는 호출자가 따로 판단해야 한다.
	 * 유효하지 않은 값, 퇴화한 삼각형 또는 표현할 수 없는 결과는 false로 처리하며 outTangent를 유지한다.
	 * 모든 검사를 통과하면 outTangent를 채우고 true를 반환한다. 아직 로더에서 호출하지 않는다.
	 */
	bool CalculateTriangleTangent(const MeshVertex& v0, const MeshVertex& v1, const MeshVertex& v2, Vector3& outTangent)
	{
		// v0에서 나머지 두 정점으로 향하는 공간상의 변이다.
		const Vector3 edge1 = v1.position - v0.position;
		const Vector3 edge2 = v2.position - v0.position;
		const Vector3 triangleAreaVector = edge1.Cross(edge2);
		if (!std::isfinite(triangleAreaVector.x) || !std::isfinite(triangleAreaVector.y) || !std::isfinite(triangleAreaVector.z)) { return false; }
		if (triangleAreaVector.x == 0.0f && triangleAreaVector.y == 0.0f && triangleAreaVector.z == 0.0f) { return false; }

		const float edge1LengthSquared = edge1.Dot(edge1);
		const float edge2LengthSquared = edge2.Dot(edge2);
		const float triangleAreaVectorLengthSquared = triangleAreaVector.Dot(triangleAreaVector);
		if (!std::isfinite(edge1LengthSquared) || !std::isfinite(edge2LengthSquared) || !std::isfinite(triangleAreaVectorLengthSquared)) { return false; }

		constexpr float TriangleParallelTolerance = 1e-6f;
		const double triangleParallelThreshold = static_cast<double>(TriangleParallelTolerance) * TriangleParallelTolerance * edge1LengthSquared * edge2LengthSquared;
		if (triangleAreaVectorLengthSquared <= triangleParallelThreshold) { return false; }

		// 각 공간 변에 대응하는 UV 변화량이다.
		const float deltaU1 = v1.uv.x - v0.uv.x;
		const float deltaV1 = v1.uv.y - v0.uv.y;
		const float deltaU2 = v2.uv.x - v0.uv.x;
		const float deltaV2 = v2.uv.y - v0.uv.y;

		// UV의 두 변이 만드는 부호 있는 평행사변형 넓이다.
		const float uvDeterminant = deltaU1 * deltaV2 - deltaV1 * deltaU2;
		if (uvDeterminant == 0.0f) { return false; }

		const float uvEdge1LengthSquared = deltaU1 * deltaU1 + deltaV1 * deltaV1;
		const float uvEdge2LengthSquared = deltaU2 * deltaU2 + deltaV2 * deltaV2;
		if (!std::isfinite(uvDeterminant) || !std::isfinite(uvEdge1LengthSquared) || !std::isfinite(uvEdge2LengthSquared)) { return false; }

		// UV 크기와 무관하게 두 변이 거의 평행한지 비교한다. 비교식 양쪽의 곱셈은 double로 수행한다.
		constexpr float UVParallelTolerance = 1e-6f;
		const double uvParallelThreshold = static_cast<double>(UVParallelTolerance) * UVParallelTolerance * uvEdge1LengthSquared * uvEdge2LengthSquared;
		if (static_cast<double>(uvDeterminant) * uvDeterminant <= uvParallelThreshold) { return false; }

		const Vector3 triangleTangent = (edge1 * deltaV2 - edge2 * deltaV1) / uvDeterminant;
		if (!std::isfinite(triangleTangent.x) || !std::isfinite(triangleTangent.y) || !std::isfinite(triangleTangent.z)) { return false; }
		if (triangleTangent.x == 0.0f && triangleTangent.y == 0.0f && triangleTangent.z == 0.0f) { return false; }

		outTangent = triangleTangent;
		return true;
	}

	/**
	 * 삼각형별 tangent를 정점에 모아 vertices의 tangent를 채운다. indices는 읽기만 한다.
	 * 모든 정점의 계산이 성공한 뒤 tangent를 저장하고 true를 반환한다. 실패하면 vertices를 유지한다.
	 * hasTexcoords가 false이면 정점 번호 검사 후 삼각형 계산을 생략하고 정점별 대체 방향을 만든다.
	 */
	bool GenerateTangents(std::vector<MeshVertex>& vertices, const std::vector<std::uint32_t>& indices, bool hasTexcoords)
	{
		if (indices.size() % 3 != 0) { return false; }

		std::vector<Vector3> tangentSums(vertices.size());

		for (std::size_t triangleStart = 0; triangleStart < indices.size(); triangleStart += 3)
		{
			const std::uint32_t vertexIndex0 = indices[triangleStart];
			const std::uint32_t vertexIndex1 = indices[triangleStart + 1];
			const std::uint32_t vertexIndex2 = indices[triangleStart + 2];
			if (vertexIndex0 >= vertices.size() || vertexIndex1 >= vertices.size() || vertexIndex2 >= vertices.size()) { return false; }
			if (!hasTexcoords) { continue; }

			Vector3 triangleTangent;
			// 방향을 구할 수 없는 삼각형은 누적하지 않고 다음 삼각형으로 넘어간다.
			if (!CalculateTriangleTangent(vertices[vertexIndex0], vertices[vertexIndex1], vertices[vertexIndex2], triangleTangent)) { continue; }

			tangentSums[vertexIndex0] += triangleTangent;
			tangentSums[vertexIndex1] += triangleTangent;
			tangentSums[vertexIndex2] += triangleTangent;
		}

		for (std::size_t vertexIndex = 0; vertexIndex < vertices.size(); ++vertexIndex)
		{
			Vector3 normal = vertices[vertexIndex].normal;
			const float normalLengthSquared = normal.Dot(normal);
			if (!std::isfinite(normalLengthSquared) || normalLengthSquared <= 0.0f) { return false; }

			// 투영식은 단위 법선을 전제로 하므로 지역 복사본만 정규화한다.
			normal = normal / std::sqrt(normalLengthSquared);

			Vector3 tangent = tangentSums[vertexIndex];
			if (!std::isfinite(tangent.x) || !std::isfinite(tangent.y) || !std::isfinite(tangent.z)) { return false; }

			const float normalProjection = normal.Dot(tangent);
			tangent -= normal * normalProjection;

			const float tangentLengthSquared = tangent.Dot(tangent);
			if (!std::isfinite(tangentLengthSquared)) { return false; }

			if (tangentLengthSquared > 0.0f)
			{
				tangent = tangent / std::sqrt(tangentLengthSquared);
			}
			else
			{
				const float normalAbsX = std::abs(normal.x);
				const float normalAbsY = std::abs(normal.y);
				const float normalAbsZ = std::abs(normal.z);

				// 법선과 가장 덜 나란한 축을 골라 직교화 결과가 영벡터가 되는 것을 피한다.
				Vector3 referenceAxis;
				if (normalAbsX <= normalAbsY && normalAbsX <= normalAbsZ)
				{
					referenceAxis = Vector3{ 1.0f, 0.0f, 0.0f };
				}
				else if (normalAbsY <= normalAbsZ)
				{
					referenceAxis = Vector3{ 0.0f, 1.0f, 0.0f };
				}
				else
				{
					referenceAxis = Vector3{ 0.0f, 0.0f, 1.0f };
				}

				tangent = referenceAxis - normal * normal.Dot(referenceAxis);
				tangent = tangent / std::sqrt(tangent.Dot(tangent));
			}

			// 누적 배열을 최종 방향의 임시 저장소로 재사용한다.
			tangentSums[vertexIndex] = tangent;
		}

		for (std::size_t vertexIndex = 0; vertexIndex < vertices.size(); ++vertexIndex)
		{
			vertices[vertexIndex].tangent = tangentSums[vertexIndex];
		}

		return true;
	}

	/** accessors 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFAccessor> ReadAccessors(const rapidjson::Value& document)
	{
		std::vector<GLTFAccessor> accessors;
		if (!document.HasMember("accessors")) { return accessors; }

		const rapidjson::Value::ConstArray accessorArray = document["accessors"].GetArray();
		for (const rapidjson::Value& item : accessorArray)
		{
			GLTFAccessor accessor;

			if (item.HasMember("bufferView")) { accessor.bufferView = item["bufferView"].GetUint64(); }
			if (item.HasMember("byteOffset")) { accessor.byteOffset = item["byteOffset"].GetUint64(); }
			accessor.componentType = item["componentType"].GetInt();
			if (item.HasMember("normalized")) { accessor.normalized = item["normalized"].GetBool(); }
			accessor.count = item["count"].GetUint64();
			accessor.type = item["type"].GetString();

			if (item.HasMember("max"))
			{
				const rapidjson::Value::ConstArray maxArray = item["max"].GetArray();
				for (const rapidjson::Value& value : maxArray)
				{
					accessor.max.push_back(value.GetDouble());
				}
			}

			if (item.HasMember("min"))
			{
				const rapidjson::Value::ConstArray minArray = item["min"].GetArray();
				for (const rapidjson::Value& value : minArray)
				{
					accessor.min.push_back(value.GetDouble());
				}
			}

			if (item.HasMember("name")) { accessor.name = item["name"].GetString(); }

			accessors.push_back(std::move(accessor));
		}

		return accessors;
	}

	/** buffers 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFBuffer> ReadBuffers(const rapidjson::Value& document)
	{
		std::vector<GLTFBuffer> buffers;
		if (!document.HasMember("buffers")) { return buffers; }

		const rapidjson::Value::ConstArray bufferArray = document["buffers"].GetArray();
		for (const rapidjson::Value& item : bufferArray)
		{
			GLTFBuffer buffer;

			if (item.HasMember("uri")) { buffer.uri = item["uri"].GetString(); }
			buffer.byteLength = item["byteLength"].GetUint64();
			if (item.HasMember("name")) { buffer.name = item["name"].GetString(); }

			buffers.push_back(std::move(buffer));
		}

		return buffers;
	}

	/** bufferViews 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFBufferView> ReadBufferViews(const rapidjson::Value& document)
	{
		std::vector<GLTFBufferView> bufferViews;
		if (!document.HasMember("bufferViews")) { return bufferViews; }

		const rapidjson::Value::ConstArray bufferViewArray = document["bufferViews"].GetArray();
		for (const rapidjson::Value& item : bufferViewArray)
		{
			GLTFBufferView bufferView;

			bufferView.buffer = item["buffer"].GetUint64();
			if (item.HasMember("byteOffset")) { bufferView.byteOffset = item["byteOffset"].GetUint64(); }
			bufferView.byteLength = item["byteLength"].GetUint64();
			if (item.HasMember("byteStride")) { bufferView.byteStride = item["byteStride"].GetUint64(); }
			if (item.HasMember("target")) { bufferView.target = item["target"].GetInt(); }
			if (item.HasMember("name")) { bufferView.name = item["name"].GetString(); }

			bufferViews.push_back(std::move(bufferView));
		}

		return bufferViews;
	}

	/** images 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFImage> ReadImages(const rapidjson::Value& document)
	{
		std::vector<GLTFImage> images;
		if (!document.HasMember("images")) { return images; }

		const rapidjson::Value::ConstArray imageArray = document["images"].GetArray();
		for (const rapidjson::Value& item : imageArray)
		{
			GLTFImage image;

			if (item.HasMember("uri")) { image.uri = item["uri"].GetString(); }
			if (item.HasMember("mimeType")) { image.mimeType = item["mimeType"].GetString(); }
			if (item.HasMember("bufferView")) { image.bufferView = item["bufferView"].GetUint64(); }
			if (item.HasMember("name")) { image.name = item["name"].GetString(); }

			images.push_back(std::move(image));
		}

		return images;
	}

	/** 재질의 텍스처 참조(textureInfo) 객체 하나를 읽는다. */
	GLTFTextureInfo ReadTextureInfo(const rapidjson::Value& item)
	{
		GLTFTextureInfo textureInfo;

		textureInfo.index = item["index"].GetUint64();
		if (item.HasMember("texCoord")) { textureInfo.texCoord = item["texCoord"].GetUint64(); }

		return textureInfo;
	}

	/** materials 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. pbrMetallicRoughness 외의 항목은 읽지 않는다. */
	std::vector<GLTFMaterial> ReadMaterials(const rapidjson::Value& document)
	{
		std::vector<GLTFMaterial> materials;
		if (!document.HasMember("materials")) { return materials; }

		const rapidjson::Value::ConstArray materialArray = document["materials"].GetArray();
		for (const rapidjson::Value& item : materialArray)
		{
			GLTFMaterial material;

			if (item.HasMember("name")) { material.name = item["name"].GetString(); }

			if (item.HasMember("pbrMetallicRoughness"))
			{
				const rapidjson::Value& pbrItem = item["pbrMetallicRoughness"];
				GLTFPBRMetallicRoughness& pbr = material.pbrMetallicRoughness;

				if (pbrItem.HasMember("baseColorFactor"))
				{
					const rapidjson::Value::ConstArray factorArray = pbrItem["baseColorFactor"].GetArray();
					for (rapidjson::SizeType i = 0; i < 4; ++i)
					{
						pbr.baseColorFactor[i] = factorArray[i].GetFloat();
					}
				}

				if (pbrItem.HasMember("baseColorTexture")) { pbr.baseColorTexture = ReadTextureInfo(pbrItem["baseColorTexture"]); }
				if (pbrItem.HasMember("metallicFactor")) { pbr.metallicFactor = pbrItem["metallicFactor"].GetFloat(); }
				if (pbrItem.HasMember("roughnessFactor")) { pbr.roughnessFactor = pbrItem["roughnessFactor"].GetFloat(); }
			}

			materials.push_back(std::move(material));
		}

		return materials;
	}

	/** meshes 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFMesh> ReadMeshes(const rapidjson::Value& document)
	{
		std::vector<GLTFMesh> meshes;
		if (!document.HasMember("meshes")) { return meshes; }

		const rapidjson::Value::ConstArray meshArray = document["meshes"].GetArray();
		for (const rapidjson::Value& item : meshArray)
		{
			GLTFMesh mesh;

			const rapidjson::Value::ConstArray primitiveArray = item["primitives"].GetArray();
			for (const rapidjson::Value& primitiveItem : primitiveArray)
			{
				GLTFPrimitive primitive;

				const rapidjson::Value::ConstObject attributeObject = primitiveItem["attributes"].GetObject();
				for (const rapidjson::Value::Member& attribute : attributeObject)
				{
					primitive.attributes[attribute.name.GetString()] = attribute.value.GetUint64();
				}
				if (primitiveItem.HasMember("indices")) { primitive.indices = primitiveItem["indices"].GetUint64(); }
				if (primitiveItem.HasMember("material")) { primitive.material = primitiveItem["material"].GetUint64(); }
				if (primitiveItem.HasMember("mode")) { primitive.mode = primitiveItem["mode"].GetInt(); }

				mesh.primitives.push_back(std::move(primitive));
			}

			if (item.HasMember("name")) { mesh.name = item["name"].GetString(); }

			meshes.push_back(std::move(mesh));
		}

		return meshes;
	}

	/** nodes 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. */
	std::vector<GLTFNode> ReadNodes(const rapidjson::Value& document)
	{
		std::vector<GLTFNode> nodes;
		if (!document.HasMember("nodes")) { return nodes; }

		const rapidjson::Value::ConstArray nodeArray = document["nodes"].GetArray();
		for (const rapidjson::Value& item : nodeArray)
		{
			GLTFNode node;

			if (item.HasMember("children"))
			{
				const rapidjson::Value::ConstArray childArray = item["children"].GetArray();
				for (const rapidjson::Value& child : childArray)
				{
					node.children.push_back(child.GetUint64());
				}
			}

			if (item.HasMember("matrix"))
			{
				const rapidjson::Value::ConstArray matrixArray = item["matrix"].GetArray();
				std::array<float, 16> matrix = {};
				for (rapidjson::SizeType i = 0; i < 16; ++i)
				{
					matrix[i] = matrixArray[i].GetFloat();
				}
				node.matrix = matrix;
			}

			if (item.HasMember("mesh")) { node.mesh = item["mesh"].GetUint64(); }

			if (item.HasMember("rotation"))
			{
				const rapidjson::Value::ConstArray rotationArray = item["rotation"].GetArray();
				for (rapidjson::SizeType i = 0; i < 4; ++i)
				{
					node.rotation[i] = rotationArray[i].GetFloat();
				}
			}

			if (item.HasMember("scale"))
			{
				const rapidjson::Value::ConstArray scaleArray = item["scale"].GetArray();
				for (rapidjson::SizeType i = 0; i < 3; ++i)
				{
					node.scale[i] = scaleArray[i].GetFloat();
				}
			}

			if (item.HasMember("translation"))
			{
				const rapidjson::Value::ConstArray translationArray = item["translation"].GetArray();
				for (rapidjson::SizeType i = 0; i < 3; ++i)
				{
					node.translation[i] = translationArray[i].GetFloat();
				}
			}

			if (item.HasMember("name")) { node.name = item["name"].GetString(); }

			nodes.push_back(std::move(node));
		}

		return nodes;
	}

	/** textures 배열을 읽는다. 배열이 없으면 빈 배열을 돌려준다. sampler는 읽지 않는다. */
	std::vector<GLTFTexture> ReadTextures(const rapidjson::Value& document)
	{
		std::vector<GLTFTexture> textures;
		if (!document.HasMember("textures")) { return textures; }

		const rapidjson::Value::ConstArray textureArray = document["textures"].GetArray();
		for (const rapidjson::Value& item : textureArray)
		{
			GLTFTexture texture;

			if (item.HasMember("source")) { texture.source = item["source"].GetUint64(); }
			if (item.HasMember("name")) { texture.name = item["name"].GetString(); }

			textures.push_back(std::move(texture));
		}

		return textures;
	}

	/** 각 buffer의 uri가 가리키는 파일을 읽어 bufferData를 채운다. 외부 파일 uri만 지원한다. 실패하면 bufferData를 바꾸지 않는다. */
	bool LoadBufferData(const std::filesystem::path& gltfPath, GLTFDocument& gltfDocument)
	{
		std::vector<std::vector<std::byte>> bufferData;
		for (const GLTFBuffer& buffer : gltfDocument.buffers)
		{
			// .glb 안의 데이터(uri 없음)와 base64 data URI는 아직 지원하지 않는다.
			if (buffer.uri.empty() || buffer.uri.starts_with("data:")) { return false; }

			// uri는 .gltf 파일이 있는 폴더 기준 상대 경로이며 UTF-8이다.
			const std::filesystem::path bufferPath = gltfPath.parent_path() / std::filesystem::path(std::u8string(buffer.uri.begin(), buffer.uri.end()));

			std::vector<std::byte> bytes;
			if (!FileHelper::ReadAllBytes(bufferPath, bytes)) { return false; }

			// 파일이 byteLength보다 작으면 accessor가 범위 밖을 읽게 된다.
			if (bytes.size() < buffer.byteLength) { return false; }

			bufferData.push_back(std::move(bytes));
		}

		gltfDocument.bufferData = std::move(bufferData);

		return true;
	}

	/** accessor의 type 문자열(SCALAR, VEC2 ...)이 뜻하는 숫자 개수. 모르는 값이면 0 */
	std::size_t GetComponentCount(const std::string& type)
	{
		if (type == "SCALAR") { return 1; }
		if (type == "VEC2") { return 2; }
		if (type == "VEC3") { return 3; }
		if (type == "VEC4") { return 4; }
		if (type == "MAT2") { return 4; }
		if (type == "MAT3") { return 9; }
		if (type == "MAT4") { return 16; }

		return 0;
	}

	/**
	 * accessor 값들이 바이트 배열의 어디에 있는지 찾는다. elementSize는 값 하나의 바이트 수다.
	 * bufferView가 없거나 번호가 범위를 벗어나거나 값이 버퍼 밖에 있으면 출력값을 유지하고 false를 반환한다.
	 */
	bool FindAccessorElements(const GLTFDocument& gltfDocument, const GLTFAccessor& accessor, std::size_t elementSize, const std::byte*& outFirstElement, std::size_t& outStride)
	{
		// bufferView가 없는(값이 모두 0인) accessor는 아직 지원하지 않는다.
		if (!accessor.bufferView.has_value()) { return false; }

		// 번호들은 파일에 적힌 값이라, 배열 범위를 벗어나면 잘못된 메모리를 읽게 되므로 쓰기 전에 확인한다.
		const std::size_t bufferViewIndex = accessor.bufferView.value();
		if (bufferViewIndex >= gltfDocument.bufferViews.size()) { return false; }

		// accessor -> bufferView -> buffer 순서로 값이 들어 있는 바이트를 찾아간다.
		const GLTFBufferView& bufferView = gltfDocument.bufferViews[bufferViewIndex];
		if (bufferView.buffer >= gltfDocument.bufferData.size()) { return false; }

		const std::vector<std::byte>& bytes = gltfDocument.bufferData[bufferView.buffer];

		// i번째 값은 start + i * stride에서 시작한다. byteStride가 없으면 값이 빈틈없이 붙어 있다.
		const std::size_t stride = bufferView.byteStride.value_or(elementSize);
		const std::size_t start = bufferView.byteOffset + accessor.byteOffset;

		// 마지막 값의 끝이 버퍼 안에 있어야 한다.
		if (accessor.count > 0 && start + stride * (accessor.count - 1) + elementSize > bytes.size()) { return false; }

		outFirstElement = bytes.data() + start;
		outStride = stride;

		return true;
	}

	/** float accessor의 값을 숫자 배열로 읽는다. 값 하나는 componentCount개의 float이며 배열에 차례로 담긴다. */
	bool ReadFloatAccessor(const GLTFDocument& gltfDocument, std::size_t accessorIndex, std::size_t componentCount, std::vector<float>& outValues)
	{
		// 번호들은 파일에 적힌 값이라, 배열 범위를 벗어나면 잘못된 메모리를 읽게 되므로 쓰기 전에 확인한다.
		if (accessorIndex >= gltfDocument.accessors.size()) { return false; }

		const GLTFAccessor& accessor = gltfDocument.accessors[accessorIndex];

		// float(5126)이고 숫자 개수가 맞아야 한다.
		if (accessor.componentType != 5126 || GetComponentCount(accessor.type) != componentCount) { return false; }

		const std::size_t elementSize = sizeof(float) * componentCount;
		const std::byte* firstElement = nullptr;
		std::size_t stride = 0;
		if (!FindAccessorElements(gltfDocument, accessor, elementSize, firstElement, stride)) { return false; }

		std::vector<float> values(accessor.count * componentCount);
		for (std::size_t i = 0; i < accessor.count; ++i)
		{
			const std::byte* sourceElementBytes = firstElement + stride * i;
			float* destinationComponents = &values[i * componentCount];

			// 정렬과 타입 규칙 문제 없이 바이트를 float로 옮기려고 memcpy를 쓴다.
			std::memcpy(destinationComponents, sourceElementBytes, elementSize);
		}

		outValues = std::move(values);

		return true;
	}

	/** 인덱스 accessor(SCALAR, uint8/uint16/uint32)의 값을 uint32로 넓혀 읽는다. */
	bool ReadIndexAccessor(const GLTFDocument& gltfDocument, std::size_t accessorIndex, std::vector<std::uint32_t>& outIndices)
	{
		// ----------
		// 인덱스 형식 검사 및 읽기 위치 확인
		// ----------
		// 번호들은 파일에 적힌 값이라, 배열 범위를 벗어나면 잘못된 메모리를 읽게 되므로 쓰기 전에 확인한다.
		if (accessorIndex >= gltfDocument.accessors.size()) { return false; }

		const GLTFAccessor& accessor = gltfDocument.accessors[accessorIndex];

		// 인덱스는 숫자 하나(SCALAR)다.
		if (accessor.type != "SCALAR") { return false; }

		// componentType에 따라 인덱스 하나의 바이트 수가 다르다. 5121 uint8, 5123 uint16, 5125 uint32
		std::size_t indexSize = 0;
		if (accessor.componentType == 5121) { indexSize = 1; }
		else if (accessor.componentType == 5123) { indexSize = 2; }
		else if (accessor.componentType == 5125) { indexSize = 4; }
		else { return false; }

		const std::byte* firstElement = nullptr;
		std::size_t stride = 0;
		if (!FindAccessorElements(gltfDocument, accessor, indexSize, firstElement, stride)) { return false; }

		// ----------
		// 인덱스를 uint32 배열로 변환
		// ----------
		std::vector<std::uint32_t> indices(accessor.count);
		for (std::size_t i = 0; i < accessor.count; ++i)
		{
			const std::byte* sourceElementBytes = firstElement + stride * i;

			// 작은 타입은 같은 크기의 변수로 복사한 뒤 uint32로 넓힌다.
			if (indexSize == 1)
			{
				std::uint8_t index = 0;
				std::memcpy(&index, sourceElementBytes, sizeof(index));
				indices[i] = index;
			}
			else if (indexSize == 2)
			{
				std::uint16_t index = 0;
				std::memcpy(&index, sourceElementBytes, sizeof(index));
				indices[i] = index;
			}
			else
			{
				std::memcpy(&indices[i], sourceElementBytes, sizeof(std::uint32_t));
			}
		}

		outIndices = std::move(indices);

		return true;
	}

	/** 노드의 이동, 회전, 크기를 엔진 좌표계(왼손, cm)의 Transform으로 바꾼다. matrix로 적힌 노드는 행렬을 나눠서 바꾼다. */
	Transform ConvertNodeTransform(const GLTFNode& node)
	{
		// ----------
		// 담기: glTF 좌표 그대로의 이동, 회전, 크기
		// ----------
		// 명세상 한 노드는 matrix와 translation, rotation, scale 중 한 방식으로만 변환을 적는다.
		Transform source;
		if (node.matrix.has_value())
		{
			// glTF는 열 우선 순서라, 16개 값을 행 단위로 채우면 엔진의 행 벡터 규칙 행렬이 된다.
			const std::array<float, 16>& values = node.matrix.value();
			Matrix4x4 matrix;
			for (int row = 0; row < 4; ++row)
			{
				for (int column = 0; column < 4; ++column)
				{
					matrix.m[row][column] = values[row * 4 + column];
				}
			}

			// 행렬을 glTF 좌표 그대로의 이동, 회전, 크기로 나눈다.
			source = Transform::CreateFromMatrix(matrix);
		}
		else
		{
			source.position.x = node.translation[0];
			source.position.y = node.translation[1];
			source.position.z = node.translation[2];

			source.rotation.x = node.rotation[0];
			source.rotation.y = node.rotation[1];
			source.rotation.z = node.rotation[2];
			source.rotation.w = node.rotation[3];

			source.scale.x = node.scale[0];
			source.scale.y = node.scale[1];
			source.scale.z = node.scale[2];
		}

		// ----------
		// 바꾸기: 엔진 좌표계(왼손, cm)로
		// ----------
		Transform transform;

		// 정점 위치와 같이 x를 뒤집고 cm로 바꾼다.
		transform.position.x = -source.position.x * MetersToCentimeters;
		transform.position.y = source.position.y * MetersToCentimeters;
		transform.position.z = source.position.z * MetersToCentimeters;

		// x를 뒤집으면 Y축, Z축 기준 회전의 방향이 반대가 되므로 y와 z 성분의 부호를 바꾼다.
		transform.rotation.x = source.rotation.x;
		transform.rotation.y = -source.rotation.y;
		transform.rotation.z = -source.rotation.z;
		transform.rotation.w = source.rotation.w;

		// 크기는 방향이 없는 축별 배율이라 그대로 쓴다.
		transform.scale = source.scale;

		return transform;
	}

	/** .gltf 파일과 외부 버퍼를 읽어 glTF 원본 그대로의 문서를 채운다. 실패하면 출력값을 유지하고 false를 반환한다. */
	bool LoadDocument(const std::filesystem::path& path, GLTFDocument& outDocument)
	{
		std::string gltfText;
		if (!FileHelper::ReadAllText(path, gltfText)) { return false; }

		rapidjson::Document document;
		document.Parse(gltfText.data(), gltfText.size());
		if (document.HasParseError() || !document.IsObject()) { return false; }

		GLTFDocument gltfDocument;
		gltfDocument.accessors = ReadAccessors(document);
		gltfDocument.buffers = ReadBuffers(document);
		gltfDocument.bufferViews = ReadBufferViews(document);
		gltfDocument.images = ReadImages(document);
		gltfDocument.materials = ReadMaterials(document);
		gltfDocument.meshes = ReadMeshes(document);
		gltfDocument.nodes = ReadNodes(document);
		gltfDocument.textures = ReadTextures(document);
		if (!LoadBufferData(path, gltfDocument)) { return false; }

		outDocument = std::move(gltfDocument);

		return true;
	}

	/**
	 * LoadDocument로 채운 원본 문서의 materialIndex번 재질을 엔진 재질 설명으로 바꿔 채운다.
	 * 계수가 0~1 밖이거나 번호가 범위를 벗어나거나, 지원하지 않는 형식의 색 텍스처면 출력값을 유지하고 false를 반환한다.
	 */
	bool ConvertMaterial(const GLTFDocument& gltfDocument, std::size_t materialIndex, GLTFModelMaterial& outMaterial)
	{
		if (materialIndex >= gltfDocument.materials.size()) { return false; }

		const GLTFMaterial& source = gltfDocument.materials[materialIndex];
		const GLTFPBRMetallicRoughness& pbr = source.pbrMetallicRoughness;

		// 명세상 계수는 모두 0~1이다. 엔진 재질도 같은 범위만 받으므로 벗어나면 잘못된 파일로 본다.
		const auto isUnitRange = [](float value) { return std::isfinite(value) && value >= 0.0f && value <= 1.0f; };
		if (!isUnitRange(pbr.baseColorFactor[0]) || !isUnitRange(pbr.baseColorFactor[1]) || !isUnitRange(pbr.baseColorFactor[2])) { return false; }
		if (!isUnitRange(pbr.metallicFactor) || !isUnitRange(pbr.roughnessFactor)) { return false; }

		GLTFModelMaterial material;
		material.baseColor = Vector3{ pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2] };
		material.metallic = pbr.metallicFactor;
		material.roughness = pbr.roughnessFactor;
		material.name = source.name;

		// 색 텍스처는 재질 -> textures -> images 순서로 번호를 따라가 이미지 경로를 찾는다.
		if (pbr.baseColorTexture.has_value())
		{
			const GLTFTextureInfo& textureInfo = pbr.baseColorTexture.value();

			// 정점에는 TEXCOORD_0만 담으므로 다른 UV를 쓰는 텍스처는 바르게 붙일 수 없다.
			if (textureInfo.texCoord != 0) { return false; }
			if (textureInfo.index >= gltfDocument.textures.size()) { return false; }

			// source가 없으면 확장이 이미지를 정하는데, 확장은 지원하지 않는다.
			const GLTFTexture& texture = gltfDocument.textures[textureInfo.index];
			if (!texture.source.has_value() || texture.source.value() >= gltfDocument.images.size()) { return false; }

			// .glb 안(bufferView)이나 data URI의 이미지는 아직 지원하지 않는다.
			const GLTFImage& image = gltfDocument.images[texture.source.value()];
			if (image.uri.empty() || image.uri.starts_with("data:")) { return false; }

			material.baseColorImageUri = image.uri;
		}

		outMaterial = std::move(material);

		return true;
	}

	/**
	 * LoadDocument로 채운 원본 문서의 노드들을 엔진 좌표계의 배치표로 바꿔 채운다.
	 * 배치표 번호는 glTF nodes 번호와 같다. 실패하면 출력값을 유지하고 false를 반환한다.
	 */
	bool ConvertSceneNodes(const GLTFDocument& gltfDocument, std::vector<GLTFSceneNode>& outSceneNodes)
	{
		// 원본 노드 수만큼 줄을 만든다. 줄 번호가 glTF 노드 번호와 같아진다.
		std::vector<GLTFSceneNode> sceneNodes(gltfDocument.nodes.size());

		for (std::size_t i = 0; i < gltfDocument.nodes.size(); ++i)
		{
			// 메시 번호는 파일에 적힌 값이라, 범위를 벗어나면 나중에 없는 메시를 찾게 되므로 확인한다.
			if (gltfDocument.nodes[i].mesh.has_value() && gltfDocument.nodes[i].mesh.value() >= gltfDocument.meshes.size()) { return false; }

			sceneNodes[i].mesh = gltfDocument.nodes[i].mesh;
			sceneNodes[i].localTransform = ConvertNodeTransform(gltfDocument.nodes[i]);
			sceneNodes[i].name = gltfDocument.nodes[i].name;
		}

		// glTF의 자식 목록으로 각 노드의 부모와 자식 목록을 함께 채운다.
		for (std::size_t i = 0; i < gltfDocument.nodes.size(); ++i)
		{
			for (const std::size_t child : gltfDocument.nodes[i].children)
			{
				// 자식 번호는 파일에 적힌 값이라, 범위를 벗어나면 없는 노드에 쓰게 되므로 확인한다.
				if (child >= sceneNodes.size()) { return false; }

				// 명세상 노드의 부모는 하나뿐이다. 이미 부모가 적힌 자식을 다시 만나면 잘못된 파일이다.
				if (sceneNodes[child].parent.has_value()) { return false; }

				// 같은 관계를 두 방향으로 함께 적어 서로 어긋나지 않게 한다.
				sceneNodes[child].parent = i;
				sceneNodes[i].children.push_back(child);
			}
		}

		// 부모를 따라 올라가 최상위에 닿는지 확인한다. 부모는 하나뿐이라 순환이 없으면 노드 수보다 적은 걸음에 끝난다.
		for (std::size_t i = 0; i < sceneNodes.size(); ++i)
		{
			std::size_t current = i;
			std::size_t steps = 0;
			while (sceneNodes[current].parent.has_value())
			{
				current = sceneNodes[current].parent.value();
				++steps;

				// 노드 수만큼 올라가도 끝나지 않으면 같은 노드를 다시 지난 것이므로 순환이다.
				if (steps >= sceneNodes.size()) { return false; }
			}
		}

		outSceneNodes = std::move(sceneNodes);

		return true;
	}

	/**
	 * LoadDocument로 채운 원본 문서의 meshIndex번 메시의 첫 프리미티브를 엔진 좌표계(왼손, cm)의 정점과 인덱스로 바꿔 채운다.
	 * 이름과 경로는 유지하며, 실패하면 출력값을 유지하고 false를 반환한다.
	 */
	bool ConvertStaticMesh(const GLTFDocument& gltfDocument, std::size_t meshIndex, StaticMesh& outMeshData)
	{
		// ----------
		// 프리미티브 선택 및 정점 속성과 인덱스 읽기
		// ----------
		// 메시 번호가 범위를 벗어나면 없는 메시를 읽게 되므로 확인한다. 메시의 첫 프리미티브만 읽으며, 메시가 있으면 프리미티브도 하나 이상이다.
		if (meshIndex >= gltfDocument.meshes.size()) { return false; }
		const GLTFPrimitive& primitive = gltfDocument.meshes[meshIndex].primitives[0];

		// 삼각형 목록(4, 인덱스 3개마다 삼각형 하나)만 지원한다.
		if (primitive.mode != 4) { return false; }

		// 위치가 없으면 그릴 수 없으므로 실패로 본다.
		const auto positionAttribute = primitive.attributes.find("POSITION");
		if (positionAttribute == primitive.attributes.end()) { return false; }

		// 인덱스는 glTF에서 없어도 되지만, 아직 인덱스가 있는 메시만 지원한다.
		if (!primitive.indices) { return false; }

		std::vector<float> positions;
		if (!ReadFloatAccessor(gltfDocument, positionAttribute->second, 3, positions)) { return false; }

		// 법선은 아직 직접 계산하지 않으므로 없으면 실패로 본다.
		const auto normalAttribute = primitive.attributes.find("NORMAL");
		if (normalAttribute == primitive.attributes.end()) { return false; }

		std::vector<float> normals;
		if (!ReadFloatAccessor(gltfDocument, normalAttribute->second, 3, normals)) { return false; }

		// UV는 없어도 된다. 없으면 texcoords가 비어 있다.
		std::vector<float> texcoords;
		const auto texcoordAttribute = primitive.attributes.find("TEXCOORD_0");
		if (texcoordAttribute != primitive.attributes.end())
		{
			if (!ReadFloatAccessor(gltfDocument, texcoordAttribute->second, 2, texcoords)) { return false; }
		}

		std::vector<std::uint32_t> indices;
		if (!ReadIndexAccessor(gltfDocument, primitive.indices.value(), indices)) { return false; }

		// 법선과 UV는 위치와 정점 수가 같아야 한다.
		const std::size_t vertexCount = positions.size() / 3;
		if (normals.size() != vertexCount * 3) { return false; }
		if (!texcoords.empty() && texcoords.size() != vertexCount * 2) { return false; }

		// ----------
		// 엔진 좌표계로 변환 및 메시 조립
		// ----------
		// glTF는 오른손 좌표계, 미터 단위다. 엔진(왼손, cm)에 맞추려고 x를 뒤집고 위치에 100을 곱한다.
		// x를 뒤집으면 모델 앞면(glTF +Z)과 오른쪽(glTF -X)이 엔진의 +Z 앞, +X 오른쪽과 맞는다.
		std::vector<MeshVertex> vertices(vertexCount);
		for (std::size_t i = 0; i < vertexCount; ++i)
		{
			MeshVertex& vertex = vertices[i];

			const float* position = &positions[i * 3];
			vertex.position.x = -position[0] * MetersToCentimeters;
			vertex.position.y = position[1] * MetersToCentimeters;
			vertex.position.z = position[2] * MetersToCentimeters;

			// 법선은 길이 1인 방향이라 단위를 바꾸지 않고 x만 뒤집는다.
			const float* normal = &normals[i * 3];
			vertex.normal.x = -normal[0];
			vertex.normal.y = normal[1];
			vertex.normal.z = normal[2];

			// glTF와 엔진 모두 UV 원점이 왼쪽 위라 그대로 쓴다. UV가 없으면 0으로 남는다.
			if (!texcoords.empty())
			{
				const float* texcoord = &texcoords[i * 2];
				vertex.uv.x = texcoord[0];
				vertex.uv.y = texcoord[1];
			}
		}

		// 삼각형 목록이라 인덱스 3개가 삼각형 하나다.
		if (indices.size() % 3 != 0) { return false; }

		// 인덱스는 파일에 적힌 값이라, 정점 수를 넘으면 그릴 때 없는 정점을 읽게 되므로 확인한다.
		for (const std::uint32_t index : indices)
		{
			if (index >= vertexCount) { return false; }
		}

		// x를 뒤집으면 삼각형이 도는 방향도 뒤집힌다. 엔진의 앞면 약속에 맞추려고 각 삼각형의 2번째와 3번째를 바꾼다.
		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			std::swap(indices[i + 1], indices[i + 2]);
		}

		const bool hasTexcoords = texcoordAttribute != primitive.attributes.end();
		if (!GenerateTangents(vertices, indices, hasTexcoords)) { return false; }

		outMeshData.vertices = std::move(vertices);
		outMeshData.indices = std::move(indices);

		return true;
	}
}

bool GLTFLoader::LoadModel(const std::filesystem::path& path, GLTFModel& outModel)
{
	// ----------
	// glTF 문서와 외부 버퍼 읽기
	// ----------
	GLTFDocument gltfDocument;
	if (!LoadDocument(path, gltfDocument)) { return false; }

	GLTFModel model;

	// ----------
	// 재질마다 설명 만들기
	// ----------
	model.materials.resize(gltfDocument.materials.size());
	for (std::size_t i = 0; i < gltfDocument.materials.size(); ++i)
	{
		if (!ConvertMaterial(gltfDocument, i, model.materials[i])) { return false; }
	}

	// ----------
	// 메시마다 StaticMesh와 재질 번호 만들기
	// ----------
	// 이름은 glTF 메시 이름을 쓰고, 비어 있으면 "파일이름_mesh_번호"로 정한다. 경로는 받는 쪽이 정하므로 비워 둔다.
	const std::u8string fileStem = path.stem().u8string();
	model.meshes.reserve(gltfDocument.meshes.size());
	model.meshMaterials.reserve(gltfDocument.meshes.size());
	for (std::size_t i = 0; i < gltfDocument.meshes.size(); ++i)
	{
		std::string name = gltfDocument.meshes[i].name;
		if (name.empty())
		{
			name = std::string(fileStem.begin(), fileStem.end()) + "_mesh_" + std::to_string(i);
		}

		model.meshes.emplace_back(std::move(name), std::string());
		if (!ConvertStaticMesh(gltfDocument, i, model.meshes.back())) { return false; }

		// 첫 프리미티브의 재질 번호를 메시의 재질로 쓴다. 번호는 파일에 적힌 값이라 범위를 확인한다.
		const std::optional<std::size_t>& materialIndex = gltfDocument.meshes[i].primitives[0].material;
		if (materialIndex.has_value() && materialIndex.value() >= gltfDocument.materials.size()) { return false; }
		model.meshMaterials.push_back(materialIndex);
	}

	// ----------
	// 배치표 만들기
	// ----------
	if (!ConvertSceneNodes(gltfDocument, model.sceneNodes)) { return false; }

	outModel = std::move(model);

	return true;
}
