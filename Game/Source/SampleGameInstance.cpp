#include "SampleGameInstance.h"

#include "Actor.h"
#include "Components/ActorComponent.h"
#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Math/Rotator.h"
#include "Core/Misc/Paths.h"
#include "Core/Templates/UniquePtr.h"
#include "GameClient.h"
#include "GameInstanceFactory.h"
#include "Image/Image.h"
#include "Image/ImageUtils.h"
#include "Renderer/Material.h"
#include "Renderer/Renderer.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/StaticMeshData.h"
#include "Renderer/StaticMeshRenderData.h"
#include "Renderer/Texture2D.h"
#include "SampleActor.h"
#include "UI/ImageWidget.h"
#include "UI/UserWidget.h"
#include "UI/Widget.h"
#include "World.h"

#include <filesystem>
#include <optional>
#include <utility>

TUniquePtr<UGameInstance> CreateGameInstance()
{
	return MakeUnique<USampleGameInstance>();
}

bool USampleGameInstance::Initialize(UWorld& world, FGameClient& gameClient, FRenderer& renderer)
{
	// 월드 콘텐츠를 준비한다. 초기화 실패의 정리는 엔진이 맡는다.
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

		// 형상/재질을 한 번 준비한다. 컴포넌트가 자산을 공유하고 자산이 GPU 형상을 단독 소유한다.
		auto renderData = MakeUnique<FStaticMeshRenderData>();
		if (!renderer.CreateMesh(meshData, *renderData)) { return false; }

		auto meshTexture = MakeShared<UTexture2D>();
		if (!renderer.CreateTexture2D(checkerImage, *meshTexture)) { return false; }

		auto meshMaterial = MakeShared<UMaterial>();
		meshMaterial->SetBaseColorTexture(meshTexture);
		auto staticMesh = MakeShared<UStaticMesh>();
		staticMesh->SetRenderData(std::move(renderData));
		staticMesh->SetStaticMaterials({meshMaterial});

		const auto configureSquare = [&staticMesh](AActor& actor, FVector3 position, FRotator rotation)
		{
			UTransformComponent* transform = actor.GetComponent<UTransformComponent>();
			transform->SetPosition(position);
			transform->SetRotation(rotation);
			transform->SetScale({1.4f, 1.4f, 1.4f});

			TUniquePtr<UStaticMeshComponent> mesh = MakeUnique<UStaticMeshComponent>(actor);
			(void)mesh->SetStaticMesh(staticMesh);
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

		// 방향광과 독립적인 균일 주변광이다. 세기 0.15는 비교용 예시이며 물리 단위가 아니다.
		AActor& skyLightActor = world.SpawnActor<AActor>();
		auto skyLight = MakeUnique<USkyLightComponent>(skyLightActor);
		skyLight->SetIntensity(0.15f);
		TUniquePtr<UActorComponent> skyLightComponent = std::move(skyLight);
		if (!skyLightActor.AddComponent(skyLightComponent)) { return false; }

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

		if (!world.SetActiveCamera(&cameraActor)) { return false; }
	}

	// 로컬 클라이언트의 UI를 준비한다.
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
