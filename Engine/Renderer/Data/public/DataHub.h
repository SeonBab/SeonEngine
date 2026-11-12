#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>
#include <concepts>

#include <DirectXMath.h>


// 레이어 간 데이터 인터페이스 / 프로토콜 을 정의합니다
// ------
// v0.1.0-alpha: 헤더 데이터 양식 / 의존성 관리 일부 변경(인터페이스 조정) / 기존 미구현 요소 일부 구현

// 변경점, 유의사항: 
// 기존 TextureType_Diffuse / CubeMap 이름이 이제 자동으로 sRGB 변환되지 않음 (포맷 + 컬러스페이스로 직접 지정)
// 기본 리소스 업로드 모두 Create로 사양 변경, 리소스 Create한 해당 프레임 이후 업로드(RTBCMD) 시 정상 동작 보장 불가
// -> 계약 변경: 리소스는 Create 시점과 동일한 프레임에 업로드 되어야 함


// 기본 헤더

#include "DataHub_Types.h"
#include "DataHub_RTB.h"
#include "DataHub_Resource.h"


namespace Hakko::DataHub
{
    
    #pragma region Lifecycle

    bool Init();
    void Finalize();
    
    #pragma endregion


    #pragma region Helper

    constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
    constexpr uint32_t FNV_PRIME = 16777619u;

    constexpr uint32_t HashString(std::string_view str) {
        uint32_t hash = FNV_OFFSET_BASIS;
        for (char c : str) {
            hash ^= static_cast<uint32_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    constexpr uint32_t HashString(std::wstring_view str) {
        uint32_t hash = FNV_OFFSET_BASIS;
        for (wchar_t c : str) {
            hash ^= static_cast<uint32_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    #pragma endregion

}


// 필수 포함 헤더

#include "DataHub_RenderIdProvider.h"
#include "DataHub_DataBoxCustomizer.h"