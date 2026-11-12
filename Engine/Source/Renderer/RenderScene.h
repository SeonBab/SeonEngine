#pragma once

#include "DataHub_Types.h"

#include <unordered_map>
#include <vector>

class DirectionalLightComponent;
class StaticMeshComponent;
class Texture;
struct CameraViewInfo;
struct Material;

/**
 * 한 World의 정적 메시와 방향광 컴포넌트 등록 목록을 관리한다. 메인 스레드에서 사용하며 World가 소유한다.
 * StaticMeshComponent와 DirectionalLightComponent는 부착 시 여기에 등록하고 소멸 전에 스스로 해제한다.
 */
class RenderScene
{
	// 생성과 소멸
public:
	RenderScene() = default;

	RenderScene(const RenderScene&) = delete;
	RenderScene& operator=(const RenderScene&) = delete;
	RenderScene(RenderScene&&) = delete;
	RenderScene& operator=(RenderScene&&) = delete;

	// 정적 메시
public:
	/**
	 * 렌더링 대상으로 추가한다. nullptr이거나 이미 추가한 대상은 무시한다.
	 * 대상은 소멸 전에 RemoveStaticMeshComponent로 제거되어야 한다. 할당 실패 예외는 전파한다.
	 */
	void AddStaticMeshComponent(StaticMeshComponent* component);

	/** 렌더링 대상에서 제거하고 등록한 객체 데이터 ID를 반납한다. 추가하지 않은 대상은 무시한다. */
	void RemoveStaticMeshComponent(StaticMeshComponent* component);

private:
	// 비소유이며 추가한 순서를 유지한다.
	std::vector<StaticMeshComponent*> staticMeshComponents;

	// 방향광
public:
	/**
	 * nullptr와 중복 등록은 무시한다. 할당 실패 예외는 전파한다.
	 * 대상은 소멸 전에 RemoveDirectionalLightComponent로 제거되어야 한다.
	 */
	void AddDirectionalLightComponent(DirectionalLightComponent* component);

	/** 등록되지 않은 대상은 무시한다. */
	void RemoveDirectionalLightComponent(DirectionalLightComponent* component);

private:
	// 비소유이며 등록 순서를 유지한다.
	std::vector<DirectionalLightComponent*> directionalLightComponents;

	// Hakko 메시 등록
public:
	/**
	 * DataHub 초기화 후 호출한다. 등록된 컴포넌트의 메시를 Hakko에 등록하고 이미 등록한 메시는 재사용한다.
	 * 빈 메시 참조는 건너뛰며 에셋 경로가 비어 있거나 등록에 실패하면 false를 반환한다.
	 * 같은 에셋 경로는 같은 메시로 취급하며 이름 조회로 등록 여부를 판단한다.
	 * 실패 전에 등록한 메시는 유지한다. 경로 변환과 할당 실패 예외는 전파한다.
	 */
	bool PrepareStaticMeshes();

	// Hakko 텍스처 등록
private:
	/**
	 * DataHub 초기화 후 메인 스레드의 렌더링 준비 중 CPU 텍스처를 Hakko에 등록한다.
	 * 정규화한 경로와 형식으로 기존 등록을 재사용한다. 같은 경로와 형식의 픽셀은 등록 후 변경하지 않는다.
	 * 새 등록에는 픽셀 복사본을 전달하고 업로드를 요청한다. 같은 프레임에 렌더러가 요청을 처리해야 한다.
	 * 성공하면 ID를 출력하고 true를 반환하며, GPU 업로드 완료를 뜻하지는 않는다.
	 * 잘못된 입력이나 등록 실패는 false를 반환하고 outTextureID는 0으로 유지한다.
	 * 경로 변환과 할당 실패 예외는 전파한다.
	 */
	bool PrepareTexture(const Texture& texture, Hakko::DataHub::TexID& outTextureID);

	/**
	 * DataHub 초기화 후 메인 스레드의 렌더링 준비 중 재질의 텍스처 조합을 준비한다.
	 * 재질 또는 색상 텍스처가 미지정이면 오류용 조합 0을 출력하고 true를 반환한다.
	 * 색상 텍스처의 정규화한 경로와 형식으로 조합을 재사용하며, 나머지 슬롯은 기본 노멀, 흰색 ORM과 검정 발광으로 고정한다.
	 * 새 조합은 같은 프레임에 렌더러가 업로드 요청을 처리해야 한다. 성공 시 ID를 출력하며 GPU 준비 완료를 뜻하지는 않는다.
	 * 텍스처 준비나 조합 등록에 실패하면 false를 반환하고 outTexturePropID는 0으로 유지한다.
	 * PrepareTexture의 예외와 같은 프레임 업로드 계약을 따른다. 실패 전에 등록한 텍스처는 유지한다.
	 */
	bool PrepareTextureProp(const Material* material, Hakko::DataHub::TexPropID& outTexturePropID);

	// Hakko 카메라 준비
public:
	/**
	 * DataHub 초기화 후 Sketch 전에 호출해 기본 레이어의 카메라 행렬과 월드 위치를 갱신한다.
	 * 행렬은 현재 Hakko 셰이더의 열 단위 저장에 맞춰 전치해 전달한다.
	 * aspectRatio는 그릴 영역의 가로를 세로로 나눈 값이다. 투영 계산에 실패하면 기존 값을 유지하고 false를 반환한다.
	 */
	bool PrepareCamera(const CameraViewInfo& cameraViewInfo, float aspectRatio);

	// Hakko 방향광 준비
public:
	/**
	 * DataHub 초기화 후 Sketch 전에 기본 레이어의 방향광을 갱신한다.
	 * 처음으로 방향 조회에 성공한 활성 방향광을 사용하며, 없으면 빛의 색상을 0으로 설정한다.
	 */
	void PrepareDirectionalLight();

	// Hakko 그리기 요청 준비
public:
	/**
	 * DataHub 초기화와 메시 준비 성공 후 Sketch 전에 호출한다. 기존 기본 레이어의 불투명 요청 목록을 비우고 다시 만든다.
	 * 메시와 TransformComponent가 있는 대상의 객체 데이터를 갱신하고 첫 서브메시의 그리기 요청을 추가한다.
	 * 메시 ID는 에셋 경로로 매번 조회하며 재질의 텍스처 조합을 준비해 요청에 연결한다.
	 * 미등록 메시, 텍스처 또는 조합 준비 실패나 객체 데이터 등록 실패 시 false를 반환한다.
	 * 요청 생성부터 렌더러의 요청 소비까지 메시, 텍스처와 조합 ID를 재배치하거나 DataHub를 종료하면 안 된다.
	 * 새 텍스처와 조합의 업로드 요청은 같은 프레임에 렌더러가 처리해야 한다.
	 * 경로 변환과 할당 실패 예외는 전파한다.
	 * 재질 또는 색상 텍스처 미지정은 오류용 조합 0을 사용한다.
	 */
	bool PrepareDrawRequests();

	/** GPU 작업 완료 후 DataHub 종료에 맞춰 ID 맵만 비운다. 실제 객체 데이터는 DataHub 종료 시 정리한다. */
	void ClearObjectDataIds();

private:
	// 컴포넌트별 객체 데이터 ID다. 키는 비소유이며 컴포넌트 제거 시 항목도 제거해야 한다.
	std::unordered_map<const StaticMeshComponent*, Hakko::DataHub::DataBoxID> objectDataIds;
};
