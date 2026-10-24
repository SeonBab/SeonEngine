#include "GameEngine.h"

#include "Components/CameraComponent.h"
#include "Core/Math/Matrix4.h"
#include "GameClient.h"
#include "GameInstance.h"
#include "GameInstanceFactory.h"
#include "Input/PlayerInput.h"
#include "Renderer/Renderer.h"
#include "World.h"

#pragma region Lifecycle

UGameEngine::UGameEngine() = default;

UGameEngine::~UGameEngine() = default;

#pragma endregion

#pragma region Execution

bool UGameEngine::Initialize(FRenderer& renderer)
{
	if (world || gameClient || gameInstance) { return false; }

	if (!UEngine::Initialize(renderer)) { return false; }

	gameClient = MakeUnique<FGameClient>();
	gameInstance = CreateGameInstance();
	if (!gameInstance || !gameInstance->Initialize(*world, *gameClient, renderer))
	{
		return false;
	}
	if (!world->CreateRenderStates())
	{
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

bool UGameEngine::Tick(float deltaSeconds, const FInputState& input, FRenderer& renderer, int width, int height)
{
	if (!bInitialized || !world->HasBegunPlay() || width <= 0 || height <= 0) { return false; }

	UPlayerInput& playerInput = gameClient->GetPlayerInput();
	if (!playerInput.EvaluateInputActions(gameClient->GetInputMappingContext(), input))
	{
		return false;
	}

	world->Tick(deltaSeconds, input, playerInput);

	FMatrix4 worldToClip;
	worldToClip.SetIdentity();
	if (const UCameraComponent* camera = world->GetActiveCamera())
	{
		const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		const FCameraViewInfo view = camera->GetCameraView();
		worldToClip = view.CalculateWorldToClipMatrix(aspectRatio);
	}

	if (!renderer.SetWorldToClip(worldToClip))
	{
		return false;
	}

	if (!world->SendAllEndOfFrameUpdates())
	{
		return false;
	}

	if (!renderer.BeginFrame(width, height))
	{
		return false;
	}

	if (!world->GetScene().Draw(renderer))
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
	world.reset();
}

#pragma endregion
