#include "Launch/EngineMain.h"

#include "Camera/CameraTypes.h"
#include "Engine/Engine.h"
#include "Launch/LaunchEngineLoop.h"
#include "Logging/LogMacros.h"
#include "Math/Transform.h"
#include "Platform/GenericPlatform/GenericWindow.h"
#include "Renderer/MeshRenderData.h"
#include "Renderer/RendererInterface.h"
#include "SceneView.h"
#include "World.h"

#include <cmath>
#include <numbers>
#include <optional>

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogEngine);

	/**
	 * 월드 카메라와 화면 크기로 위치·뷰 회전·투영 데이터를 준비한다.
	 * 카메라 부재나 유효하지 않은 입력은 false이며 출력을 유지한다.
	 * @param sizeX 화면의 가로 픽셀 수. 0보다 커야 한다.
	 * @param sizeY 화면의 세로 픽셀 수. 0보다 커야 한다.
	 * @param outProjectionData 성공할 때만 준비된 값을 저장한다.
	 * 수평 FOV 유지·near 10cm·무한 far·Reversed-Z를 사용하고 LUF의 X 방향을 반영한다.
	 * 월드 카메라 조회의 유한 변환·단위 회전 전제는 유지한다.
	 * @return 데이터를 준비했으면 true.
	 */
	[[nodiscard]] bool GetProjectionData(const FWorld& world, uint32 sizeX, uint32 sizeY, FSceneViewProjectionData& outProjectionData);

	/** 월드 카메라와 화면 크기로 시점을 구성한다. 투영 준비 실패는 값 없음. 카메라 조회의 유한 변환·단위 회전 전제는 유지한다. */
	[[nodiscard]] std::optional<FSceneView> CalcSceneView(const FWorld& world, uint32 sizeX, uint32 sizeY);

	bool GetProjectionData(const FWorld& world, uint32 sizeX, uint32 sizeY, FSceneViewProjectionData& outProjectionData)
	{
		if (sizeX == 0 || sizeY == 0) { return false; }

		FMinimalViewInfo cameraView;
		if (!world.GetCameraView(cameraView)) { return false; }
		if (!std::isfinite(cameraView.location.x) || !std::isfinite(cameraView.location.y) || !std::isfinite(cameraView.location.z) || !std::isfinite(cameraView.rotation.pitch) || !std::isfinite(cameraView.rotation.yaw) || !std::isfinite(cameraView.rotation.roll) || !std::isfinite(cameraView.fov) || cameraView.fov <= 0.0f || cameraView.fov >= 180.0f)
		{
			return false;
		}

		const float aspectRatio = static_cast<float>(sizeX) / static_cast<float>(sizeY);
		const float halfFovRadians = cameraView.fov * (std::numbers::pi_v<float> / 360.0f);
		const float scaleX = 1.0f / std::tan(halfFovRadians);
		const float scaleY = aspectRatio * scaleX;
		if (!std::isfinite(scaleX) || !std::isfinite(scaleY) || scaleX <= 0.0f || scaleY <= 0.0f) { return false; }

		constexpr float nearClipPlane = 10.0f;
		FMatrix4 projection{};
		projection.m[0][0] = -scaleX;
		projection.m[1][1] = scaleY;
		projection.m[2][3] = 1.0f;
		projection.m[3][2] = nearClipPlane;

		FTransform cameraRotation;
		cameraRotation.rotation = cameraView.rotation.ToQuat();

		FSceneViewProjectionData projectionData;
		projectionData.viewOrigin = cameraView.location;
		projectionData.viewRotationMatrix = cameraRotation.ToMatrixNoScale().Inverse();
		projectionData.projectionMatrix = projection;
		outProjectionData = projectionData;

		return true;
	}

	std::optional<FSceneView> CalcSceneView(const FWorld& world, uint32 sizeX, uint32 sizeY)
	{
		FSceneViewInitOptions initOptions;
		if (!GetProjectionData(world, sizeX, sizeY, initOptions))
		{
			return std::nullopt;
		}

		return FSceneView(initOptions);
	}
}

int EngineMain(FGenericWindow& window, IRenderer& renderer)
{
	// TODO(seon): 실제 프레임 진행과 창 / 출력 책임을 목표 실행 구조에 맞춰 단계적으로 옮긴다.
	if (!window.Initialize())
	{
		return 1;
	}

	FEngineLoop engineLoop;
	engineLoop.Init();

	TArray<FMeshRenderData> renderData;

	// 객체가 존재해도 시작 최소화 상태에서는 그래픽스 초기화를 아직 하지 않았을 수 있다.
	bool bRendererInitialized = false;
	while (true)
	{
		window.PumpMessages();
		// 닫기 요청은 펌프 안에서 창 프로시저가 기록하므로 펌프 직후에 확인한다
		if (window.IsCloseRequested())
		{
			break;
		}

		engineLoop.Tick();

		// 유휴 대기 정책과 별개로 크기 0인 출력 자원을 만들거나 최소화 상태에 제출하지 않는다.
		const uint32 width = window.GetClientWidth();
		const uint32 height = window.GetClientHeight();
		if (window.IsMinimized() || width == 0 || height == 0) { continue; }

		if (!bRendererInitialized)
		{
			if (!renderer.Init(window.GetOSWindowHandle(), width, height))
			{
				// 실패한 구현이 부분 자원을 정리한다. 정상 종료용 Shutdown을 다시 호출하지 않는다.
				SE_LOG(LogEngine, Fatal, "Renderer initialization failed");
			}
			bRendererInitialized = true;
		}

		renderer.Resize(width, height);
		renderData.clear();
		const UEngine* engine = engineLoop.GetEngine();
		const FWorld* world = engine ? engine->GetWorld() : nullptr;
		std::optional<FSceneView> sceneView;
		if (world)
		{
			world->CollectMeshRenderData(renderData);
			sceneView = CalcSceneView(*world, width, height);
		}
		renderer.RenderFrame(renderData, sceneView ? &*sceneView : nullptr);
	}

	engineLoop.Exit();

	// 출력 자원을 정리하는 동안 OS 창을 유지한다. 출력 초기화 전에 닫혔다면 창만 정리한다.
	if (bRendererInitialized)
	{
		renderer.Shutdown();
	}
	window.Shutdown();
	return 0;
}
