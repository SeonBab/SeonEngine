#include "InputCoreTypes.h"

#include "HAL/PlatformInput.h"

#include <functional>

const FKey EKeys::Invalid;
const FKey EKeys::W("W");
const FKey EKeys::A("A");
const FKey EKeys::S("S");
const FKey EKeys::D("D");

TSharedPtr<FInputKeyManager> FInputKeyManager::instance = nullptr;

FKey::FKey(const FString& inName)
	: keyName(inName)
{
}

bool FKey::operator==(const FKey& other) const
{
	return keyName == other.keyName;
}

uint32 GetTypeHash(const FKey& key)
{
	return static_cast<uint32>(std::hash<FString>{}(key.keyName));
}

FInputKeyManager& FInputKeyManager::Get()
{
	if (!instance)
	{
		instance = TSharedPtr<FInputKeyManager>(new FInputKeyManager());
	}

	return *instance;
}

FInputKeyManager::FInputKeyManager()
{
	InitKeyMappings();
}

void FInputKeyManager::InitKeyMappings()
{
	constexpr uint32 maxMappings = 256;
	uint32 keyCodes[maxMappings]{};
	FString keyNames[maxMappings]{};

	const uint32 numMappings = FPlatformInput::GetKeyMap(keyCodes, keyNames, maxMappings);

	keyMapVirtualToEnum.Reset();

	for (uint32 index = 0; index < numMappings; ++index)
	{
		keyMapVirtualToEnum.Add(keyCodes[index], FKey(keyNames[index]));
	}
}

FKey FInputKeyManager::GetKeyFromCodes(uint32 keyCode, [[maybe_unused]] uint32 characterCode) const
{
	const FKey* key = keyMapVirtualToEnum.Find(keyCode);
	if (key == nullptr)
	{
		return EKeys::Invalid;
	}

	return *key;
}
