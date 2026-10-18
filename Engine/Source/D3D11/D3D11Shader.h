#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"

#include <filesystem>

/** 비어 있지 않은 파일 전체를 읽는다. 실패하면 로그와 false를 반환하고 출력은 유지한다. 셰이더 유효성은 검사하지 않는다. */
[[nodiscard]] bool ReadShaderBytecode(const std::filesystem::path& path, TArray<uint8>& outBytecode);
