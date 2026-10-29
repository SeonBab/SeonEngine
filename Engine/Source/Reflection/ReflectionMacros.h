#pragma once

// 분석 때만 표시를 AST에 남긴다. 일반 C++ 컴파일에는 별도의 속성을 요구하지 않는다.
// 분석 모드는 GenerateReflection 도구 전용이며 제품 컴파일에서 정의하지 않는다.
#if !defined(SE_REFLECTION_ANALYSIS)
#define SE_REFLECTION_ANALYSIS 0
#endif

#if SE_REFLECTION_ANALYSIS
#if !defined(__clang__)
#error Reflection analysis requires Clang.
#endif

#define UCLASS() static_assert(true, "SE reflected class");
#define UPROPERTY() [[clang::annotate("se_property")]]
#define GENERATED_BODY() static_assert(true, "SE generated body");
#else
#define UCLASS()
#define UPROPERTY()

// 생성 헤더의 파일 식별자와 호출 줄에 해당하는 선언 묶음을 선택한다.
// generated.h는 해당 사용자 헤더의 마지막 include여야 한다. 매크로 뒤에는 접근 지정자를 명시한다.
#define SE_REFLECTION_BODY_NAME_INNER(file, line) file##_##line##_BODY
#define SE_REFLECTION_BODY_NAME(file, line) SE_REFLECTION_BODY_NAME_INNER(file, line)
#define GENERATED_BODY() SE_REFLECTION_BODY_NAME(SE_REFLECTION_FILE_ID, __LINE__)
#endif
