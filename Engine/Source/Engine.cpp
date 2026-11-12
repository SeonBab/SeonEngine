#include "Engine.h"

#include "CameraViewInfo.h"
#include "Component/CameraComponent.h"
#include "Core/FrameTimer.h"
#include "Core/Log.h"
#include "Input/InputDispatcher.h"

#include "D3D12Renderer.h"
#include "DataHub.h"
#include "DataHub_Interface.h"

#include <memory>
#include <utility>

Engine::Engine() = default;

Engine::~Engine()
{
	Shutdown();
}

bool Engine::Initialize()
{
	if (!windowsApplication.Initialize()) { return false; }

	canRun = true;
	return true;
}

bool Engine::Shutdown()
{
	canRun = false;
	if (renderer)
	{
		// GPU 작업을 마친 뒤 객체 데이터 ID 맵과 DataHub를 비우고, 렌더러 객체를 창보다 먼저 해제한다.
		renderer->Finalize();
		world.GetRenderScene().ClearObjectDataIds();
		Hakko::DataHub::Finalize();
		renderer.reset();
	}

	return windowsApplication.Shutdown();
}

World& Engine::GetWorld()
{
	return world;
}

int Engine::Run()
{
	if (!canRun) { return 1; }
	canRun = false;

	if (!InitializeRendering())
	{
		Shutdown();
		return 1;
	}

	Hakko::DataHub::SetLayerBackground(RESERVED_TEX_NONE_DIFFUSE_ID);
	Hakko::DataHub::SetLayerBackgroundColor(0.3f, 0.05f, 0.1f, 1.0f);

	Log::Write(LogLevel::Info, "Engine", "Run loop started");

	// 초기화와 World 구성 시간을 첫 측정에서 빼려고 루프 직전에 만든다.
	FrameTimer frameTimer;
	int exitCode = 0;

	while (windowsApplication.ProcessMessages(exitCode))
	{
		Tick(frameTimer.Tick());

		// 최소화 중에도 월드와 시간은 갱신하고, 렌더링 작업만 생략한다.
		if (windowsApplication.IsMinimized()) { continue; }

		int clientWidth = 0;
		int clientHeight = 0;
		if (!windowsApplication.GetClientSize(clientWidth, clientHeight))
		{
			Log::Write(LogLevel::Error, "Engine", "Failed to get client size for rendering");
			Shutdown();
			return 1;
		}

		if (clientWidth <= 0 || clientHeight <= 0) { continue; }

		if (clientWidth != renderWidth || clientHeight != renderHeight)
		{
			renderer->OnResize(clientWidth, clientHeight);
			renderWidth = clientWidth;
			renderHeight = clientHeight;
		}

		const CameraComponent* camera = world.GetActiveCamera();
		if (camera == nullptr) { continue; }

		CameraViewInfo cameraViewInfo;
		if (!camera->GetCameraViewInfo(cameraViewInfo)) { continue; }

		const float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
		if (!world.GetRenderScene().PrepareCamera(cameraViewInfo, aspectRatio)) { continue; }

		world.GetRenderScene().PrepareDirectionalLight();

		if (!world.GetRenderScene().PrepareStaticMeshes())
		{
			Shutdown();
			return 1;
		}

		if (!world.GetRenderScene().PrepareDrawRequests())
		{
			Shutdown();
			return 1;
		}

		renderer->Sketch();
		renderer->Render();
	}

	if (!Shutdown()) { return 1; }

	return exitCode;
}

bool Engine::InitializeRendering()
{
	int clientWidth = 0;
	int clientHeight = 0;
	if (!windowsApplication.GetClientSize(clientWidth, clientHeight) || clientWidth <= 0 || clientHeight <= 0)
	{
		Log::Write(LogLevel::Error, "Engine", "Invalid client size for rendering");
		return false;
	}

	std::unique_ptr<Hakko::D3D12Renderer> newRenderer = std::make_unique<Hakko::D3D12Renderer>(Hakko::RenderLimits{});
	// 렌더러 초기화에서 예약 리소스와 그리기 설정을 읽으므로 DataHub를 먼저 준비한다.
	if (!Hakko::DataHub::Init())
	{
		Log::Write(LogLevel::Error, "Engine", "DataHub initialization failed");
		
		Hakko::DataHub::Finalize();
		
		return false;
	}

	if (!newRenderer->Init(windowsApplication.GetWindowHandle(), clientWidth, clientHeight))
	{
		Log::Write(LogLevel::Error, "Engine", "D3D12 renderer initialization failed");

		// Finalize는 생성된 GPU 펜스를 전제로 하므로 Init 실패 경로에서 호출하지 않는다.
		Hakko::DataHub::Finalize();
		
		return false;
	}

	renderer = std::move(newRenderer);
	renderWidth = clientWidth;
	renderHeight = clientHeight;

	Log::Write(LogLevel::Info, "Engine", "D3D12 renderer initialized");

	return true;
}

void Engine::Tick(float deltaTime)
{
	// 입력 바인딩을 먼저 실행한 뒤 일반 갱신을 한다.
	world.GetInputDispatcher().Dispatch(windowsApplication.GetInputState(), deltaTime);
	world.Tick(deltaTime);
}
