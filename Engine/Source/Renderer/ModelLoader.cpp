#include "Renderer/ModelLoader.h"

#include "Core/Containers/SeonString.h"
#include "Core/Logging/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Renderer/ModelData.h"

#include <cctype>
#include <charconv>
#include <cmath>
#include <concepts>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>

namespace
{
	bool Fail(FStringView message)
	{
		WriteLog("Model", ELogLevel::Error, "{}", message);

		return false;
	}

	// 지원하는 텍스트 .x 헤더를 검사하고 본문만 읽는다. 실패하면 기존 출력을 유지한다.
	[[maybe_unused]] bool ReadTextXFile(const std::filesystem::path& path, FString& outText)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			return Fail("Opening the text X file failed.");
		}

		char header[16]{};
		if (!file.read(header, sizeof(header)))
		{
			return Fail("Reading the X file header failed.");
		}

		if (FStringView(header, sizeof(header)) != "xof 0303txt 0032")
		{
			return Fail("Only text X files with 32-bit floats and version 03.03 are supported.");
		}

		// 헤더 뒤에 남은 내용을 4,096 바이트씩 읽어서 붙인다.
		FString text;
		char buffer[4096];
		while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
		{
			text.append(buffer, static_cast<size_t>(file.gcount()));
		}

		if (file.bad() || !file.eof())
		{
			return Fail("Reading the X file body failed.");
		}

		outText = std::move(text);

		return true;
	}

	// 원본 문자열은 유지하고 읽기 위치를 첫 번째 공백 아닌 문자까지 옮긴다.
	void SkipWhitespace(FStringView text, size_t& textOffset)
	{
		while (textOffset < text.size() && std::isspace(static_cast<unsigned char>(text[textOffset])))
		{
			++textOffset;
		}
	}

	// 템플릿 선언 하나를 닫는 중괄호 다음까지 건너뛴다. 실패하면 기존 읽기 위치를 유지한다.
	// 현재 드워프 파일처럼 선언 내부에 주석과 문자열이 없는 입력을 가정한다.
	bool SkipTemplateDeclaration(FStringView text, size_t& textOffset)
	{
		size_t cursor = textOffset;
		constexpr FStringView Keyword = "template";

		if (cursor > text.size() || text.substr(cursor, Keyword.size()) != Keyword)
		{
			return Fail("A template declaration was expected.");
		}

		cursor += Keyword.size();
		if (cursor == text.size() || !std::isspace(static_cast<unsigned char>(text[cursor])))
		{
			return Fail("Whitespace after the template keyword was expected.");
		}

		SkipWhitespace(text, cursor);

		// .x 템플릿 이름 규칙에서 선언 이름은 영문자 또는 밑줄로 시작한다.
		if (cursor == text.size() || (!std::isalpha(static_cast<unsigned char>(text[cursor])) && text[cursor] != '_'))
		{
			return Fail("A template name was expected.");
		}

		while (cursor < text.size() && (std::isalnum(static_cast<unsigned char>(text[cursor])) || text[cursor] == '_'))
		{
			++cursor;
		}

		SkipWhitespace(text, cursor);
		if (cursor == text.size() || text[cursor] != '{')
		{
			return Fail("An opening brace after the template name was expected.");
		}

		++cursor;
		size_t depth = 1;

		while (cursor < text.size())
		{
			const char character = text[cursor];
			++cursor;

			if (character == '{')
			{
				++depth;
			}
			else if (character == '}')
			{
				--depth;
				if (depth == 0)
				{
					textOffset = cursor;

					return true;
				}
			}
		}

		return Fail("The template declaration was not closed.");
	}

	// 본문 앞의 템플릿 선언들을 건너뛰고 첫 실제 데이터의 위치로 이동한다.
	bool SkipTemplateDeclarations(FStringView text, size_t& textOffset)
	{
		size_t cursor = textOffset;
		constexpr FStringView Keyword = "template";

		while (true)
		{
			SkipWhitespace(text, cursor);

			if (text.substr(cursor, Keyword.size()) != Keyword)
			{
				break;
			}

			const size_t keywordEnd = cursor + Keyword.size();
			if (keywordEnd < text.size() && !std::isspace(static_cast<unsigned char>(text[keywordEnd])))
			{
				break;
			}

			if (!SkipTemplateDeclaration(text, cursor))
			{
				return false;
			}
		}

		textOffset = cursor;

		return true;
	}

	// 앞의 공백을 건너뛰고 이름 구간을 반환한다. 실패하면 기존 위치와 출력을 유지한다.
	bool ParseIdentifier(FStringView text, size_t& textOffset, FStringView& outIdentifier)
	{
		size_t cursor = textOffset;
		SkipWhitespace(text, cursor);

		if (cursor >= text.size() || (!std::isalpha(static_cast<unsigned char>(text[cursor])) && text[cursor] != '_'))
		{
			return Fail("An identifier was expected.");
		}

		const size_t start = cursor;
		++cursor;

		while (cursor < text.size())
		{
			const unsigned char character = static_cast<unsigned char>(text[cursor]);

			if (!std::isalnum(character) && character != '_' && character != '-')
			{
				break;
			}

			++cursor;
		}

		outIdentifier = text.substr(start, cursor - start);
		textOffset = cursor;

		return true;
	}

	// 앞의 공백을 건너뛰고 예상한 기호를 소비한다. 실패하면 기존 읽기 위치를 유지한다.
	bool ExpectCharacter(FStringView text, size_t& textOffset, char expected)
	{
		size_t cursor = textOffset;
		SkipWhitespace(text, cursor);

		if (cursor >= text.size() || text[cursor] != expected)
		{
			return Fail("The expected character was not found.");
		}

		textOffset = cursor + 1;

		return true;
	}

	// 앞의 공백을 건너뛰고 float 또는 uint32 숫자를 읽는다.
	// 실패하면 기존 읽기 위치와 출력 값을 유지한다.
	template<typename T>
		requires(std::same_as<T, float> || std::same_as<T, uint32>)
	bool ParseNumber(FStringView text, size_t& textOffset, T& outValue)
	{
		size_t cursor = textOffset;
		SkipWhitespace(text, cursor);

		if (cursor >= text.size())
		{
			return Fail("A number was expected.");
		}

		// 현재 읽기 위치부터 본문 끝까지를 숫자 변환 범위로 지정한다.
		const char* begin = text.data() + cursor;
		const char* end = text.data() + text.size();
		T value{};
		const std::from_chars_result result = std::from_chars(begin, end, value);
		if (result.ec != std::errc{})
		{
			return Fail("Reading a number failed.");
		}

		// 실수 타입에서만 NaN과 무한대를 거부한다.
		if constexpr (std::same_as<T, float>)
		{
			if (!std::isfinite(value))
			{
				return Fail("A finite floating-point value was expected.");
			}
		}

		outValue = value;
		textOffset = static_cast<size_t>(result.ptr - text.data());

		return true;
	}

	// 큰따옴표 안의 문자열 구간을 반환한다. 실패하면 기존 위치와 출력을 유지한다.
	bool ParseQuotedString(FStringView text, size_t& textOffset, FStringView& outString)
	{
		size_t cursor = textOffset;
		if (!ExpectCharacter(text, cursor, '"'))
		{
			return false;
		}

		const size_t start = cursor;
		while (cursor < text.size() && text[cursor] != '"')
		{
			++cursor;
		}

		if (cursor == text.size())
		{
			return Fail("The quoted string was not closed.");
		}

		outString = text.substr(start, cursor - start);
		textOffset = cursor + 1;

		return true;
	}

	// 재질 하나의 이름, 수치, 텍스처 경로를 읽는다.
	// 실패하면 기존 읽기 위치와 출력 재질을 유지한다.
	bool ParseMaterial(FStringView text, size_t& textOffset, const std::filesystem::path& modelDirectory, FModelMaterialData& outMaterial)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		FStringView name;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "Material")
		{
			return Fail("A Material block was expected.");
		}

		if (!ParseIdentifier(text, cursor, name) || !ExpectCharacter(text, cursor, '{'))
		{
			return false;
		}

		FModelMaterialData material;
		material.name = FString(name);

		// 기본색의 R, G, B, A를 순서대로 읽음
		for (float& component : material.baseColor)
		{
			if (!ParseNumber(text, cursor, component) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}
		}

		if (!ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		// Power를 읽어 재질에 저장한다
		if (!ParseNumber(text, cursor, material.power) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		// 첫 번쨰 RGB 묶음 -> diffuse
		for (float& component : material.diffuse)
		{
			if (!ParseNumber(text, cursor, component) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}
		}

		if (!ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		// 두 번쨰 RGB 묶음 -> ambient
		for (float& component : material.ambient)
		{
			if (!ParseNumber(text, cursor, component) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}
		}

		if (!ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		// TextureFilename 블록이 있으면 이미지 파일명을 읽는다.
		FStringView textureName;
		SkipWhitespace(text, cursor);

		if (cursor < text.size() && text[cursor] != '}')
		{
			if (!ParseIdentifier(text, cursor, keyword))
			{
				return false;
			}

			if (keyword != "TextureFilename")
			{
				return Fail("A TextureFilename block was expected.");
			}

			if (!ExpectCharacter(text, cursor, '{') || !ParseQuotedString(text, cursor, textureName) || !ExpectCharacter(text, cursor, ';') || !ExpectCharacter(text, cursor, '}'))
			{
				return false;
			}
		}

		// Material 블록의 닫는 중괄호를 확인한다.
		if (!ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		// 텍스처 파일명을 모델 폴더 기준의 절대경로로 변환한다.
		if (!textureName.empty())
		{
			std::error_code error;
			material.baseColorTexturePath = std::filesystem::absolute(modelDirectory / std::filesystem::path(textureName), error);

			if (error)
			{
				return Fail("Resolving the material texture path failed.");
			}
		}

		// 모든 처리가 성공한 뒤 출력과 익기 위치를 확정한다
		outMaterial = std::move(material);
		textOffset = cursor;

		return true;
	}

	// 연속된 Material 블록을 배열로 읽고 다음 선언 앞에서 멈춘다
	// 실패하면 기존 읽기 위치와 출력 배열을 유지한다
	bool ParseMaterials(FStringView text, size_t& textOffset, const std::filesystem::path& modelDirectory, TArray<FModelMaterialData>& outMaterials)
	{
		size_t cursor = textOffset;
		TArray<FModelMaterialData> materials;

		while (true)
		{
			SkipWhitespace(text, cursor);
			if (cursor >= text.size())
			{
				break;
			}

			// 다음 선언을 미리 확인한다. 실제 읽기 위치는 옮기지 않기 위해 별도의 변수를 사용한다
			size_t lookahead = cursor;
			FStringView keyword;
			if (!ParseIdentifier(text, lookahead, keyword))
			{
				return false;
			}

			if (keyword != "Material")
			{
				break;
			}

			FModelMaterialData material;
			if (!ParseMaterial(text, cursor, modelDirectory, material))
			{
				return false;
			}

			// 읽기를 마친 재질을 목록에 추가한다
			materials.push_back(std::move(material));
		}

		outMaterials = std::move(materials);
		textOffset = cursor;

		return true;
	}

	// Frame 키워드, 이름, 여는 중괄호를 읽는다.
	// 실패하면 기존 읽기 위치와 출력 이름을 유지한다.
	bool ParseFrameHeader(FStringView text, size_t& textOffset, FStringView& outName)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		FStringView name;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "Frame")
		{
			return Fail("A Frame block was expected.");
		}

		if (!ParseIdentifier(text, cursor, name) || !ExpectCharacter(text, cursor, '{'))
		{
			return false;
		}

		outName = name;
		textOffset = cursor;

		return true;
	}

	// FrameTransformMatrix 블록의 16개 성분을 행 우선 순서로 읽는다.
	// 실패하면 기존 읽기 위치와 출력 행렬을 유지한다.
	bool ParseFrameTransformMatrix(FStringView text, size_t& textOffset, FMatrix4& outMatrix)
	{
		size_t cursor = textOffset;
		FStringView keyword;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "FrameTransformMatrix")
		{
			return Fail("A FrameTransformMatrix block was expected.");
		}

		if (!ExpectCharacter(text, cursor, '{'))
		{
			return false;
		}

		FMatrix4 matrix;

		for (size_t row = 0; row < 4; ++row)
		{
			for (size_t column = 0; column < 4; ++column)
			{
				if (!ParseNumber(text, cursor, matrix.m[row][column]))
				{
					return false;
				}

				// 마지막 성분 뒤에는 쉼표 대신 세미콜론이 온다
				const char separator = (row == 3 && column == 3) ? ';' : ',';

				if (!ExpectCharacter(text, cursor, separator))
				{
					return false;
				}
			}
		}

		// 마지막 성분 뒤의 두 번째 세미콜론과 블록 끝을 확인한다
		if (!ExpectCharacter(text, cursor, ';') || !ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		outMatrix = matrix;
		textOffset = cursor;

		return true;
	}

	// Mesh 키워드, 이름, 여는 중괄호, 정점 개수를 읽는다
	// 실패하면 기존 읽기 위치와 출력 값을 유지한다
	bool ParseMeshHeader(FStringView text, size_t& textOffset, FStringView& outName, uint32& outVertexCount)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		FStringView name;
		uint32 vertexCount = 0;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "Mesh")
		{
			return Fail("A Mesh block was expected.");
		}

		if (!ParseIdentifier(text, cursor, name) || !ExpectCharacter(text, cursor, '{'))
		{
			return false;
		}

		if (!ParseNumber(text, cursor, vertexCount) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		outName = name;
		outVertexCount = vertexCount;
		textOffset = cursor;

		return true;
	}

	// 지정된 개수의 XYZ 좌표와 배열 끝 구분자를 읽는다
	// 실패하면 기존 읽기 위치와 출력 배열을 유지한다
	bool ParseVector3Array(FStringView text, size_t& textOffset, uint32 vectorCount, TArray<FVector3>& outVectors)
	{
		size_t cursor = textOffset;
		TArray<FVector3> vectors;

		for (uint32 index = 0; index < vectorCount; ++index)
		{
			FVector3 vectorValue;

			if (!ParseNumber(text, cursor, vectorValue.x) || !ExpectCharacter(text, cursor, ';') ||
				!ParseNumber(text, cursor, vectorValue.y) || !ExpectCharacter(text, cursor, ';') ||
				!ParseNumber(text, cursor, vectorValue.z) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}

			// 마지막 벡터를 제외하고 벡터 사이의 쉼표를 확인한다
			if (index + 1 < vectorCount && !ExpectCharacter(text, cursor, ','))
			{
				return false;
			}

			vectors.push_back(vectorValue);
		}

		// 벡터 배열을 끝내는 추가 세미콜론을 확인한다
		if (!ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		outVectors = std::move(vectors);
		textOffset = cursor;

		return true;
	}

	// 삼각형 면의 인덱스를 읽고 참조 배열 범위를 확인한다
	// 실패하면 기존 읽기 위치와 출력 배열을 유지한다
	bool ParseTriangleIndices(FStringView text, size_t& textOffset, uint32 elementCount, TArray<uint32>& outIndices)
	{
		size_t cursor = textOffset;
		TArray<uint32> indices;
		uint32 faceCount = 0;

		if (!ParseNumber(text, cursor, faceCount) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		for (uint32 faceIndex = 0; faceIndex < faceCount; ++faceIndex)
		{
			uint32 faceVertexCount = 0;
			if (!ParseNumber(text, cursor, faceVertexCount) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}

			if (faceVertexCount != 3)
			{
				return Fail("A triangular mesh face was expected.");
			}

			for (uint32 corner = 0; corner < 3; ++corner)
			{
				uint32 elementIndex = 0;
				if (!ParseNumber(text, cursor, elementIndex))
				{
					return false;
				}

				if (elementIndex >= elementCount)
				{
					return Fail("A triangle index was out of range.");
				}

				// 면 내부 첫 두 인덱스 뒤에는 쉼표, 마지막에는 세미콜론
				const char separator = (corner == 2) ? ';' : ',';
				if (!ExpectCharacter(text, cursor, separator))
				{
					return false;
				}

				indices.push_back(elementIndex);
			}

			// 마지막 면을 제외하고 면 사이의 쉼표를 확인한다
			if (faceIndex + 1 < faceCount &&
				!ExpectCharacter(text, cursor, ','))
			{
				return false;
			}
		}

		// 면 배열을 끝내는 추가 세미콜론을 확인한다
		if (!ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		outIndices = std::move(indices);
		textOffset = cursor;

		return true;
	}
	// MeshNormals의 법선 배열과 면별 법선 인덱스를 읽는다.
	// 실패하면 기존 읽기 위치와 출력 배열들을 유지한다.
	bool ParseMeshNormals(FStringView text, size_t& textOffset, TArray<FVector3>& outNormals, TArray<uint32>& outNormalIndices)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		uint32 normalCount = 0;
		TArray<FVector3> normals;
		TArray<uint32> normalIndices;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "MeshNormals")
		{
			return Fail("A MeshNormals block was expected.");
		}

		if (!ExpectCharacter(text, cursor, '{') || !ParseNumber(text, cursor, normalCount) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		if (!ParseVector3Array(text, cursor, normalCount, normals) || !ParseTriangleIndices(text, cursor, normalCount, normalIndices) || !ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		outNormals = std::move(normals);
		outNormalIndices = std::move(normalIndices);
		textOffset = cursor;

		return true;
	}
	// 단일 재질 슬롯의 면별 번호와 재질 참조 이름을 읽는다.
	// 실패하면 기존 읽기 위치와 출력 이름을 유지한다.
	bool ParseMeshMaterialList(FStringView text, size_t& textOffset, uint32 expectedFaceCount, FStringView& outMaterialName)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		FStringView materialName;
		uint32 materialCount = 0;
		uint32 faceCount = 0;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "MeshMaterialList")
		{
			return Fail("A MeshMaterialList block was expected.");
		}

		if (!ExpectCharacter(text, cursor, '{') || !ParseNumber(text, cursor, materialCount) || !ExpectCharacter(text, cursor, ';') || !ParseNumber(text, cursor, faceCount) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		if (materialCount != 1)
		{
			return Fail("Exactly one material slot per mesh was expected.");
		}

		if (faceCount != expectedFaceCount)
		{
			return Fail("The material face count did not match the mesh.");
		}

		for (uint32 faceIndex = 0; faceIndex < faceCount; ++faceIndex)
		{
			uint32 materialSlot = 0;
			if (!ParseNumber(text, cursor, materialSlot))
			{
				return false;
			}

			if (materialSlot != 0)
			{
				return Fail("Material slot zero was expected.");
			}

			if (faceIndex + 1 < faceCount && !ExpectCharacter(text, cursor, ','))
			{
				return false;
			}
		}

		if (!ExpectCharacter(text, cursor, ';') || !ExpectCharacter(text, cursor, '{') || !ParseIdentifier(text, cursor, materialName) || !ExpectCharacter(text, cursor, '}') || !ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		outMaterialName = materialName;
		textOffset = cursor;

		return true;
	}
	// UV 배열을 읽고 개수가 메시의 정점 개수와 같은지 확인한다.
	// 실패하면 기존 읽기 위치와 출력 배열을 유지한다.
	bool ParseMeshTextureCoords(FStringView text, size_t& textOffset, uint32 expectedVertexCount, TArray<FVector2>& outTextureCoords)
	{
		size_t cursor = textOffset;
		FStringView keyword;
		uint32 textureCoordCount = 0;
		TArray<FVector2> textureCoords;

		if (!ParseIdentifier(text, cursor, keyword))
		{
			return false;
		}

		if (keyword != "MeshTextureCoords")
		{
			return Fail("A MeshTextureCoords block was expected.");
		}

		// 블록에 이름이 있으면 읽고 넘어간다.
		SkipWhitespace(text, cursor);
		if (cursor < text.size() && text[cursor] != '{')
		{
			FStringView name;
			if (!ParseIdentifier(text, cursor, name))
			{
				return false;
			}
		}

		if (!ExpectCharacter(text, cursor, '{') || !ParseNumber(text, cursor, textureCoordCount) || !ExpectCharacter(text, cursor, ';'))
		{
			return false;
		}

		if (textureCoordCount != expectedVertexCount)
		{
			return Fail("The texture coordinate count did not match the mesh.");
		}

		for (uint32 index = 0; index < textureCoordCount; ++index)
		{
			FVector2 textureCoord;
			if (!ParseNumber(text, cursor, textureCoord.x) || !ExpectCharacter(text, cursor, ';') || !ParseNumber(text, cursor, textureCoord.y) || !ExpectCharacter(text, cursor, ';'))
			{
				return false;
			}

			if (index + 1 < textureCoordCount && !ExpectCharacter(text, cursor, ','))
			{
				return false;
			}

			textureCoords.push_back(textureCoord);
		}

		if (!ExpectCharacter(text, cursor, ';') || !ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		outTextureCoords = std::move(textureCoords);
		textOffset = cursor;

		return true;
	}
	// 메시의 위치·법선·UV·인덱스·재질 연결을 구성한다.
	// 실패하면 기존 읽기 위치와 출력 메시를 유지한다.
	bool ParseMesh(FStringView text, size_t& textOffset, const TArray<FModelMaterialData>& materials, FStaticMeshData& outMesh)
	{
		size_t cursor = textOffset;
		FStringView meshName;
		FStringView materialName;
		uint32 vertexCount = 0;
		TArray<FVector3> vertexPositions;
		TArray<FVector3> normals;
		TArray<FVector2> textureCoords;
		TArray<uint32> indices;
		TArray<uint32> normalIndices;

		if (!ParseMeshHeader(text, cursor, meshName, vertexCount) ||
			!ParseVector3Array(text, cursor, vertexCount, vertexPositions) ||
			!ParseTriangleIndices(text, cursor, vertexCount, indices) ||
			!ParseMeshNormals(text, cursor, normals, normalIndices))
		{
			return false;
		}

		// 원본 번호를 그대로 사용하려면 위치와 법선의 대응이 같아야 한다.
		if (normals.size() != vertexPositions.size() || normalIndices != indices)
		{
			return Fail("Matching position and normal arrays and indices were expected.");
		}

		const size_t faceCount = indices.size() / 3;
		if (faceCount > std::numeric_limits<uint32>::max())
		{
			return Fail("The mesh face count exceeded the supported integer range.");
		}

		if (!ParseMeshMaterialList(text, cursor, static_cast<uint32>(faceCount), materialName) ||
			!ParseMeshTextureCoords(text, cursor, vertexCount, textureCoords) ||
			!ExpectCharacter(text, cursor, '}'))
		{
			return false;
		}

		FStaticMeshData mesh;

		// 메시 내부 슬롯이 아닌 모델 전체 재질 목록의 번호를 연결한다.
		for (size_t index = 0; index < materials.size(); ++index)
		{
			if (materials[index].name != materialName)
			{
				continue;
			}

			if (mesh.materialIndex != -1)
			{
				return Fail("The referenced material name was not unique.");
			}

			if (index > static_cast<size_t>(std::numeric_limits<int32>::max()))
			{
				return Fail("The material index exceeded the supported integer range.");
			}

			mesh.materialIndex = static_cast<int32>(index);
		}

		if (mesh.materialIndex == -1)
		{
			return Fail("The referenced mesh material was not found.");
		}

		const FModelMaterialData& material = materials[static_cast<size_t>(mesh.materialIndex)];
		mesh.vertices.resize(vertexPositions.size());

		for (size_t index = 0; index < mesh.vertices.size(); ++index)
		{
			FVertex& vertex = mesh.vertices[index];
			vertex.position = vertexPositions[index];
			vertex.normal = normals[index];
			vertex.uv = textureCoords[index];

			for (size_t component = 0; component < 4; ++component)
			{
				vertex.color[component] = material.baseColor[component];
			}
		}

		mesh.indices = std::move(indices);
		outMesh = std::move(mesh);
		textOffset = cursor;

		return true;
	}

	// Frame과 하위 Frame·Mesh를 읽어 모델에 추가한다.
	// SceneRoot도 노드로 보관하며, 변환행렬은 부모 기준 값 그대로 저장한다.
	// 실패하면 이번 호출에서 추가한 노드·메시와 읽기 위치를 되돌린다.
	bool ParseFrame(FStringView text, size_t& textOffset, int32 parentIndex, FModelData& model)
	{
		// -1은 부모 없는 루트다. 나머지는 이미 존재하는 노드 번호여야 한다.
		if (parentIndex < -1)
		{
			return Fail("The parent node index was out of range.");
		}

		if (parentIndex >= 0)
		{
			const size_t parentNodeIndex = static_cast<size_t>(parentIndex);
			if (parentNodeIndex >= model.nodes.size())
			{
				return Fail("The parent node index was out of range.");
			}
		}

		// 실패 시 이 길이로 줄여 현재 Frame과 하위 호출의 추가분을 제거한다.
		const size_t nodeCountBefore = model.nodes.size();
		const size_t meshCountBefore = model.meshes.size();
		if (nodeCountBefore > static_cast<size_t>(std::numeric_limits<int32>::max()))
		{
			return Fail("The node index exceeded the supported integer range.");
		}

		size_t cursor = textOffset;
		FModelNodeData node;
		node.parentIndex = parentIndex;

		// Frame 이름을 읽고 원본 본문 수명과 독립적인 문자열로 복사한다.
		FStringView frameName;
		if (!ParseFrameHeader(text, cursor, frameName))
		{
			return false;
		}

		node.name = FString(frameName);

		// 현재 입력 범위에서는 Frame 시작 직후 변환행렬을 읽는다.
		if (!ParseFrameTransformMatrix(text, cursor, node.localTransform))
		{
			return false;
		}

		const auto rollback = [&]()
		{
			model.nodes.resize(nodeCountBefore);
			model.meshes.resize(meshCountBefore);

			return false;
		};

		// 배열 접근용 번호만 기억한다. 재귀 호출 너머로 노드 참조를 보관하지 않는다.
		const size_t currentNodeIndex = nodeCountBefore;
		model.nodes.push_back(std::move(node));

		while (true)
		{
			SkipWhitespace(text, cursor);
			if (cursor >= text.size())
			{
				Fail("The Frame block was not closed.");
				return rollback();
			}

			// 닫는 중괄호는 식별자가 아니므로 선언 이름을 읽기 전에 처리한다.
			// 현재 Frame 전체가 끝났을 때만 호출자의 읽기 위치를 확정한다.
			if (text[cursor] == '}')
			{
				++cursor;
				textOffset = cursor;

				return true;
			}

			// 처리할 함수를 고르기 위해 다음 단어만 확인한다.
			// cursor는 유지해 선택한 함수가 Frame 또는 Mesh 키워드부터 읽게 한다.
			size_t lookahead = cursor;
			FStringView keyword;
			if (!ParseIdentifier(text, lookahead, keyword))
			{
				return rollback();
			}

			if (keyword == "Frame")
			{
				// 현재 노드가 자식 Frame의 부모가 된다. 번호 범위는 위에서 확인했다.
				const int32 childParentIndex = static_cast<int32>(currentNodeIndex);
				if (!ParseFrame(text, cursor, childParentIndex, model))
				{
					return rollback();
				}
			}
			else if (keyword == "Mesh")
			{
				if (model.meshes.size() > static_cast<size_t>(std::numeric_limits<uint32>::max()))
				{
					Fail("The mesh index exceeded the supported integer range.");
					return rollback();
				}

				FStaticMeshData mesh;
				if (!ParseMesh(text, cursor, model.materials, mesh))
				{
					return rollback();
				}

				const uint32 meshIndex = static_cast<uint32>(model.meshes.size());
				model.meshes.push_back(std::move(mesh));

				// 파싱이 끝난 뒤 노드를 다시 찾고 모델 전체 메시 번호를 연결한다.
				// 이 참조는 다음 재귀 호출까지 보관하지 않고 여기서만 사용한다.
				FModelNodeData& currentNode = model.nodes[currentNodeIndex];
				currentNode.meshIndices.push_back(meshIndex);
			}
			else
			{
				Fail("A child Frame, Mesh, or closing brace was expected.");
				return rollback();
			}
		}
	}

	// 모델의 원본 파일명과 배열에서 계산한 개수를 로그로 출력한다
	void LogModelInfo(const FModelData& model)
	{
		size_t vertexCount = 0;
		size_t triangleCount = 0;

		for (const FStaticMeshData& mesh : model.meshes)
		{
			vertexCount += mesh.vertices.size();
			triangleCount += mesh.indices.size() / 3;
		}

		// 경로의 파일명을 로그에서 사용하는 UTF-8 문자열로 가져온다
		const std::u8string utf8FileName = model.sourcePath.filename().u8string();
		const FString fileName(utf8FileName.begin(), utf8FileName.end());

		WriteLog("Model", ELogLevel::Info, "Model: {}", fileName.empty() ? "<unnamed>" : fileName);
		WriteLog("Model", ELogLevel::Info, "Nodes: {}, Meshes: {}, Materials: {}", model.nodes.size(), model.meshes.size(), model.materials.size());
		WriteLog("Model", ELogLevel::Info, "Vertices: {}, Triangles: {}", vertexCount, triangleCount);
	}
}

bool FModelLoader::LoadTextXModel(const std::filesystem::path& path, FModelData& outModel)
{
	std::error_code error;
	const std::filesystem::path absolutePath = std::filesystem::absolute(path, error);

	if (error)
	{
		return Fail("Resolving the model file path failed.");
	}

	FString text;
	if (!ReadTextXFile(absolutePath, text))
	{
		return false;
	}

	FModelData model;
	model.sourcePath = absolutePath;

	const std::filesystem::path modelDirectory = absolutePath.parent_path();

	size_t textOffset = 0;
	if (!SkipTemplateDeclarations(text, textOffset) ||
		!ParseMaterials(text, textOffset, modelDirectory, model.materials) ||
		!ParseFrame(text, textOffset, -1, model))
	{
		return false;
	}

	// 루트 Frame 뒤의 공백을 건너뛰고 본문을 끝까지 읽었는지 확인한다
	SkipWhitespace(text, textOffset);
	if (textOffset != text.size())
	{
		return Fail("Unexpected data remained after the root Frame.");
	}

	outModel = std::move(model);
	LogModelInfo(outModel);

	return true;
}
