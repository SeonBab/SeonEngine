#include "SampleGameInstance.h"

#include "Actor.h"
#include "AssetManager.h"
#include "Components/ActorComponent.h"
#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Logging/Log.h"
#include "Core/Templates/UniquePtr.h"
#include "Engine.h"
#include "GameClient.h"
#include "GameInstanceFactory.h"
#include "Renderer/Font.h"
#include "Renderer/Texture2D.h"
#include "SampleFPSWidget.h"
#include "UI/ImageWidget.h"
#include "UI/UserWidget.h"
#include "UI/Widget.h"
#include "World.h"

#include <utility>

TUniquePtr<UGameInstance> CreateGameInstance()
{
	return MakeUnique<USampleGameInstance>();
}

bool USampleGameInstance::Initialize(UWorld& world, FGameClient& gameClient, IRenderer&)
{
	UEngine* engine = UEngine::Get();
	if (!engine)
	{
		WriteLog("SampleGame", ELogLevel::Error, "Preparing sample assets requires an active engine.");
		return false;
	}
	UAssetManager& assets = engine->GetAssetManager();

	// 게임 실행에서만 사용할 카메라를 준비한다.
	{
		AActor& cameraActor = world.SpawnActor<AActor>();
		cameraActor.GetComponent<UTransformComponent>()->SetPosition({0.0f, 0.0f, -6.0f});

		TUniquePtr<UCameraComponent> camera = MakeUnique<UCameraComponent>(cameraActor);
		camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		camera->SetOrthographicWidth(9.0f);

		TUniquePtr<UActorComponent> cameraComponent = std::move(camera);
		if (!cameraActor.AddComponent("Camera", cameraComponent))
		{
			return false;
		}

		if (!world.SetActiveCamera(&cameraActor)) { return false; }
	}

	// 로컬 클라이언트의 UI를 준비한다.
	TSharedPtr<UTexture2D> texture = assets.LoadTexture2D("/Engine/Textures/Checker.png");
	if (!texture) { return false; }

	TUniquePtr<UImageWidget> imageWidget = MakeUnique<UImageWidget>();
	if (!imageWidget->SetTexture(texture)) { return false; }
	imageWidget->SetPosition({20.0f, 20.0f});
	imageWidget->SetSize({128.0f, 128.0f});

	TUniquePtr<UUserWidget> widget = MakeUnique<UUserWidget>();
	TUniquePtr<UWidget> root = std::move(imageWidget);
	if (!widget->SetRootWidget(root)) { return false; }

	if (!gameClient.GetWidgetSubsystem().AddWidget(widget)) { return false; }

	TSharedPtr<UFont> font = assets.LoadFont("/Engine/Fonts/DebugMono.spritefont");
	if (!font) { return false; }

	TUniquePtr<USampleFPSWidget> fpsWidget = MakeUnique<USampleFPSWidget>();
	if (!fpsWidget->Initialize(font)) { return false; }

	// Client가 FPS 위젯과 루트를 소유하고 Subsystem이 프레임마다 갱신한다.
	TUniquePtr<UUserWidget> userWidget = std::move(fpsWidget);

	return gameClient.GetWidgetSubsystem().AddWidget(userWidget);
}
