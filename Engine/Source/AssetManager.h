#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/Templates/SharedPtr.h"
#include "Object.h"

#include <unordered_map>

class IRenderer;
class UFont;
class UStaticMesh;
class UTexture2D;

/** 같은 논리 경로의 텍스처·정적 메시·폰트 로딩을 재사용한다. 생성·사용·파괴는 Renderer와 같은 스레드에서 수행한다. */
class UAssetManager : public UObject
{
#pragma region Lifecycle

public:
	/** 초기화된 Renderer를 빌린다. 관리자와 반환 자산의 사용·정리를 끝낸 뒤 Renderer를 종료한다. */
	explicit UAssetManager(IRenderer& inRenderer);

private:
	IRenderer& renderer;

#pragma endregion

#pragma region Textures

public:
	/**
	 * UTF-8 논리 경로 /Engine/... 또는 /Game/...에서 이미지를 읽어 GPU 텍스처를 준비한다.
	 * .dds 확장자는 대소문자와 무관하게 원본 바이트를 Renderer의 DDS 생성 경로로 넘긴다. 나머지는 CPU 이미지 경로다.
	 * FPaths::ResolveContentPath의 경로 규칙을 따르며 키는 대소문자를 포함한 문자열의 정확한 일치다.
	 * 성공한 자산은 관리자 수명 동안 강하게 보관하고 같은 키에 같은 공유 객체를 반환한다. 변경은 모든 사용자에게 보인다.
	 * 경로·파일 읽기·GPU 생성 실패는 로그와 nullptr이며 실패 결과는 보관하지 않아 다음 요청에서 재시도한다.
	 * 입력은 저장할 때 복사한다. 반환 참조는 관리자 파괴 후에도 남을 수 있다. 메모리 할당 예외는 복구하지 않는다.
	 */
	[[nodiscard]] TSharedPtr<UTexture2D> LoadTexture2D(FStringView contentPath);

private:
	// 소비자가 참조를 놓아도 성공한 텍스처를 보관한다. 물리 파일의 별칭은 통합하지 않는다.
	std::unordered_map<FString, TSharedPtr<UTexture2D>> textures;

#pragma endregion

#pragma region StaticMeshes

public:
	/**
	 * 텍스트 X 파일에서 GPU 메시와 기본 재질을 준비한다. 경로·캐시·수명 규칙은 LoadTexture2D와 같다.
	 * 메시 하나, 재질 하나와 메시 배치 하나만 지원하며 배치 노드의 누적 변환은 정확히 항등이어야 한다.
	 * 기본 색은 로더의 정점 색을 유지하고 색상 텍스처만 연결한다. 나머지 재질 설정은 UMaterial 기본값을 쓴다.
	 * 텍스처는 모델 기준 경로를 논리 Content 경로로 바꿔 요청한다. Content 밖의 참조는 거부한다.
	 * 미지원 구조·파일·텍스처·GPU 생성 실패는 로그와 nullptr다. 완성된 메시만 캐시하며 이미 로드한 텍스처는 남을 수 있다.
	 * Actor 생성·World 배치·노드 변환 굽기는 하지 않는다. 반환 자산의 사용과 해제는 Renderer 종료 전에 끝낸다.
	 */
	[[nodiscard]] TSharedPtr<UStaticMesh> LoadStaticMesh(FStringView contentPath);

private:
	// 기본 재질과 텍스처는 메시 자산이 공유한다. 성공한 메시를 관리자 수명 동안 강하게 보관한다.
	std::unordered_map<FString, TSharedPtr<UStaticMesh>> staticMeshes;

#pragma endregion

#pragma region Fonts

public:
	/**
	 * UTF-8 논리 경로의 컴파일된 폰트를 읽어 GPU 자원을 준비한다. 경로·키·공유 수명 규칙은 LoadTexture2D와 같다.
	 * 현재 Renderer가 지원하는 Premultiplied 아틀라스의 .spritefont 입력을 사용한다. 표현을 자동 판별하거나 변환하지 않는다.
	 * 경로·파일 읽기·생성의 반환값 실패는 로그와 nullptr이며 성공한 자산만 보관한다. 실패 뒤 재요청할 수 있다.
	 * 외부 라이브러리의 형식·자원 생성 예외와 할당 예외는 잡지 않고 전파한다.
	 * 파일 바이트는 생성 호출 동안만 유지한다. 반환 폰트의 사용과 해제는 Renderer 종료 전에 끝낸다.
	 */
	[[nodiscard]] TSharedPtr<UFont> LoadFont(FStringView contentPath);

private:
	// 마지막 소비자가 놓아도 관리자는 성공한 폰트를 보관한다. 같은 키의 요청에는 같은 객체를 반환한다.
	std::unordered_map<FString, TSharedPtr<UFont>> fonts;

#pragma endregion
};
