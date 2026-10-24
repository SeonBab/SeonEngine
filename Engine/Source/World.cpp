#include "World.h"

#include "Actor.h"
#include "Components/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/RenderStateComponent.h"
#include "Components/TransformComponent.h"
#include "Input/PlayerInput.h"

#include <algorithm>

#pragma region Lifecycle

UWorld::UWorld() = default;

UWorld::~UWorld()
{
	EndPlay(EEndPlayReason::Quit);

	while (!actors.empty())
	{
		(void)DestroyActorImmediately(actors.back().get(), EEndPlayReason::Quit);
	}
}

#pragma endregion

#pragma region Rendering

FScene& UWorld::GetScene()
{
	return scene;
}

const FScene& UWorld::GetScene() const
{
	return scene;
}

bool UWorld::CreateRenderStates()
{
	TArray<URenderStateComponent*> renderComponents;
	for (const TUniquePtr<AActor>& actor : actors)
	{
		actor->GetComponents(renderComponents);
		for (URenderStateComponent* component : renderComponents)
		{
			if (component->IsRenderStateCreated()) { continue; }
			if (!component->CreateRenderState()) { return false; }
		}
	}

	return true;
}

bool UWorld::SendAllEndOfFrameUpdates()
{
	TArray<URenderStateComponent*> renderComponents;
	for (const TUniquePtr<AActor>& actor : actors)
	{
		actor->GetComponents(renderComponents);
		for (URenderStateComponent* component : renderComponents)
		{
			if (!component->IsRenderStateCreated()) { continue; }
			if (!component->SendRenderUpdates()) { return false; }
		}
	}

	return true;
}

#pragma endregion

#pragma region Actors

void UWorld::GetActors(TArray<AActor*>& outActors) const
{
	outActors.clear();
	for (const TUniquePtr<AActor>& actor : actors)
	{
		outActors.push_back(actor.get());
	}
}

bool UWorld::SetActorParent(AActor* actor, AActor* newParent, EActorTransformRule rule)
{
	if (!actor || actor == newParent)
	{
		return false;
	}

	bool bActorRegistered = false;
	bool bParentRegistered = newParent == nullptr;
	for (const TUniquePtr<AActor>& ownedActor : actors)
	{
		bActorRegistered = bActorRegistered || ownedActor.get() == actor;
		bParentRegistered = bParentRegistered || ownedActor.get() == newParent;
		if (bActorRegistered && bParentRegistered)
		{
			break;
		}
	}
	if (!bActorRegistered || !bParentRegistered)
	{
		return false;
	}
	if (&actor->GetWorld() != this || (newParent && &newParent->GetWorld() != this))
	{
		return false;
	}
	if (actor->IsActorBeingDestroyed() || (newParent && newParent->IsActorBeingDestroyed())) { return false; }
	for (const AActor* ancestor = newParent; ancestor; ancestor = ancestor->parent)
	{
		if (ancestor == actor)
		{
			return false;
		}
	}
	if (actor->parent == newParent)
	{
		return true;
	}

	AActor* const oldParent = actor->parent;
	TArray<AActor*>::iterator oldChild{};
	if (oldParent)
	{
		oldChild = std::find(oldParent->children.begin(), oldParent->children.end(), actor);
		if (oldChild == oldParent->children.end())
		{
			return false;
		}
	}
	if (newParent && newParent->children.size() == newParent->children.capacity())
	{
		// 관계 변경 전에 저장 공간을 준비한다. 두 부모는 서로 다르므로 기존 반복자는 유지된다.
		constexpr TArray<AActor*>::size_type GrowthFactor = 2;
		newParent->children.reserve(newParent->children.empty() ? 1 : newParent->children.size() * GrowthFactor);
	}
	UTransformComponent* actorTransform = actor->GetComponent<UTransformComponent>();
	FTransform previousWorld{};
	if (rule == EActorTransformRule::KeepWorld)
	{
		previousWorld = actorTransform->GetComponentTransform();
	}
	if (oldParent)
	{
		oldParent->children.erase(oldChild);
	}
	if (newParent)
	{
		newParent->children.push_back(actor);
	}
	actor->parent = newParent;
	if (rule == EActorTransformRule::KeepWorld)
	{
		actorTransform->SetComponentTransform(previousWorld);
	}
	else
	{
		// 상대값을 유지해도 부모가 바뀌면 자신과 자손의 월드 캐시는 갱신한다.
		actorTransform->UpdateComponentToWorld();
	}

	return true;
}

