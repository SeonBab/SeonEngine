#include "Actor.h"
#include "Components/ActorComponent.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Misc/Paths.h"
#include "Core/Templates/UniquePtr.h"
#include "GameClient.h"
#include "Image/Image.h"
#include "Image/ImageUtils.h"
#include "Launch/GameEntry.h"
#include "Renderer/Renderer.h"
#include "Renderer/StaticMeshData.h"
#include "Renderer/Texture2D.h"
#include "SampleActor.h"
#include "UI/ImageWidget.h"
#include "UI/UserWidget.h"
#include "UI/Widget.h"
#include "World.h"

#include <filesystem>
#include <optional>
#include <utility>

bool ConfigureGameWorld(UWorld& world)
{
	const std::optional<std::filesystem::path> baseDir = FPaths::BaseDir();
	if (!baseDir) { return false; }

	FImage checkerImage;
	if (!FImageUtils::LoadImageFile(*baseDir / L"Content" / L"Textures" / L"Checker.png", checkerImage)) { return false; }

	AActor& actor = world.SpawnActor<ASampleActor>();
	const FStaticMeshData meshData =
		{
			{{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {}, {0.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {}, {0.0f, 0.0f}},
				{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {}, {1.0f, 0.0f}},
				{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {}, {1.0f, 1.0f}}},
			{0, 1, 2, 0, 2, 3}};
	TUniquePtr<UStaticMeshComponent> mesh = MakeUnique<UStaticMeshComponent>(actor);
	mesh->SetMeshData(meshData);
	mesh->SetBaseColorImage(checkerImage);

	// 게임은 CPU 데이터와 소유 관계만 구성하고 렌더 상태 준비는 엔진에 맡긴다.
	TUniquePtr<UActorComponent> component = std::move(mesh);

	if (!actor.AddComponent(component))
	{
		return false;
	}

	AActor& cameraActor = world.SpawnActor<AActor>();
	cameraActor.GetComponent<UTransformComponent>()->SetPosition({0.0f, 0.0f, -4.0f});

	TUniquePtr<UCameraComponent> camera = MakeUnique<UCameraComponent>(cameraActor);
	camera->SetProjectionMode(ECameraProjectionMode::Orthographic);

	TUniquePtr<UActorComponent> cameraComponent = std::move(camera);
	if (!cameraActor.AddComponent(cameraComponent))
	{
		return false;
	}

	return world.SetActiveCamera(&cameraActor);
}

bool ConfigureGameClient(FGameClient& gameClient, FRenderer& renderer)
{
	const std::optional<std::filesystem::path> baseDir = FPaths::BaseDir();
	if (!baseDir) { return false; }

	FImage image;
	if (!FImageUtils::LoadImageFile(*baseDir / L"Content" / L"Textures" / L"Checker.png", image)) { return false; }

	TUniquePtr<UTexture2D> texture = MakeUnique<UTexture2D>();
	if (!renderer.CreateTexture2D(image, *texture)) { return false; }

	TUniquePtr<UImageWidget> imageWidget = MakeUnique<UImageWidget>();
	if (!imageWidget->SetTexture(texture)) { return false; }
	imageWidget->SetPosition({20.0f, 20.0f});
	imageWidget->SetSize({128.0f, 128.0f});

	TUniquePtr<UUserWidget> widget = MakeUnique<UUserWidget>();
	TUniquePtr<UWidget> root = std::move(imageWidget);
	if (!widget->SetRootWidget(root)) { return false; }

	return gameClient.GetWidgetSubsystem().AddWidget(widget);
}
