#include "D3D11Shader.h"

#include "Core/Logging/Log.h"
#include "Core/Misc/FileHelper.h"

#include <utility>

bool ReadShaderBytecode(const std::filesystem::path& path, TArray<uint8>& outBytecode)
{
	TArray<uint8> newBytecode;
	if (!FFileHelper::ReadBinaryFile(path, newBytecode)) { return false; }
	if (newBytecode.empty())
	{
		WriteLog("D3D11", ELogLevel::Error, "Shader bytecode file is empty.");
		return false;
	}

	outBytecode = std::move(newBytecode);

	return true;
}
