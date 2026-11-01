#pragma once

struct FRenderView;
class FScene;
class FSceneViewport;
class IRenderer;
class UWorld;

////////////////////////////////////////////////////////////////////////////////
// 뷰포트의 시점 구성과 장면 제출을 위한 공통 계약이다.
// 출력 자원의 소유, 출력 전환과 프레임 시작 및 종료는 담당하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FViewportClient
{
#pragma region Lifecycle

public:
	/** 기반 포인터를 통해 파괴해도 파생 클래스의 소멸자를 호출한다. */
	virtual ~FViewportClient() = default;

#pragma endregion

#pragma region Drawing

public:
	/**
	 * 양수 출력 크기의 뷰포트와 월드를 호출 동안 빌려 그릴 시점 값을 구성한다.
	 * 카메라 선택은 파생 클래스가 담당한다. GPU 자원 생성이나 실제 그리기는 하지 않는다.
	 */
	[[nodiscard]] virtual FRenderView BuildRenderView(const FSceneViewport& viewport, const UWorld& world) const = 0;

	/**
	 * 준비된 시점으로 장면을 제출한다. 인자는 호출 동안만 빌리고 시점을 다시 계산하지 않는다.
	 * 호출자는 활성 프레임과 올바른 출력을 준비한다. 실패하면 false이며 GPU 완료를 뜻하지 않는다.
	 * 출력 전환, clear와 Present는 수행하지 않는다.
	 */
	[[nodiscard]] virtual bool Draw(FScene& scene, IRenderer& renderer, const FRenderView& renderView) const;

#pragma endregion
};
