#pragma once

#include "Math/Matrix4.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 시점 구성에 사용할 위치와 뷰 회전·투영 행렬을 값으로 보관한다.
// 기본값은 위치 원점과 두 항등행렬이며 실제 투영 계산은 후속 단계다.
////////////////////////////////////////////////////////////////////////////////
struct FSceneViewProjectionData
{
	/** 두 행렬을 항등으로 준비한다. */
	FSceneViewProjectionData();

	/** 카메라의 월드 위치. 단위는 cm이며 기본값은 원점이다. */
	FVector3 viewOrigin{};

	/** 월드 방향을 카메라 기준 방향으로 변환한다. 이동과 크기를 포함하지 않는다. */
	FMatrix4 viewRotationMatrix;

	/** 카메라 기준 좌표를 클립 좌표로 변환한다. */
	FMatrix4 projectionMatrix;
};

////////////////////////////////////////////////////////////////////////////////
// 시점 생성에 사용할 입력이다. 위치·뷰 회전·투영 데이터를 상속한다.
// FSceneView 생성자를 통해 시점 행렬 구성에 전달한다.
////////////////////////////////////////////////////////////////////////////////
struct FSceneViewInitOptions : public FSceneViewProjectionData
{
};

////////////////////////////////////////////////////////////////////////////////
// 렌더링할 시점의 변환 행렬을 관리한다.
// 기본값은 뷰·투영 항등행렬이며 입력의 위치·뷰 회전·투영을 구성해 보관한다.
////////////////////////////////////////////////////////////////////////////////
struct FViewMatrices
{
	/** 행렬 구성에 사용할 카메라 위치와 뷰 회전·투영 행렬을 값으로 보관한다. */
	struct FMinimalInitializer
	{
		/** 뷰 회전·투영 행렬을 항등으로 준비한다. */
		FMinimalInitializer();

		/** 월드 방향을 카메라 기준 방향으로 변환한다. 이동과 크기를 포함하지 않는다. */
		FMatrix4 viewRotationMatrix;

		/** 카메라 기준 좌표를 클립 좌표로 변환한다. 기본값은 항등이다. */
		FMatrix4 projectionMatrix;

		/** 카메라의 월드 위치. 단위는 cm이며 기본값은 원점이다. */
		FVector3 viewOrigin{};
	};

	/** 기본 뷰·투영 행렬을 항등행렬로 준비한다. */
	FViewMatrices();

	/** 기본 행렬을 준비한 뒤 Init으로 입력 위치·뷰 회전을 결합하고 투영을 보관한다. 입력은 유한한 값과 이동·크기가 없는 뷰 회전 행렬이다. */
	explicit FViewMatrices(const FMinimalInitializer& initializer);

	/** 시점 입력의 위치·뷰 회전·투영을 최소 입력에 옮겨 Init으로 구성한다. 입력은 유한한 값과 이동·크기가 없는 뷰 회전 행렬이다. */
	explicit FViewMatrices(const FSceneViewInitOptions& initOptions);

	/** 보관한 월드→카메라 변환 행렬을 읽기 전용으로 반환한다. 참조는 원본 객체의 수명 동안 유효하다. */
	[[nodiscard]] const FMatrix4& GetWorldToView() const;

	/** 보관한 카메라→클립 변환 행렬을 읽기 전용으로 반환한다. 참조는 원본 객체의 수명 동안 유효하다. */
	[[nodiscard]] const FMatrix4& GetViewToClip() const;

private:
	/** 유한한 입력으로 반대 방향 이동 후 뷰 회전을 적용하는 행렬을 구성하고 투영을 그대로 보관한다. 뷰 회전은 이동·크기를 포함하지 않는다. */
	void Init(const FMinimalInitializer& initializer);

	/** 월드 좌표를 카메라 기준 좌표로 변환한다. */
	FMatrix4 worldToView;

	/** 카메라 기준 좌표를 클립 좌표로 변환한다. */
	FMatrix4 viewToClip;
};

////////////////////////////////////////////////////////////////////////////////
// 화면을 그릴 시점의 정보를 값으로 보관한다.
// 준비된 입력으로 시점 행렬을 구성한다.
////////////////////////////////////////////////////////////////////////////////
class FSceneView
{
public:
	/** 준비된 렌더링 입력으로 값 멤버의 시점 행렬을 구성한다. */
	explicit FSceneView(const FSceneViewInitOptions& initOptions);

	/** 이 시점의 변환 행렬을 값으로 보관한다. */
	FViewMatrices viewMatrices;
};
