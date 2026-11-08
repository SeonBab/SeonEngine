#include "GLTFLoader.h"

#include "Core/FileHelper.h"
#include "GLTFDocument.h"
#include "Mesh/MeshVertex.h"
#include "Mesh/StaticMeshData.h"

#include "rapidjson/document.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace
{
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
}

bool GLTFLoader::LoadStaticMesh(const std::filesystem::path& path, StaticMeshData& outMeshData)
{
	// ----------
	// glTF 문서와 외부 버퍼 읽기
	// ----------
	std::string gltfText;
	if (!FileHelper::ReadAllText(path, gltfText)) { return false; }

	rapidjson::Document document;
	document.Parse(gltfText.data(), gltfText.size());
	if (document.HasParseError() || !document.IsObject()) { return false; }

	GLTFDocument gltfDocument;
	gltfDocument.accessors = ReadAccessors(document);
	gltfDocument.buffers = ReadBuffers(document);
	gltfDocument.bufferViews = ReadBufferViews(document);
	gltfDocument.meshes = ReadMeshes(document);
	if (!LoadBufferData(path, gltfDocument)) { return false; }

	// ----------
	// 프리미티브 선택 및 정점 속성과 인덱스 읽기
	// ----------
	// 첫 메시의 첫 프리미티브만 읽는다. 메시가 하나 이상이면 프리미티브도 하나 이상이다.
	if (gltfDocument.meshes.empty()) { return false; }
	const GLTFPrimitive& primitive = gltfDocument.meshes[0].primitives[0];

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
	constexpr float metersToCentimeters = 100.0f;

	std::vector<MeshVertex> vertices(vertexCount);
	for (std::size_t i = 0; i < vertexCount; ++i)
	{
		MeshVertex& vertex = vertices[i];

		const float* position = &positions[i * 3];
		vertex.position.x = -position[0] * metersToCentimeters;
		vertex.position.y = position[1] * metersToCentimeters;
		vertex.position.z = position[2] * metersToCentimeters;

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

		// tangent는 노멀맵을 쓸 때 계산한다. 지금은 0으로 둔다.
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

	outMeshData.vertices = std::move(vertices);
	outMeshData.indices = std::move(indices);

	return true;
}