#pragma endregion

#pragma region Camera

bool UWorld::SetActiveCamera(AActor* actor)
{
	if (!actor)
	{
		activeCameraActor = nullptr;

		return true;
	}

	const auto ownedActor = std::find_if(actors.begin(), actors.end(), [actor](const TUniquePtr<AActor>& entry)
	{
		return entry.get() == actor;
	});
	if (ownedActor == actors.end()) { return false; }
	if (&actor->GetWorld() != this || actor->IsActorBeingDestroyed()) { return false; }
	if (!actor->GetComponent<UCameraComponent>()) { return false; }

	activeCameraActor = actor;

	return true;
}

UCameraComponent* UWorld::GetActiveCamera() const
{
	if (!activeCameraActor) { return nullptr; }

	return activeCameraActor->GetComponent<UCameraComponent>();
}

#pragma endregion

#pragma region Destruction

bool UWorld::DestroyActor(AActor* actor)
{
	if (!actor) { return false; }

	const auto ownedActor = std::find_if(actors.begin(), actors.end(), [actor](const TUniquePtr<AActor>& entry)
	{
		return entry.get() == actor;
	});
	if (ownedActor == actors.end()) { return false; }
	if (&actor->GetWorld() != this) { return false; }

	// TODO(seon): 월드 시작·종료 알림과 Tick 외의 콜백에서도 논리 제거와 메모리 회수의 경계를 검토한다.
	const bool bIsEndingPlay = bIsTearingDown && bBegunPlay;
	if (!bIsTicking && !bIsEndingPlay) { return DestroyActorImmediately(actor); }
	if (actor->IsActorBeingDestroyed()) { return true; }

	if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) == pendingDestroyActors.end())
	{
		pendingDestroyActors.push_back(actor);
	}

	return true;
}

bool UWorld::DestroyActorImmediately(AActor* actor, EEndPlayReason reason)
{
	TArray<AActor*> subtree{actor};
	for (TArray<AActor*>::size_type index = 0; index < subtree.size(); ++index)
	{
		for (AActor* child : subtree[index]->children)
		{
			subtree.push_back(child);
		}
	}

	TArray<TArray<TUniquePtr<AActor>>::size_type> actorIndices;
	actorIndices.reserve(subtree.size());
	for (AActor* target : subtree)
	{
		const auto entry = std::find_if(actors.begin(), actors.end(), [target](const TUniquePtr<AActor>& candidate)
		{
			return candidate.get() == target;
		});
		if (entry == actors.end()) { return false; }

		actorIndices.push_back(static_cast<TArray<TUniquePtr<AActor>>::size_type>(entry - actors.begin()));
	}
	TArray<TUniquePtr<AActor>> removedActors;
	removedActors.reserve(subtree.size());

	AActor* const oldParent = actor->parent;
	TArray<AActor*>::iterator oldChild{};
	if (oldParent)
	{
		oldChild = std::find(oldParent->children.begin(), oldParent->children.end(), actor);
		if (oldChild == oldParent->children.end()) { return false; }
	}
	if (oldParent)
	{
		oldParent->children.erase(oldChild);
	}
	if (std::find(subtree.begin(), subtree.end(), activeCameraActor) != subtree.end())
	{
		activeCameraActor = nullptr;
	}
	// 소멸 중 같은 주소에 새 액터가 생성돼도 이전 요청이 남지 않게 한다.
	std::erase_if(pendingDestroyActors, [&subtree](AActor* pendingActor)
	{
		return std::find(subtree.begin(), subtree.end(), pendingActor) != subtree.end();
	});
	for (AActor* target : subtree)
	{
		target->bActorIsBeingDestroyed = true;
		target->parent = nullptr;
		target->children.clear();
		// 관계 해제 뒤 종료 콜백에서도 부모 없는 상대값과 일치하는 월드 캐시를 읽게 한다.
		target->GetComponent<UTransformComponent>()->UpdateComponentToWorld();
	}
	for (const TArray<TUniquePtr<AActor>>::size_type index : actorIndices)
	{
		removedActors.push_back(std::move(actors[index]));
	}
	std::erase_if(actors, [](const TUniquePtr<AActor>& entry)
	{
		return !entry;
	});

	// World 배열 정리를 마친 뒤 종료 처리와 소멸자를 호출해 재진입이 순회/반복자를 무효화하지 않게 한다.
	for (auto entry = removedActors.rbegin(); entry != removedActors.rend(); ++entry)
	{
		(*entry)->RouteEndPlay(reason);
		entry->reset();
	}

	return true;
}

