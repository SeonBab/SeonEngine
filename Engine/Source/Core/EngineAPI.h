#pragma once

// 빌드 대상이 두 값을 각각 0 또는 1로 지정한다. 현재 단일 실행 빌드는 둘 다 0이다.
// 엔진 DLL을 만드는 쪽과 가져오는 쪽을 동시에 선택할 수 없다.
#if (BUILD_ENGINE_DLL != 0 && BUILD_ENGINE_DLL != 1) || (BUILD_ENGINE_DLL_CONSUMER != 0 && BUILD_ENGINE_DLL_CONSUMER != 1)
#error Engine DLL build flags must be 0 or 1.
#endif

#if BUILD_ENGINE_DLL && BUILD_ENGINE_DLL_CONSUMER
#error Engine DLL export and import modes cannot be enabled together.
#endif

// 모듈 밖에서 사용할 엔진 선언에 붙인다. DLL을 사용하지 않는 빌드에서는 빈 표기다.
#if BUILD_ENGINE_DLL
#define ENGINE_API __declspec(dllexport)
#elif BUILD_ENGINE_DLL_CONSUMER
#define ENGINE_API __declspec(dllimport)
#else
#define ENGINE_API
#endif
