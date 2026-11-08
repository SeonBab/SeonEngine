#pragma once

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

/** glTF 파일 하나의 최상위 객체. 각 항목은 배열에 담기고, 다른 항목을 이 배열의 번호로 가리킨다. 배열이 없으면 비어 있다. */
struct GLTFDocument
{
	std::vector<GLTFAccessor> accessors;
	std::vector<GLTFBuffer> buffers;
	std::vector<GLTFBufferView> bufferViews;
	std::vector<GLTFMesh> meshes;

	// 명세 항목이 아니다. buffers[i]의 uri에서 불러온 바이트이며 buffers와 같은 번호를 쓴다.
	std::vector<std::vector<std::byte>> bufferData;
};
