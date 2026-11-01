#include "SampleGameInstance.h"

#include "AssetManager.h"
#include "Components/ActorComponent.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Logging/Log.h"
#include "Core/Templates/UniquePtr.h"
#include "Engine.h"
#include "GameClient.h"
#include "GameInstanceFactory.h"
#include "Image/Image.h"
#include "Object/Actor.h"
#include "Renderer/Font.h"
#include "Renderer/Material.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture2D.h"
#include "SampleFPSWidget.h"
#include "UI/ImageWidget.h"
#include "UI/UserWidget.h"
#include "UI/Widget.h"
#include "World/World.h"

#include <array>
#include <utility>

namespace
{

	// 초기 구성 중 호출한다. World가 컴포넌트를 소유하고, 컴포넌트가 재질과 텍스처의 수명을 유지한다.
	bool AddTextureBlendSamples(UWorld& world, UAssetManager& assets, IRenderer& renderer,
		const TSharedPtr<UTexture2D>& checkerTexture)
	{
		TSharedPtr<UStaticMesh> mesh = assets.LoadStaticMesh("/Game/Meshes/SampleSquare.x");
		if (!mesh) { return false; }

		// 위쪽 행은 빨강·노랑, 아래쪽 행은 파랑·초록인 불투명 RGBA8 패턴이다.
		const FImage image{
			.sizeX = 2,
			.sizeY = 2,
			.rawData = {255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 255, 0, 255, 0, 255}};
		TSharedPtr<UTexture2D> colorTexture = MakeShared<UTexture2D>();
		if (!renderer.CreateTexture2D(image, *colorTexture)) { return false; }

		constexpr std::array<float, 3> BlendFactors{0.0f, 0.5f, 1.0f};
		for (size_t index = 0; index < BlendFactors.size(); ++index)
		{
			TSharedPtr<UMaterial> material = MakeShared<UMaterial>();
			material->SetBaseColorTexture(checkerTexture);
			material->SetBlendTexture(colorTexture);
			material->SetBlendFactor(BlendFactors[index]);

			AActor& actor = world.SpawnActor<AActor>();
			UTransformComponent* transform = actor.GetComponent<UTransformComponent>();
			// 기존 위아래 두 줄 사이에서 왼쪽부터 0·0.5·1을 비교한다.
			transform->SetPosition({-2.5f + 2.5f * static_cast<float>(index), 0.0f, 0.0f});
			// 회전하는 기존 사각형의 최대 반경과도 겹치지 않는 크기다.
			transform->SetScale({0.4f, 0.4f, 0.4f});

			TUniquePtr<UStaticMeshComponent> component = MakeUnique<UStaticMeshComponent>(actor);
			if (!component->SetStaticMesh(mesh)) { return false; }
			// 컴포넌트별 재질 지정으로 공유 메시 자산의 기존 기본 재질을 유지한다.
			component->SetMaterial(0, material);
			TUniquePtr<UActorComponent> actorComponent = std::move(component);
			if (!actor.AddComponent("TextureBlendSample", actorComponent)) { return false; }
		}

		return true;
	}

} // namespace

TUniquePtr<UGameInstance> CreateGameInstance()
{
	return MakeUnique<USampleGameInstance>();
}

bool USampleGameInstance::Initialize(UWorld& world, FGameClient& gameClient, IRenderer& renderer)
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
	if (!AddTextureBlendSamples(world, assets, renderer, texture)) { return false; }

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
