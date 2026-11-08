#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

struct StaticMeshData;

/**
 * 불러온 CPU 자산을 경로별로 보관하고 재사용한다. Engine이 값으로 소유하며 Get으로 어디서든 빌려 쓴다.
 * 한 번에 하나만 존재해야 하고 메인 스레드에서 사용한다.
 */
class AssetManager
{
	// 생성과 소멸
public:
	/** 이 매니저를 Get의 대상으로 등록한다. 이미 등록된 매니저가 있으면 안 된다. */
	AssetManager();

	/** Get의 등록을 해제한다. */
	~AssetManager();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	// 전역 접근
public:
	/** 등록된 매니저를 빌려준다. 매니저가 살아 있는 동안(엔진 생성부터 소멸까지)만 호출한다. */
	static AssetManager& Get();

private:
	// 비소유. 생성자에서 등록하고 소멸자에서 비운다. 등록된 매니저가 없으면 nullptr이다.
	static AssetManager* instance;

	// 정적 메시
public:
	/**
	 * 메인 스레드에서 저장소 루트 기준 UTF-8 경로로 정적 메시를 요청한다.
	 * 파일명과 폴더명의 대소문자는 실제 자산 경로에 맞춘다.
	 * 로드한 메시를 이 매니저의 수명 동안 보관하고, 같은 경로 요청에 재사용한다.
	 * 경로를 구하거나 메시를 읽는 데 실패하면 nullptr를 반환한다.
	 */
	std::shared_ptr<const StaticMeshData> LoadStaticMesh(std::string_view relativePath);

private:
	std::unordered_map<std::filesystem::path, std::shared_ptr<const StaticMeshData>> staticMeshes;
};
