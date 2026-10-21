#include "Actor.h"
#include "Components/ActorComponent.h"
#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Math/Rotator.h"
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

	const FStaticMeshData meshData =
		{
			{{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
				{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
				{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}},
			{0, 1, 2, 0, 2, 3}};

	const auto configureSquare = [&meshData, &checkerImage](AActor& actor, FVector3 position, FRotator rotation)
	{
		UTransformComponent* transform = actor.GetComponent<UTransformComponent>();
		transform->SetPosition(position);
		transform->SetRotation(rotation);
		transform->SetScale({1.4f, 1.4f, 1.4f});

		TUniquePtr<UStaticMeshComponent> mesh = MakeUnique<UStaticMeshComponent>(actor);
		mesh->SetMeshData(meshData);
		mesh->SetBaseColorImage(checkerImage);
		TUniquePtr<UActorComponent> component = std::move(mesh);

		return actor.AddComponent(component);
	};

	struct FSampleSquarePose
	{
		FVector3 position;
		FRotator rotation;
	};

	// 같은 메시와 색으로 면의 기울기에 따른 광량만 비교한다.
	const FSampleSquarePose fixedSquares[] =
		{
			{{-2.5f, 1.2f, 0.0f}, {35.0f, -35.0f, 0.0f}},
			{{0.0f, 1.2f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{2.5f, 1.2f, 0.0f}, {0.0f, 60.0f, 0.0f}},
			{{-2.5f, -1.2f, 0.0f}, {60.0f, 0.0f, 0.0f}},
			{{0.0f, -1.2f, 0.0f}, {-60.0f, 0.0f, 0.0f}}};

	for (const FSampleSquarePose& pose : fixedSquares)
	{
		if (!configureSquare(world.SpawnActor<AActor>(), pose.position, pose.rotation)) { return false; }
	}
	if (!configureSquare(world.SpawnActor<ASampleActor>(), {2.5f, -1.2f, 0.0f}, {0.0f, 0.0f, 15.0f})) { return false; }

	AActor& lightActor = world.SpawnActor<AActor>();
	lightActor.GetComponent<UTransformComponent>()->SetRotation({35.0f, -35.0f, 0.0f});
	TUniquePtr<UDirectionalLightComponent> light = MakeUnique<UDirectionalLightComponent>(lightActor);
	light->SetLightColor({1.0f, 1.0f, 1.0f});
	light->SetIntensity(1.0f);
	TUniquePtr<UActorComponent> lightComponent = std::move(light);
	if (!lightActor.AddComponent(lightComponent))
	{
		return false;
	}

	AActor& cameraActor = world.SpawnActor<AActor>();
	cameraActor.GetComponent<UTransformComponent>()->SetPosition({0.0f, 0.0f, -6.0f});

	TUniquePtr<UCameraComponent> camera = MakeUnique<UCameraComponent>(cameraActor);
	camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	camera->SetOrthographicWidth(9.0f);

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
