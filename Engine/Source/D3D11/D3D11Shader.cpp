#include "D3D11Shader.h"

#include "Core/Logging/Log.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <utility>

bool ReadShaderBytecode(const std::filesystem::path& path, TArray<uint8>& outBytecode)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		WriteLog("D3D11", ELogLevel::Error, "Opening shader bytecode file failed.");
		return false;
	}

	const std::streampos endPosition = file.tellg();
	if (endPosition <= std::streampos(0))
	{
		WriteLog("D3D11", ELogLevel::Error, "Shader bytecode file is empty or its size could not be read.");
		return false;
	}

	const std::uintmax_t byteCount = static_cast<std::uintmax_t>(static_cast<std::streamoff>(endPosition));
	TArray<uint8> newBytecode;
	if (byteCount > newBytecode.max_size() || byteCount > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
	{
		WriteLog("D3D11", ELogLevel::Error, "Shader bytecode file exceeds the supported read size.");
		return false;
	}

	newBytecode.resize(static_cast<size_t>(byteCount));
	file.seekg(0, std::ios::beg);
	if (!file.read(reinterpret_cast<char*>(newBytecode.data()), static_cast<std::streamsize>(byteCount)))
	{
		WriteLog("D3D11", ELogLevel::Error, "Reading the complete shader bytecode file failed.");
		return false;
	}

	outBytecode = std::move(newBytecode);

	return true;
}
