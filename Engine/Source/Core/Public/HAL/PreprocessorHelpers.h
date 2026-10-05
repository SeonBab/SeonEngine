#pragma once

// 전처리기 도우미. Unreal HAL/PreprocessorHelpers.h에 해당한다.

// 인자를 문자열로 만든다. 한 단계를 거쳐야 인자 안의 매크로가 먼저 펼쳐진다
#define SE_PRIVATE_STRINGIZE(Token) #Token
#define SE_STRINGIZE(Token) SE_PRIVATE_STRINGIZE(Token)

// 두 토큰을 하나로 붙인다. 한 단계를 거쳐야 인자 안의 매크로가 먼저 펼쳐진다
#define SE_PRIVATE_JOIN(TokenA, TokenB) TokenA##TokenB
#define SE_JOIN(TokenA, TokenB) SE_PRIVATE_JOIN(TokenA, TokenB)

// 플랫폼 헤더 경로를 만드는 데 쓰는 플랫폼 이름(Windows 등)은 빌드 설정(Directory.Build.props)이 준다
#if !defined(SE_PLATFORM_HEADER_NAME)
#error "빌드 설정에서 SE_PLATFORM_HEADER_NAME을 정의해야 합니다."
#endif

// 빌드하는 플랫폼의 헤더 경로를 만든다. SE_COMPILED_PLATFORM_HEADER(PlatformMisc.h) → "Windows/WindowsPlatformMisc.h"
#define SE_COMPILED_PLATFORM_HEADER(Suffix) SE_STRINGIZE(SE_JOIN(SE_PLATFORM_HEADER_NAME/SE_PLATFORM_HEADER_NAME, Suffix))
