#include "Windows/WindowsPlatformInput.h"

#include <array>

uint32 FWindowsPlatformInput::GetKeyMap(uint32* keyCodes, FString* keyNames, uint32 maxMappings)
{
	if (keyCodes == nullptr || keyNames == nullptr || maxMappings == 0) { return 0; }

	// Windows의 A-Z 가상 키 번호는 해당 대문자 문자 코드와 같다.
	constexpr std::array<uint32, 4> codes = {'W', 'A', 'S', 'D'};
	constexpr std::array<const char*, 4> names = {"W", "A", "S", "D"};

	uint32 numMappings = 0;
	for (uint32 index = 0; index < codes.size() && numMappings < maxMappings; ++index)
	{
		keyCodes[numMappings] = codes[index];
		keyNames[numMappings] = names[index];
		++numMappings;
	}

	return numMappings;
}