void UWorld::PurgeDestroyedActors()
{
	while (!pendingDestroyActors.empty())
	{
		AActor* actor = pendingDestroyActors.back();
		pendingDestroyActors.pop_back();

		(void)DestroyActorImmediately(actor);
	}
}

#pragma endregion

#pragma region PlayLifecycle

void UWorld::BeginPlay()
{
	if (bIsTicking || bBegunPlay || bIsTearingDown) { return; }

	const TArray<TUniquePtr<AActor>>::size_type actorCount = actors.size();
	bIsTicking = true;
	for (decltype(actors.size()) index = 0; index < actorCount; ++index)
	{
		AActor* actor = actors[index].get();
		if (actor->IsActorBeingDestroyed()) { continue; }
		if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
		{
			continue;
		}

		actor->DispatchBeginPlay();
	}
	PurgeDestroyedActors();
	bBegunPlay = true;
	bIsTicking = false;
}

bool UWorld::HasBegunPlay() const
{
	return bBegunPlay;
}

void UWorld::Tick(float deltaSeconds, const FInputState& input, UPlayerInput& playerInput)
{
	if (bIsTicking || !bBegunPlay || bIsTearingDown) { return; }

	const TArray<TUniquePtr<AActor>>::size_type tickCount = actors.size();
	TArray<UInputComponent*> inputComponents;
	TArray<UActorComponent*> tickComponents;
	bIsTicking = true;
	for (decltype(actors.size()) index = 0; index < tickCount; ++index)
	{
		AActor* actor = actors[index].get();
		if (actor->IsActorBeingDestroyed()) { continue; }
		if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
		{
			continue;
		}

		actor->DispatchBeginPlay();
		if (!actor->HasActorBegunPlay()) { continue; }
		if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
		{
			continue;
		}

		// TODO(seon): 입력 우선순위, 소비, 플레이어별 대상이 필요해지면 입력 처리 계층으로 배분 책임을 옮긴다.
		actor->GetComponents(inputComponents);
		for (UInputComponent* component : inputComponents)
		{
			component->ProcessInput(input, deltaSeconds);
			if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
			{
				break;
			}

			playerInput.DispatchActionEvents(*component, deltaSeconds);
			if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
			{
				break;
			}
		}
		if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
		{
			continue;
		}

		if (actor->IsActorTickEnabled())
		{
			actor->Tick(deltaSeconds);
		}
		if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
		{
			continue;
		}

		actor->GetComponents(tickComponents);
		for (UActorComponent* component : tickComponents)
		{
			if (!component->HasBegunPlay() || !component->IsComponentTickEnabled())
			{
				continue;
			}

			component->TickComponent(deltaSeconds);
			if (std::find(pendingDestroyActors.begin(), pendingDestroyActors.end(), actor) != pendingDestroyActors.end())
			{
				break;
			}
		}
	}
	PurgeDestroyedActors();
	bIsTicking = false;
}

void UWorld::EndPlay(EEndPlayReason reason)
{
	if (bIsTicking || bIsTearingDown) { return; }

	bIsTearingDown = true;
	if (!bBegunPlay) { return; }

	const TArray<TUniquePtr<AActor>>::size_type actorCount = actors.size();
	for (decltype(actors.size()) index = 0; index < actorCount; ++index)
	{
		actors[index]->RouteEndPlay(reason);
	}
	bBegunPlay = false;

	PurgeDestroyedActors();
}

#pragma endregion
