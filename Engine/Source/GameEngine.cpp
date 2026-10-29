#include "GameEngine.h"

#include "Components/CameraComponent.h"
#include "Core/Math/Matrix4.h"
#include "GameClient.h"
#include "GameInstance.h"
#include "GameInstanceFactory.h"
#include "Input/PlayerInput.h"
#include "Maps/GameMapsSettings.h"
#include "Renderer/Renderer.h"
#include "Renderer/SceneRenderData.h"
#include "World.h"

#pragma region Lifecycle

UGameEngine::UGameEngine() = default;

UGameEngine::~UGameEngine()
{
	PreExit();
}

#pragma endregion

#pragma region Execution

bool UGameEngine::Initialize(IRenderer& renderer)
{
	if (world || gameClient || gameInstance) { return false; }

	if (!UEngine::Initialize(renderer)) { return false; }

	if (!LoadMap(GetGameMapsSettings().gameDefaultMap))
	{
		PreExit();
		return false;
	}

	gameClient = MakeUnique<FGameClient>();
	gameInstance = CreateGameInstance();
	if (!gameInstance || !gameInstance->Initialize(*world, *gameClient, renderer))
	{
		PreExit();
		return false;
	}
	if (!world->CreateRenderStates())
	{
		PreExit();
		return false;
	}

	bInitialized = true;

	return true;
}

void UGameEngine::Start()
{
	if (!bInitialized) { return; }

	world->BeginPlay();
}

bool UGameEngine::Tick(float deltaSeconds, const FInputState& input, IRenderer& renderer, int width, int height)
{
	if (!bInitialized || !world->HasBegunPlay() || width <= 0 || height <= 0) { return false; }

	UPlayerInput& playerInput = gameClient->GetPlayerInput();
	if (!playerInput.EvaluateInputActions(gameClient->GetInputMappingContext(), input))
	{
		return false;
	}

	world->Tick(deltaSeconds, input, playerInput);
	gameClient->GetWidgetSubsystem().Tick(deltaSeconds);

	FRenderView renderView;
	if (const UCameraComponent* camera = world->GetActiveCamera())
	{
		const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		const FCameraViewInfo view = camera->GetCameraView();
		renderView.worldToView = view.CalculateViewMatrix();
		renderView.viewToClip = view.CalculateProjectionMatrix(aspectRatio);
		renderView.location = view.location;
		renderView.bOrthographic = view.projectionMode == ECameraProjectionMode::Orthographic;
	}

	if (!world->SendAllEndOfFrameUpdates())
	{
		return false;
	}

	if (!renderer.BeginFrame(width, height))
	{
		return false;
	}

	if (!world->CaptureScenes(renderer))
	{
		return false;
	}

	if (!world->GetScene().Draw(renderer, renderView))
	{
		return false;
	}

	if (!gameClient->GetWidgetSubsystem().Draw(renderer))
	{
		return false;
	}

	if (!renderer.EndFrame())
	{
		return false;
	}

	return true;
}

void UGameEngine::PreExit()
{
	bInitialized = false;
	if (world)
	{
		world->EndPlay(EEndPlayReason::Quit);
	}
	gameInstance.reset();
	gameClient.reset();
	UEngine::PreExit();
}

#pragma endregion
