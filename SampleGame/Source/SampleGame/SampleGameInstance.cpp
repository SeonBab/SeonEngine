#include "SampleGameInstance.h"

#include "Actor.h"
#include "Engine/StaticMesh.h"
#include "Logging/LogMacros.h"
#include "Renderer/StaticMeshVertex.h"
#include "StaticMeshComponent.h"
#include "World.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogSampleGameInstance);
}

USampleGameInstance::USampleGameInstance(UEngine& inEngine)
	: UGameInstance(inEngine)
{
}

void USampleGameInstance::StartGameInstance()
{
	UGameInstance::StartGameInstance();

	FWorld* world = GetWorld();
	if (!world)
	{
		SE_LOG(LogSampleGameInstance, Fatal, "Starting SampleGame requires an initialized world");
	}

	FActor& actor = world->SpawnActor<FActor>();
	TUniquePtr<FStaticMeshComponent> mesh = MakeUnique<FStaticMeshComponent>(actor);
	TSharedPtr<UStaticMesh> meshAsset = MakeShared<UStaticMesh>();
	if (!meshAsset->SetMeshData(
			{
				FStaticMeshVertex{FVector3{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
				FStaticMeshVertex{FVector3{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
				FStaticMeshVertex{FVector3{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
			},
			EPrimitiveType::PT_TriangleList))
	{
		SE_LOG(LogSampleGameInstance, Fatal, "Setting the sample mesh asset data failed");
	}
	// 처음 지정하며 반환은 변경 여부다. 컴포넌트가 지역 자산의 공유 수명을 유지한다.
	mesh->SetStaticMesh(meshAsset);

	if (!actor.AddComponent(mesh))
	{
		SE_LOG(LogSampleGameInstance, Fatal, "Adding the sample mesh component failed");
	}
}

TUniquePtr<UGameInstance> CreateSampleGameInstance(UEngine& inEngine)
{
	return MakeUnique<USampleGameInstance>(inEngine);
}
