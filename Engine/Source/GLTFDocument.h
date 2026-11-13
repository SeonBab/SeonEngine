#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

// glTF 2.0 JSON 항목을 담는 구조체들이다. 구조체와 필드 순서는 glTF 2.0 명세의 순서를 따른다.

/** glTF accessors의 항목 하나. bufferView의 바이트를 어떤 타입의 값 몇 개로 읽을지 정한다. */
struct GLTFAccessor
{
	// 선택. 값이 들어 있는 bufferViews 번호. 없으면 값이 모두 0이다.
	std::optional<std::size_t> bufferView;
	// 선택. bufferView 처음부터 첫 값까지의 바이트 수. 없으면 0
	std::size_t byteOffset = 0;
	// 필수. 숫자 하나의 타입. 5120 int8, 5121 uint8, 5122 int16, 5123 uint16, 5125 uint32, 5126 float
	int componentType = 0;
	// 선택. 정수를 0~1(부호 있으면 -1~1)로 바꿔 읽을지 여부. 없으면 false
	bool normalized = false;
	// 필수. 값의 개수
	std::size_t count = 0;
	// 필수. 값 하나를 이루는 숫자 개수. SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4
	std::string type;
	// 선택. 숫자별 최댓값과 최솟값. POSITION accessor에는 반드시 있다.
	std::vector<double> max;
	std::vector<double> min;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF buffers의 항목 하나. 정점, 인덱스 같은 바이너리 데이터 덩어리다. */
struct GLTFBuffer
{
	// 선택. .gltf 폴더 기준 파일 이름 또는 base64 data URI. 없으면 .glb 안의 데이터다.
	std::string uri;
	// 필수. 바이트 단위 크기
	std::size_t byteLength = 0;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF bufferViews의 항목 하나. buffer 안의 한 구간이다. */
struct GLTFBufferView
{
	// 필수. 이 구간이 들어 있는 buffers 번호
	std::size_t buffer = 0;
	// 선택. buffer 처음부터 구간 시작까지의 바이트 수. 없으면 0
	std::size_t byteOffset = 0;
	// 필수. 구간의 바이트 수
	std::size_t byteLength = 0;
	// 선택. 한 값의 시작에서 다음 값의 시작까지의 바이트 수(4~252, 4의 배수). 없으면 값이 빈틈없이 붙어 있다.
	std::optional<std::size_t> byteStride;
	// 선택. GPU 버퍼 용도 힌트. 34962는 정점, 34963은 인덱스
	std::optional<int> target;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF images의 항목 하나. uri 또는 bufferView 중 하나로 이미지 파일 데이터를 가리킨다. */
struct GLTFImage
{
	// 선택. .gltf 폴더 기준 상대 경로 또는 data URI. bufferView를 쓰면 비어 있다.
	std::string uri;
	// 선택. image/jpeg 또는 image/png. bufferView를 쓰면 반드시 있다.
	std::string mimeType;
	// 선택. 이미지 파일 데이터가 들어 있는 bufferViews 번호. uri를 쓰면 없다.
	std::optional<std::size_t> bufferView;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** 재질이 텍스처를 가리키는 참조(textureInfo)다. */
struct GLTFTextureInfo
{
	// 필수. textures 번호
	std::size_t index = 0;
	// 선택. 사용할 TEXCOORD_n 속성의 n. 없으면 0
	std::size_t texCoord = 0;
};

/** glTF material.pbrMetallicRoughness. 금속성-거칠기 모델의 값이다. */
struct GLTFPBRMetallicRoughness
{
	// 선택. 선형 RGBA 계수. 색 텍스처가 있으면 텍스처 값에 곱한다. 없으면 (1, 1, 1, 1)
	std::array<float, 4> baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 선택. 색 텍스처. 텍스처의 RGB는 sRGB, A는 선형 값이다.
	std::optional<GLTFTextureInfo> baseColorTexture;
	// 선택. 금속성 계수(0~1). 없으면 1
	float metallicFactor = 1.0f;
	// 선택. 거칠기 계수(0~1). 없으면 1
	float roughnessFactor = 1.0f;
};

/** glTF materials의 항목 하나. 지금은 pbrMetallicRoughness만 담는다. */
struct GLTFMaterial
{
	// 선택. 사람이 보기 위한 이름
	std::string name;
	// 선택. 없으면 모든 값이 기본값이다.
	GLTFPBRMetallicRoughness pbrMetallicRoughness;
};

struct GLTFPrimitive;

/** glTF meshes의 항목 하나. 한 번에 그리는 조각(primitive) 여러 개로 이루어진 모양이다. */
struct GLTFMesh
{
	// 필수. 조각 목록. 하나 이상이다.
	std::vector<GLTFPrimitive> primitives;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF mesh.primitives의 항목 하나. 같은 재질과 그리기 방식으로 한 번에 그리는 조각이다. */
struct GLTFPrimitive
{
	// 필수. 속성 이름(POSITION, NORMAL, TEXCOORD_0 등)에서 accessors 번호로의 대응
	std::map<std::string, std::size_t> attributes;
	// 선택. 인덱스가 들어 있는 accessors 번호. 없으면 정점을 순서대로 잇는다.
	std::optional<std::size_t> indices;
	// 선택. materials 번호. 없으면 기본 재질
	std::optional<std::size_t> material;
	// 선택. 정점을 잇는 방식. 0 점, 1 선, 2 선 고리, 3 선 띠, 4 삼각형 목록, 5 삼각형 띠, 6 삼각형 부채. 없으면 4
	int mode = 4;
};

/** glTF nodes의 항목 하나. 장면에 놓인 항목이며 메시를 번호로 가리킨다. */
struct GLTFNode
{
	// 선택. 자식 nodes 번호 목록. 자식의 변환은 이 노드 기준이다.
	std::vector<std::size_t> children;
	// 선택. 부모 기준 변환 행렬. 열 우선 순서의 숫자 16개다. 있으면 rotation, scale, translation 대신 쓴다.
	std::optional<std::array<float, 16>> matrix;
	// 선택. 이 노드가 그리는 meshes 번호
	std::optional<std::size_t> mesh;
	// 선택. 부모 기준 회전 쿼터니언 (x, y, z, w). 없으면 회전 없음
	std::array<float, 4> rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
	// 선택. 부모 기준 축별 배율. 없으면 1
	std::array<float, 3> scale = { 1.0f, 1.0f, 1.0f };
	// 선택. 부모 기준 이동(m). 없으면 0
	std::array<float, 3> translation = { 0.0f, 0.0f, 0.0f };
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF textures의 항목 하나. 이미지와 샘플러의 조합이다. 샘플러는 아직 읽지 않는다. */
struct GLTFTexture
{
	// 선택. images 번호. 없으면 확장이 이미지를 정한다.
	std::optional<std::size_t> source;
	// 선택. 사람이 보기 위한 이름
	std::string name;
};

/** glTF 파일 하나의 최상위 객체. 각 항목은 배열에 담기고, 다른 항목을 이 배열의 번호로 가리킨다. 배열이 없으면 비어 있다. */
struct GLTFDocument
{
	std::vector<GLTFAccessor> accessors;
	std::vector<GLTFBuffer> buffers;
	std::vector<GLTFBufferView> bufferViews;
	std::vector<GLTFImage> images;
	std::vector<GLTFMaterial> materials;
	std::vector<GLTFMesh> meshes;
	std::vector<GLTFNode> nodes;
	std::vector<GLTFTexture> textures;

	// 명세 항목이 아니다. buffers[i]의 uri에서 불러온 바이트이며 buffers와 같은 번호를 쓴다.
	std::vector<std::vector<std::byte>> bufferData;
};
