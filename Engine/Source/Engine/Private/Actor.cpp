#include "Actor.h"

#include "ActorComponent.h"
#include "TransformComponent.h"
#include "World.h"

#include <utility>

FActor::FActor(FWorld& inWorld)
	: world(&inWorld)
{
	components.push_back(MakeUnique<FTransformComponent>(*this));
}

FActor::~FActor() = default;

FWorld& FActor::GetWorld() const
{
	return *world;
}

FActor* FActor::GetParent() const
{
	return parent;
}

bool FActor::SetParent(FActor* newParent, ETransformRule rule)
{
	if (rule != ETransformRule::KeepWorld && rule != ETransformRule::KeepLocal)
	{
		return false;
	}
	if (bActorIsBeingDestroyed || (newParent && newParent->bActorIsBeingDestroyed))
	{
		return false;
	}
	if (newParent == this || (newParent && newParent->world != world))
	{
		return false;
	}
	if (!IsOwnedByWorld() || (newParent && !newParent->IsOwnedByWorld()))
	{
		return false;
	}
	for (const FActor* ancestor = newParent; ancestor; ancestor = ancestor->parent)
	{
		if (ancestor == this) { return false; }
	}
	if (parent == newParent) { return true; }

	TArray<TUniquePtr<FActor>>& currentOwners = parent ? parent->children : world->actors;
	TArray<TUniquePtr<FActor>>::iterator ownedActor = currentOwners.begin();
	while (ownedActor != currentOwners.end() && ownedActor->get() != this)
	{
		++ownedActor;
	}
	if (ownedActor == currentOwners.end()) { return false; }

	FTransformComponent* transformComponent = GetComponent<FTransformComponent>();
	FTransform newLocal = transformComponent->GetTransform();
	if (rule == ETransformRule::KeepWorld)
	{
		const FTransform oldWorld = transformComponent->GetComponentTransform();
		newLocal = newParent ? oldWorld.GetRelativeTransform(newParent->GetComponent<FTransformComponent>()->GetComponentTransform()) : oldWorld;
	}

	TArray<TUniquePtr<FActor>>& newOwners = newParent ? newParent->children : world->actors;
	// 할당 실패가 소유권 이전 도중 발생하지 않도록 먼저 저장 공간을 준비한다.
	if (newOwners.size() == newOwners.capacity())
	{
		constexpr TArray<TUniquePtr<FActor>>::size_type growthFactor = 2;
		newOwners.reserve(newOwners.empty() ? 1 : newOwners.size() * growthFactor);
	}
	TUniquePtr<FActor> owned = std::move(*ownedActor);
	currentOwners.erase(ownedActor);
	newOwners.push_back(std::move(owned));
	parent = newParent;
	transformComponent->SetRelativeTransform(newLocal);

	return true;
}

bool FActor::IsOwnedByWorld() const
{
	const FActor* root = this;
	while (root->parent)
	{
		root = root->parent;
	}
	for (const TUniquePtr<FActor>& actor : world->actors)
	{
		if (actor.get() == root) { return true; }
	}

	return false;
}

int32 FActor::GetChildCount() const
{
	return static_cast<int32>(children.size());
}

FActor* FActor::GetChild(int32 index) const
{
	if (index < 0 || static_cast<TArray<TUniquePtr<FActor>>::size_type>(index) >= children.size())
	{
		return nullptr;
	}

	return children[index].get();
}

bool FActor::IsActorBeingDestroyed() const
{
	return bActorIsBeingDestroyed;
}

bool FActor::Destroy()
{
	if (!IsActorBeingDestroyed())
	{
		(void)GetWorld().DestroyActor(this);
	}

	return IsActorBeingDestroyed();
}
