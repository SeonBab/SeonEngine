# SeonEngine Code Convention

## 관련 문서

- [Architecture](../../../Docs/Architecture.md) — 모듈, 서브시스템, 플랫폼, 렌더링 등 엔진 구조
- [Project Settings](../ProjectSettings.md) — 프로젝트에 적용한 설정
- Deferred Tasks — 필요해지면 진행할 작업
- [Git Convention](GitConvention.md) — 브랜치, 커밋, 병합
- Asset / Resource Naming Convention — 별도 문서 (예정)

## 적용 범위

| 대상 | 적용 |
|---|---|
| 엔진 코드 (`Engine/`) | **필수**. 이 문서의 모든 규칙을 따른다. 서식은 저장소 루트의 `.clang-format`이 적용한다 |
| 게임 코드 (`SampleGame` 등 게임 모듈) | **권장**. 엔진과 같은 규칙을 권하지만 강제하지 않는다. 서식은 저장소 루트의 `.clang-format`이 기본으로 적용되고, 게임 폴더에 자기 `.clang-format`을 두면 그쪽이 우선한다 |
| 서드파티 (`Engine/ThirdParty/`) | 적용하지 않는다(6.3 예외 조항) |

## 목차

- 1장. Style & Naming
  - 1.1 Formatting
  - 1.2 Naming
  - 1.3 Comment
  - 1.4 API Documentation
- 2장. Files & Code Layout
  - 2.1 파일 구성
  - 2.2 Header 규칙
  - 2.3 Class 구성
  - 2.4 Namespace
- 3장. C++ Language Rules
  - 3.1 C++ 표준 및 기능 정책
  - 3.2 함수 작성
  - 3.3 `const` / `constexpr`
  - 3.4 Pointer / Reference
  - 3.5 `auto`
  - 3.6 Cast
  - 3.7 Enum
  - 3.8 상수 / Literal
  - 3.9 Macro
  - 3.10 Template
  - 3.11 Lambda
  - 3.12 STL 사용 정책
- 4장. Memory & Lifetime
  - 4.1 메모리 소유권
  - 4.2 객체 생명주기
- 5장. Error Handling & Diagnostics
  - 5.1 에러 처리
  - 5.2 Assert
  - 5.3 Logging
- 6장. Tooling & Enforcement
  - 6.1 빌드 / 컴파일러 설정
  - 6.2 자동화 도구
  - 6.3 예외 조항

---

## 1장. Style & Naming

### 1.1 Formatting `필수`

이 절의 규칙은 대부분 `.clang-format`이 자동으로 적용한다(6.2 자동화 도구 참고). 도구가 구분하지 못하는 규칙(한 줄 early exit의 범위, 반환 전 빈 줄)만 직접 지킨다.

- **들여쓰기** — Tab 사용, 표시 폭 4
- **한 줄 최대 길이** — 제한 없음
- **중괄호 스타일** — Allman. 여는 중괄호는 항상 다음 줄에 둔다.
- **제어문 중괄호** — `if` / `for` / `while` 등은 본문이 한 줄이어도 중괄호를 생략하지 않는다.
- **한 줄 early exit** — 본문이 `return` / `continue` / `break` 한 문장뿐이고 `else`가 없는 `if`에 한해 한 줄 표기를 허용한다. 그 외에는 Allman을 따른다.

  ```cpp
  // 허용
  if (!device) { return; }
  if (!IsVisible(object)) { continue; }

  // 금지 — 중괄호 생략
  if (!device)
    return;

  // 금지 — early exit가 아닌 본문은 Allman
  if (mesh) { mesh->Upload(); }
  ```

- **한 줄 getter** — 클래스 안에서 정의하는 getter 중 본문이 `return` 한 문장뿐인 경우 한 줄 표기를 허용한다. setter와 그 외 함수는 Allman을 따른다.

  ```cpp
  class FMesh
  {
  public:
    uint32 GetVertexCount() const { return vertexCount; }
    bool IsVisible() const { return bVisible; }
  };
  ```

- **switch** — `case`는 `switch`보다 한 단계 들여쓴다. `case` 안에서 변수를 선언하는 등 블록이 필요하면 `case` 다음 줄에 중괄호를 연다.
- **포인터 / 레퍼런스 위치** — `*`와 `&`는 타입 쪽에 붙인다(`FTexture* texture`).
- **공백**
  - 넣는 곳: 제어문 키워드(`if`, `for`, `while`, `switch`) 뒤, 이항·대입 연산자 양쪽, 쉼표와 `for`의 `;` 뒤
  - 넣지 않는 곳: 함수 이름과 `(` 사이, 괄호 안쪽, 단항 연산자 뒤, `template`과 `<` 사이
- **한 줄에 한 문장** — 한 줄에 문장은 하나, 변수 선언도 하나만 쓴다. 단, 위의 한 줄 early exit와 한 줄 getter는 예외로 허용한다.
- **정렬** — 연속한 줄의 대입(`=`), 선언 이름, 매크로 값은 공백으로 맞추지 않는다. 줄 끝 주석만 맞춘다(빈 줄이나 다른 종류의 줄에서 묶음이 끊긴다). 직접 맞추지 않고 clang-format에 맡긴다. 표 모양이 꼭 필요한 코드는 6.3 예외로 쓴다.

  ```cpp
  using int8 = signed char;
  using int16 = signed short;

  HWND hwnd = CreateWindowExW(
    0,                   // 확장 스타일
    WS_OVERLAPPEDWINDOW, // 스타일
    ...);
  ```
- **인코딩 / 줄바꿈** — UTF-8 (BOM 없음), CRLF. 파일은 newline으로 끝낸다. 관련 프로젝트 설정은 [Project Settings](../ProjectSettings.md) 참고.
- **빈 줄** — 함수 정의 사이, 함수 안의 논리 단위 사이, `#pragma once` / include 묶음 / 선언 사이에 1개를 둔다. 2개 이상 연속으로 쓰지 않고, 여는 `{` 바로 뒤와 닫는 `}` 바로 앞에는 두지 않는다.

- **반환 전 빈 줄** — 함수 마지막의 `return` 앞에 다른 코드가 있으면 빈 줄 1개를 둔다. 본문이 `return` 한 문장뿐인 함수와 한 줄 early exit에는 적용하지 않는다.

### 1.2 Naming `필수`

엔진의 공개 타입은 namespace 없이 전역에 두고(2.4 Namespace 참고), 타입 접두사로 종류를 구분하고 이름 충돌을 막는다.

| 대상 | 규칙 | 예시 |
|---|---|---|
| 타입 | 접두사 + PascalCase (아래 타입 접두사 표) | `FRenderer`, `TRingBuffer`, `IRenderDevice` |
| Enum 값 | PascalCase | `ETextureFormat::RGBA8` |
| Template parameter | `T`, 또는 PascalCase + `Type` | `T`, `KeyType`, `InElementType` |
| Function / Method | PascalCase | `CreateTexture()` |
| Namespace | PascalCase | `SE::Private` |
| 지역 변수 / 매개변수 | camelCase | `vertexCount`, `newWidth` |
| 멤버 변수 | camelCase, 접두사·접미사 없음 | `vertexCount` |
| bool 변수 | `b` + PascalCase | `bVisible`, `bHasParent` |
| bool 반환 함수 | `Is` / `Has` / `Can` 등 + PascalCase | `IsVisible()`, `CanRender()` |
| 상수 / `constexpr` | PascalCase | `MaxLightCount` |
| 전역 변수 | `g` + PascalCase | `gEngine` |
| static 멤버 변수 | 일반 멤버 변수와 동일 (접두사 없음) | `liveCount` |
| Macro | `SE_` + UPPER_SNAKE_CASE | `SE_ASSERT`, `SE_PLATFORM_WINDOWS` |

- **렌더러의 Unreal 대응 이름** — 직접 확인한 Unreal 코드와 역할이 대응하는 렌더러 변수 / 매개변수는 원본 이름을 우선한다. 이 범위에서는 PascalCase와 입력 매개변수의 `In` 접두사를 허용한다(예: `Direct3DDevice`, `SizeX`, `InSizeX`). 역할이 다르면 이름을 억지로 맞추지 않고 차이를 설명한다. 다른 시스템에는 위 기본 표기를 유지한다.

- **월드 시간 멤버의 Unreal 대응 이름** — `FWorld`의 `DeltaTimeSeconds`와 `TimeSeconds`는 직접 확인한 `UWorld`의 대응 이름과 자료형을 그대로 사용한다. 이 두 멤버에 한해 PascalCase를 허용하며, 다른 월드 멤버는 기본 표기를 유지한다.

- **액터 Tick의 Unreal 대응 인수 이름** — `FActor::Tick`의 시간 인수는 직접 확인한 `AActor::Tick`과 같은 `DeltaSeconds`를 사용한다. 이 인수에 한해 PascalCase를 허용하며, 빈 기본 본문에서도 이름을 유지하고 `[[maybe_unused]]`로 미사용을 표시한다.

- **타입 접두사** — 모든 타입에 접두사를 붙이며, 각 글자는 타입의 종류를 나타낸다. 표기는 Unreal 관례를 따른다.

  | 접두사 | 뜻 | 대상 |
  |---|---|---|
  | `F` | — | 다른 접두사가 적용되지 않는 일반 class / struct (값 타입, 소유권 등 특정 성질을 뜻하지 않음) |
  | `T` | Template | 템플릿 class / struct |
  | `I` | Interface | 순수 가상 함수로만 이루어진 class |
  | `E` | Enum | 열거형 |
  | `C` | Concept | C++20 concept (3.10 Template 참고) |
  | `U` | UObject 계열 | 자체 객체 루트 `UObject`와 그 파생 클래스 (`UEngine`, `UGameInstance` 등). 최소 UObject·UEngine·UGameInstance 구현 완료 / 실제 실행 연결 전이며 GC·리플렉션 구현 여부를 뜻하지 않음 |

  Interface를 구현하는 클래스는 일반 타입이므로 `F`를 붙인다.

  ```cpp
  class FRenderer;
  struct FVertexData;

  template<typename T>
  class TRingBuffer;

  class IRenderDevice;
  class FD3D12RenderDevice : public IRenderDevice { ... };

  enum class ETextureFormat : uint8;
  ```

- **Template parameter** — 파라미터가 하나면 `T`를 쓴다. 의미를 드러내야 하면 PascalCase + `Type`을 쓴다. 파라미터를 `using`으로 다시 노출할 때는 파라미터에 `In`을 붙여 별칭과 구분한다.

  ```cpp
  template<typename KeyType, typename ValueType>
  class TSparseMap;

  template<typename InElementType>
  class TRingBuffer
  {
  public:
    using ElementType = InElementType;
  };
  ```

- **멤버 변수** — `m`, `m_`, `_` 같은 표식을 붙이지 않고 이름 자체를 명확하게 짓는다. 단, bool 멤버 변수는 예외로 `b`를 붙인다.
- **매개변수와 멤버 이름 충돌 금지** — 매개변수는 멤버 변수와 다른 이름을 쓴다. 이름으로 구분되므로 `this->`는 필요하지 않다. (MSVC 경고 C4458로 검출된다. 6.1 빌드 / 컴파일러 설정 참고)

  ```cpp
  class FMesh
  {
  public:
    explicit FMesh(uint32 initialVertexCount)
        : vertexCount(initialVertexCount)
    {
    }

    void SetVertexCount(uint32 newVertexCount)
    {
        vertexCount = newVertexCount;
    }

  private:
    uint32 vertexCount = 0;
  };
  ```

- **약어** — 약어는 대문자를 유지한다. camelCase 이름이 약어로 시작하면 약어 전체를 소문자로 쓴다. 외부 API 이름(`ID3D12Device` 등)은 원래 이름을 그대로 쓴다.

  ```cpp
  class FRHIDevice;
  class FGPUBuffer;
  class FD3D12RenderDevice;

  uint32 textureID;
  FGPUBuffer* gpuBuffer = nullptr;
  ```

- **전역 변수** — `g` 접두사를 붙여 전역임을 드러낸다. static 멤버 변수는 `Class::`로 접근하므로 접두사를 붙이지 않는다.
- **Getter / Setter** — `Get` / `Set` 접두어를 쓴다(`GetWidth()`, `SetWidth(uint32 newWidth)`). bool은 `IsVisible()` / `SetVisible(bool bNewVisible)`처럼 쓴다.
- **Type alias** — `typedef`는 쓰지 않고 `using`만 쓴다. 별칭 이름은 가리키는 타입의 종류에 맞는 접두사를 붙인다.
  - 구체적인 타입의 별칭은 `F` (템플릿 인스턴스도 더 이상 템플릿이 아니므로 `F`)
  - 별칭 템플릿은 `T`
  - 예외: 클래스 안의 멤버 별칭(`ElementType` 등)은 접두사 없이 PascalCase, 기본 정수 별칭(`int32`, `uint8` 등)은 소문자

  ```cpp
  using FMeshList = TArray<FMesh*>;

  template<typename T>
  using TSharedPtr = std::shared_ptr<T>;
  ```

### 1.3 Comment `중요`

- **언어** — 주석은 한국어로 쓴다.
- **원칙** — 이유, 제약, 주의사항(Why)을 우선으로 쓴다. 흐름 구분이나 복잡한 로직의 요약처럼 읽는 데 도움이 되는 설명(What)은 자유롭게 쓸 수 있다. 단, 코드 한 줄을 그대로 풀어 쓰는 주석은 쓰지 않는다.
- **문서 참조 금지** — 주석에 규칙 문서의 이름이나 절 번호(예: `(Architecture 3. 플랫폼 추상화)`)를 쓰지 않는다. 문서 구성이 바뀌면 주석이 틀어진다. 주석만으로 이해되도록 필요한 내용을 직접 쓴다.

  ```cpp
  // 금지: 코드를 그대로 풀어 씀
  // 카운트를 1 증가시킨다
  ++frameCount;

  // 권장: 이유를 설명함
  // GPU가 이전 프레임 버퍼를 아직 읽고 있을 수 있어 펜스를 기다린다
  WaitForFence(previousFrame);

  // 허용: 흐름 구분
  // 1. 가시성 판정
  ...
  // 2. 정렬
  ...
  ```

- **태그** — `태그(이름): 내용` 형식으로 쓴다(`// TODO(seon): 밉맵 생성 추가`). `NOTE`는 담당자가 없으므로 이름을 생략한다. 태그는 다음 네 가지만 쓴다.

  | 태그 | 용도 |
  |---|---|
  | `TODO` | 해야 할 일 |
  | `FIXME` | 알려진 버그 |
  | `NOTE` | 주의사항 |
  | `HACK` | 임시 처리 (제거 조건을 함께 적는다) |

- **주석 종류** — 설명하는 범위에 따라 문법을 구분한다.

  | 범위 | 문법 | 위치 |
  |---|---|---|
  | 클래스 / 구조체 등 큰 단위 | 구분선(`/` 80개) 사이에 `//` | 타입 선언 바로 위 |
  | 함수 | `/** */` | 헤더의 선언 바로 위 (`.cpp` 정의에는 반복하지 않는다) |
  | 함수 본문 안의 코드 | `//` | 설명할 코드 바로 위 |

  ```cpp
  ////////////////////////////////////////////////////////////////////////////////
  // 프레임 렌더링 전체를 관리한다.
  // 렌더 스레드에서만 생성하고 파괴한다.
  ////////////////////////////////////////////////////////////////////////////////
  class FRenderer
  {
  public:
    /**
     * 한 프레임을 렌더링한다.
     * BeginFrame()을 먼저 호출하지 않으면 assert가 발생한다.
     */
    void Render();

  private:
    void SubmitCommands()
    {
        // 드라이버가 빈 커맨드 리스트 제출을 에러로 처리한다
        if (commandCount == 0) { return; }
    }
  };
  ```

  - `/** */`는 여는 줄과 닫는 줄을 따로 두고 가운데 줄은 ` * `로 시작한다. 한 줄로 끝나는 설명은 `/** 설명 */`처럼 한 줄로 쓸 수 있다.
  - 코드를 임시로 막을 때는 `/* */`를 쓰지 않고 `//`를 쓴다. `/* */`는 중첩되지 않아서, 함수 설명이 있는 코드를 감싸면 깨진다.

### 1.4 API Documentation `중요`

- **대상** — 콘텐츠 코드가 사용하는 공개 API(public 타입과 함수)에는 문서 주석(`/** */`)을 반드시 단다. 단, `GetWidth()`처럼 이름만으로 충분한 함수는 생략할 수 있다. private 함수나 내부 구현에는 필요할 때만 단다.
- **형식** — 기본은 문장으로 설명한다. 매개변수나 반환값에 이름만으로 알 수 없는 조건(범위, `nullptr` 허용 여부, 실패 시 반환값 등)이 있을 때만 `@param` / `@return` 태그를 쓴다.

  ```cpp
  /** 현재 프레임 번호를 반환한다. */
  uint64 GetFrameIndex() const;

  /**
   * 텍스처를 이름으로 찾는다.
   *
   * @param name 대소문자를 구분한다.
   * @return 없으면 nullptr.
   */
  FTexture* FindTexture(const FString& name) const;
  ```

---

## 2장. Files & Code Layout

### 2.1 파일 구성 `필수`

- **확장자** — 헤더는 `.h`, 소스는 `.cpp`만 쓴다. `.inl`은 쓰지 않는다. 템플릿 구현은 헤더에(3.10 Template 참고), 긴 inline 함수는 `.cpp`에 둔다(3.2 함수 작성 참고).
- **파일 이름** — 파일에 담긴 주 타입의 이름에서 접두사를 뺀 이름을 쓴다.

  ```text
  Renderer.h        → class FRenderer
  RingBuffer.h      → class TRingBuffer
  RenderDevice.h    → class IRenderDevice
  TextureFormat.h   → enum class ETextureFormat
  ```

  - 예외: 대소문자를 무시했을 때 C / C++ 표준 헤더나 Windows SDK 헤더와 같아지는 이름(`String.h`, `Math.h`, `Memory.h` 등)이면 엔진 이름 `Seon`을 앞에 붙인다(`SeonString.h` → `FString`). Windows는 파일 이름의 대소문자를 구분하지 않아서, 표준 헤더를 찾을 때 엔진 헤더가 대신 잡힐 수 있다.
  - 렌더러 계약 예외: `IRenderer`는 `RendererInterface.h`, 기본 구현 `FRenderer`는 `Renderer.h`에 둔다. Public / Private include 경로에서 같은 파일 이름이 겹치지 않도록 계약과 구현을 구분한다. 다른 인터페이스에 일괄 적용하는 규칙은 아니다.

- **파일 단위** — 파일마다 파일 이름과 같은 주 타입을 하나 둔다. 그 타입에서만 쓰는 작은 보조 타입(Desc 구조체, enum 등)은 같은 파일에 둘 수 있다(예: `Texture.h`에 `FTextureDesc`, `ETextureFormat`, `FTexture`).
- **Public / Private 분리** — 엔진 모듈마다 `Public`과 `Private` 폴더를 둔다.
  - `Public`: 다른 모듈과 콘텐츠 코드가 include할 수 있는 헤더. 1.4 API Documentation의 "공개 API"는 이 폴더의 헤더를 뜻한다.
  - `Private`: `.cpp` 전부와 모듈 내부 전용 헤더.
  - 새 헤더는 기본적으로 `Private`에 두고, 공개가 필요해지면 `Public`으로 옮긴다.
  - 콘텐츠 프로젝트의 include 경로에는 엔진 모듈의 `Public` 폴더만 추가한다.

  ```text
  Renderer/
  ├─ Public/
  │  └─ Renderer.h
  └─ Private/
     ├─ Renderer.cpp
     └─ RenderQueue.h
  ```

- **게임 모듈은 나누지 않음** — 게임 모듈은 다른 모듈이 include하지 않으므로 `Public` / `Private` 없이 모듈 폴더에 바로 파일을 둔다. 하위 폴더는 기능별로 나눈다. 다른 모듈이 include하게 된 게임 모듈만 엔진 모듈처럼 나눈다.

  ```text
  SampleGame/
  ├─ SampleGame.cpp
  └─ Player/
     ├─ PlayerCharacter.h
     └─ PlayerCharacter.cpp
  ```

- **디렉터리 구조** — 모듈마다 폴더를 두고, 엔진 모듈은 그 안에 `Public` / `Private`를 둔다. 모듈 구성은 [Architecture](../../../Docs/Architecture.md) 1장을 따른다.

### 2.2 Header 규칙 `필수`

- **중복 include 방지** — 모든 헤더의 첫 줄에 `#pragma once`를 쓴다. include guard는 쓰지 않는다.
- **include 순서** — 아래 순서로 그룹을 나누고, 그룹 사이에 빈 줄 1개를 둔다. 그룹 안에서는 알파벳순으로 정렬한다.
  1. 자기 헤더 (`Renderer.cpp`라면 `Renderer.h`)
  2. 엔진 헤더
  3. 서드파티 헤더
  4. 표준 라이브러리 헤더

- **include 경로** — 모듈의 `Public` 폴더를 기준으로 경로를 쓴다. 모듈 내부의 `.cpp`와 Private 헤더에서는 자기 모듈의 `Private` 폴더도 기준으로 쓸 수 있다. 게임 모듈은 모듈 폴더가 기준이다(`#include "Player/PlayerCharacter.h"`).
  - 엔진 헤더는 `""`, 표준 라이브러리와 서드파티 헤더는 `<>`로 include한다.
  - `../` 같은 상대 경로는 쓰지 않는다.
  - 헤더 이름은 엔진 전체에서 고유해야 한다. (다른 모듈에 같은 이름의 헤더를 두지 않는다)

  ```cpp
  #include "Renderer.h"          // Renderer/Public/Renderer.h
  #include "RHI/RHIDevice.h"     // Renderer/Public/RHI/RHIDevice.h
  #include "Math/Vector3.h"      // Core/Public/Math/Vector3.h

  #include <vector>

  // 금지
  #include "../Public/Renderer.h"
  ```

- **전방 선언** — 헤더에서 포인터, 레퍼런스, 함수 선언에만 쓰는 타입은 include하지 않고 전방 선언한다. 값 멤버, 부모 클래스, 헤더 안의 inline 함수에서 멤버를 사용하는 타입은 include한다. 전방 선언은 클래스처럼 전방 선언할 수 있는 타입에만 적용한다. 타입 별칭(`FString`, `TArray` 등)은 포인터나 레퍼런스로만 써도 별칭을 제공하는 헤더를 include한다.
- **직접 쓰는 것만 include** — 파일은 자기가 직접 쓰는 헤더를 include한다. 다른 헤더를 통해 간접적으로 들어오는 헤더에 기대지 않는다. `.cpp`에서만 필요한 include는 헤더가 아니라 `.cpp`에 둔다.
- **`using namespace`** — 헤더에서는 쓰지 않는다. `.cpp`에서도 `using namespace std;`는 쓰지 않는다(Windows 헤더의 `byte`와 `std::byte`가 충돌한다). `.cpp`에서 `using std::vector;`처럼 이름 하나씩 가져오는 것은 허용한다.

### 2.3 Class 구성 `필수`

- **선언 순서** — 접근 지정자는 `public` → `protected` → `private` 순서로 각각 한 번씩만 쓴다. 각 구역 안에서는 타입(`using`, 중첩 타입) → 생성자 / 소멸자 → 함수 → 변수 순서로 선언한다.

  ```cpp
  class FRenderer
  {
  public:
    using Callback = void(*)();

    FRenderer();
    ~FRenderer();

    void Render();

  protected:
    virtual void OnResize();

  private:
    void CreateDevice();

    FRenderDevice* device = nullptr;
  };
  ```

- **`friend`** — 멤버가 아니라 접근 지정자의 영향을 받지 않으므로, 접근 지정자보다 앞인 클래스 맨 위에 둔다. 왜 필요한지 주석을 단다.

  ```cpp
  class FWindow
  {
    // 메시지 처리 보조 구조체가 창 생성 / 파괴 중에 nativeHandle을 바꾼다
    friend struct SE::Private::FWindowsWindowProc;

  public:
    ...
  };
  ```

- **멤버 변수 접근** — class의 멤버 변수는 `private`을 원칙으로 한다. 자식 클래스에 필요하면 `protected` 함수로 제공한다. 단, 다음은 `public`으로 둘 수 있다.
  - 외부에서 구독하는 이벤트(델리게이트). 소유 클래스만 발생시킬 수 있는 이벤트 타입을 쓴다. (Deferred Tasks 참고)
  - `static constexpr` 상수
- **struct vs class** — 모든 멤버가 public이고 어떤 값 조합이든 유효한 데이터 묶음(불변 조건 없음)은 struct로 쓴다. 간단한 생성자나 계산 함수는 둘 수 있지만 가상 함수는 두지 않는다. 그 외에는 class로 쓴다.

  ```cpp
  struct FVector3
  {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float Length() const;
  };
  ```

- **특수 멤버 함수** — 소멸자, 복사 / 이동 생성자, 복사 / 이동 대입 연산자는 가능하면 직접 정의하지 않는다(Rule of 0). 하나라도 직접 정의하면 다섯 가지를 모두 정의하거나 `= default` / `= delete`로 명시한다(Rule of 5). GPU 리소스처럼 복사하면 안 되는 클래스는 복사를 `= delete`로 막는다.

  ```cpp
  class FTexture
  {
  public:
    explicit FTexture(const FTextureDesc& desc);
    ~FTexture();

    FTexture(const FTexture&) = delete;
    FTexture& operator=(const FTexture&) = delete;
    FTexture(FTexture&&) noexcept;
    FTexture& operator=(FTexture&&) noexcept;
  };
  ```

- **멤버 초기화** — 모든 멤버 변수는 선언하는 자리에서 기본값을 준다. 생성자 인자로 받는 값은 초기화 리스트에서 넣는다. 생성자 본문에서 대입하는 방식은 쓰지 않는다(1.2 Naming의 `FMesh` 예시 참고).
- **virtual / override / final** — 함수에는 `virtual`, `override`, `final` 중 하나만 쓴다.
  - 부모 클래스에서 처음 선언하는 가상 함수는 `virtual`
  - 재정의는 `override` (`virtual`을 반복하지 않는다)
  - 더 이상 재정의하면 안 되는 재정의는 `override` 대신 `final`
  - 상속을 고려하지 않은 클래스에는 클래스에 `final`을 붙인다.
  - 상속되는 클래스의 소멸자는 `virtual`로 둔다.

  ```cpp
  class IRenderDevice
  {
  public:
    virtual ~IRenderDevice() = default;
    virtual void Present() = 0;
  };

  class FD3D12RenderDevice final : public IRenderDevice
  {
  public:
    void Present() override;
  };
  ```

### 2.4 Namespace `중요`

- **공개 타입은 전역** — 엔진 사용자(콘텐츠 코드)가 쓰는 타입은 namespace 없이 전역에 둔다. 이름 충돌은 타입 접두사(1.2 Naming 참고)로 막는다.
- **내부 구현은 `SE::Private`** — 헤더에 있어야 하지만 외부에 노출하지 않을 구현 세부 사항(템플릿 구현 보조 함수 등)은 `SE::Private`에 넣는다.
- **표기** — 중첩은 `namespace A::B`로 쓰고(`namespace SE::Private`), namespace 안쪽은 들여쓴다.
- **파일 내부 전용** — `.cpp` 안에서만 쓰는 함수, 변수, 타입은 익명 namespace에 넣는다. `static`으로 숨기지 않는다.
- **유틸리티 함수 묶음** — 콘텐츠 코드가 쓰는 함수 묶음은 namespace나 전역 함수 대신 static 함수를 모은 struct로 둔다.

  ```cpp
  struct FMath
  {
    static float Lerp(float a, float b, float t);
    static float Clamp(float value, float minValue, float maxValue);
  };

  float x = FMath::Lerp(0.0f, 1.0f, 0.5f);
  ```

- **`using namespace`** — 2.2 Header 규칙을 따른다.

---

## 3장. C++ Language Rules

### 3.1 C++ 표준 및 기능 정책 `중요`

- **표준** — C++20을 쓴다.
- **Exception** — 엔진 코드에서 `throw` / `try` / `catch`를 쓰지 않는다. 에러는 반환값으로 처리한다(5장 Error Handling & Diagnostics 참고). 컴파일러 옵션으로도 꺼져 있다(6.1 빌드 / 컴파일러 설정 참고).
- **RTTI** — 컴파일러 옵션으로 켜며 `dynamic_cast`와 `typeid` 사용을 허용한다. 실제 타입 식별이 필요할 때 사용하고, 공통 동작은 가상 함수로 표현한다. 문제가 확인되거나 필요성이 없어지면 비활성화를 재검토한다(6.1 빌드 / 컴파일러 설정 참고).
- **제한 기능** — 아래 기능은 쓰지 않는다. 그 외의 C++20 기능은 이 문서의 다른 규칙을 따르는 한 자유롭게 쓴다.

  | 기능 | 이유 |
  |---|---|
  | Modules (`import`) | IntelliSense, 서드파티, 빌드 도구 지원이 아직 불안정하고, 헤더 기반 구조(2.1 파일 구성, 2.2 Header 규칙 참고)와 맞지 않는다. |
  | Coroutines | 코루틴 프레임 할당과 수명 관리가 까다롭다. |

### 3.2 함수 작성 `필수`

- **길이 / 매개변수 개수** — 강제하지 않는다. 함수가 한 화면(약 50줄)을 넘으면 나눌 수 있는지, 매개변수가 5개를 넘으면 Desc struct로 묶을 수 있는지(`CreateTexture(const FTextureDesc& desc)`) 검토한다.
- **인자 전달** — 기본형, enum, 포인터, 16바이트 이하의 작은 struct는 값으로 받는다. 그 외에는 `const&`로 받는다. 소유권을 넘겨받을 때는 값으로 받고 `std::move`한다.

  ```cpp
  void SetPosition(FVector3 newPosition);
  void SetName(const FString& newName);
  void Draw(const FMesh& mesh);
  void SetTexture(TUniquePtr<FTexture> newTexture);
  ```

- **반환값** — 결과는 반환값으로 돌려준다. 여러 값은 struct로 묶는다. 큰 버퍼를 재사용하는 경우처럼 꼭 필요할 때만 out 매개변수를 쓰고, 이름에 `out`을 붙인다. bool out 매개변수는 `bOut`을 붙인다.

  ```cpp
  void CollectVisibleObjects(TArray<FRenderObject*>& outObjects) const;
  ```

- **`[[nodiscard]]`** — 반환값을 버리면 버그가 되는 함수에만 붙인다. 성공 / 실패를 돌려주는 함수, 새로 만든 리소스를 돌려주는 함수가 해당한다. getter에는 붙이지 않는다.
- **`noexcept`** — 이동 생성자, 이동 대입 연산자, swap에만 붙인다. STL 컨테이너는 이동 생성자가 `noexcept`여야 재할당할 때 복사 대신 이동한다.
- **기본 인자** — 허용한다. 가상 함수에 기본 인자를 둘 때는 재정의에서도 같은 값을 쓴다. 기본값은 호출하는 포인터 타입 기준으로 정해지므로, 값이 다르면 부모의 값이 들어간다.
- **`inline`** — 헤더에 정의하는 비멤버 함수와 변수에만 붙인다(중복 정의 링크 에러 방지). 클래스 안에서 정의한 함수는 이미 inline이므로 붙이지 않는다. 헤더에는 짧은 함수만 정의하고 나머지는 `.cpp`에 둔다.

### 3.3 `const` / `constexpr` `중요`

- **멤버 함수** — 객체 상태를 바꾸지 않는 멤버 함수는 반드시 `const`로 만든다.
- **지역 변수 / 값 매개변수** — 바뀌지 않는 값에 `const`를 붙이는 것을 권장하되 강제하지 않는다. 값 매개변수의 `const`는 호출하는 쪽에 의미가 없으므로 헤더 선언에는 붙이지 않는다.
- **반환 타입** — 값으로 반환할 때는 `const`를 붙이지 않는다. `const` 값은 이동할 수 없어서 복사가 일어난다. `const&`와 `const*` 반환은 허용한다.
- **위치** — `const`는 타입 앞에 쓴다(`const FMesh&`, `const FTexture*`). 포인터 자체가 const면 `FTexture* const`로 쓴다.
- **`constexpr`** — 컴파일 타임에 값이 정해지는 상수는 `const` 대신 `constexpr`로 쓴다. 함수에는 상수 계산에 쓰이는 간단한 함수(수학 함수 등)에만 붙인다. `consteval`과 `constinit`은 필요할 때만 쓴다.

### 3.4 Pointer / Reference `필수`

- **`&`와 `*` 구분** — 값이 반드시 있어야 하면 `&`, 없을 수 있으면(`nullptr` 가능) `*`를 쓴다. 둘 다 소유권이 없다(4.1 메모리 소유권 참고).

  ```cpp
  void Draw(const FMesh& mesh);            // mesh는 항상 있음
  void SetParent(FTransform* newParent);   // nullptr이면 부모 없음
  ```

- **Nullable 표시** — `*` 자체가 "없을 수 있음"을 뜻하므로 추가로 표시하지 않는다. 포인터가 아닌 값이 없을 수 있을 때는 `std::optional`을 쓴다.
- **`nullptr`** — 널 포인터는 항상 `nullptr`로 쓴다. `NULL`과 `0`은 쓰지 않는다.
- **레퍼런스 멤버 금지** — 멤버 변수는 레퍼런스로 두지 않고 포인터로 둔다. 레퍼런스 멤버가 있으면 대입 연산자가 삭제되어 컨테이너에 넣거나 다시 대입할 수 없다. 대상이 반드시 있어야 하면 생성자에서 레퍼런스로 받아 주소를 저장한다.

  ```cpp
  class FRenderPass
  {
  public:
    explicit FRenderPass(FRenderDevice& targetDevice)
        : device(&targetDevice)
    {
    }

  private:
    FRenderDevice* device = nullptr;
  };
  ```

### 3.5 `auto` `중요`

- **원칙** — `auto`는 쓰지 않고 타입을 명시한다. 다음 경우만 예외로 허용한다.
  - 람다를 변수에 담을 때 (람다 타입은 코드로 쓸 수 없다)
  - 반복자 타입이 너무 길어 읽기 어려울 때
  - 템플릿 코드에서 식의 타입을 알기 어려울 때
  - structured binding (`auto`로만 쓸 수 있다)

  ```cpp
  TUniquePtr<FTexture> texture = MakeUnique<FTexture>(desc);   // 타입 명시

  auto it = textureMap.find(name);
  for (const auto& [name, texture] : textureMap)
  {
    ...
  }
  ```

### 3.6 Cast `중요`

- **C 스타일 캐스트 금지** — `(T)value`와 `T(value)` 형태의 캐스트는 쓰지 않고 C++ 캐스트를 쓴다. 다형 객체의 실제 타입을 확인하는 변환에는 `dynamic_cast`를 사용할 수 있다. 포인터 변환 실패 시 반환되는 `nullptr`를 처리한다. 실패 가능한 참조 dynamic_cast는 예외를 요구하므로 현재 예외 금지 정책에서 사용하지 않는다.
- **축소 변환** — `int64 → int32`, `float → int`, `size_t → uint32`처럼 값이 잘릴 수 있는 변환은 암묵적으로 하지 않고 `static_cast`로 명시한다. 컨테이너 크기(`size_t`)를 `uint32`에 넣을 때가 가장 흔하다(`static_cast<uint32>(meshes.size())`).
- **`const_cast`** — const를 지키지 않는 외부 API에 값을 넘길 때만 쓴다. 엔진 코드 안에서는 const 설계를 고쳐서 해결한다.
- **`reinterpret_cast`** — 서로 관계없는 포인터 타입 사이의 변환, 포인터와 정수 사이의 변환처럼 저수준 코드에서만 쓴다. `void*`에서 원래 타입으로 돌아갈 때는 `static_cast`를 쓴다.
- **`std::bit_cast`** — 값의 비트를 다른 타입으로 해석할 때 쓴다. 포인터를 `reinterpret_cast`해서 읽는 방식은 정의되지 않은 동작이므로 쓰지 않는다.

  ```cpp
  FVertex* vertices = static_cast<FVertex*>(mapped);   // void* → 원래 타입

  uintptr_t address = reinterpret_cast<uintptr_t>(pointer);

  uint32 bits = std::bit_cast<uint32>(1.0f);

  // 금지
  uint32 bits = *reinterpret_cast<uint32*>(&value);
  ```

### 3.7 Enum `중요`

- **`enum class`만 사용** — 일반 `enum`은 쓰지 않는다.
- **내부 타입 명시** — 항상 내부 타입을 명시하고, 값이 들어가는 가장 작은 타입(보통 `uint8`)을 쓴다. 전방 선언할 때도 같은 타입을 쓴다(`enum class ETextureFormat : uint8;`).
- **비트 플래그** — 이름은 `Flags`로 끝낸다. `None = 0`을 두고 값은 `1 << n`으로 쓴다. 비트 연산자는 `SE_ENUM_CLASS_FLAGS` 매크로로 정의한다.

  ```cpp
  enum class ETextureUsageFlags : uint8
  {
    None = 0,
    ShaderRead = 1 << 0,
    RenderTarget = 1 << 1,
    DepthStencil = 1 << 2
  };
  SE_ENUM_CLASS_FLAGS(ETextureUsageFlags)

  ETextureUsageFlags usage = ETextureUsageFlags::ShaderRead | ETextureUsageFlags::RenderTarget;
  ```

- **특수 값** — 값의 개수가 필요하면 마지막에 `Count`를 둔다. "없음"이 필요하면 맨 앞에 `None`을 둔다.

### 3.8 상수 / Literal `중요`

- **매직 넘버** — 의미가 있는 숫자는 `constexpr` 상수로 이름을 붙인다. `0`, `1`, `-1`, 절반을 뜻하는 `0.5f`처럼 뜻이 자명한 값은 그대로 쓸 수 있다.
- **매직 문자열** — 에셋 경로, 셰이더 진입점 이름, 설정 키처럼 두 곳 이상에서 쓰거나 오타가 런타임에야 드러나는 문자열은 상수로 만든다. 로그 메시지처럼 한 번만 쓰는 문자열은 그대로 쓴다.
- **리터럴** — float 값은 `1.0f`처럼 소수점과 `f`를 모두 쓴다(`1.0`은 double, `1.f`는 쓰지 않음). 접미사는 소문자로 쓴다. 큰 숫자는 `'`로 자리를 나눌 수 있다.

### 3.9 Macro `중요`

- **이름** — `SE_` + UPPER_SNAKE_CASE (1.2 Naming 참고).
- **사용 범위** — 매크로로만 할 수 있는 일에만 쓴다.
  - 플랫폼 / 빌드 구성 분기
  - 파일 이름과 줄 번호가 필요한 assert, 로그
  - 반복 코드 생성 (`SE_ENUM_CLASS_FLAGS`, 리플렉션 등록 등)
  - DLL export (`SE_API`)

  상수는 `constexpr`, 함수는 inline 함수나 템플릿으로 쓴다(`#define SE_MAX_LIGHTS 16` 같은 매크로 상수는 쓰지 않는다).

- **분기 매크로** — 항상 `0` 또는 `1`로 정의하고 `#if`로 검사한다. 꺼진 기능도 정의를 생략하지 않고 `0`으로 둔다(`#define SE_WITH_EDITOR 0` → `#if SE_WITH_EDITOR`). `#ifdef`는 쓰지 않는다. 예외로, 헤더 경로 생성용 이름 매크로(`SE_PLATFORM_HEADER_NAME`)처럼 0 / 1이 아닌 필수 입력이 정의되었는지 확인할 때는 `#if !defined(MACRO)` → `#error`를 쓸 수 있다. 이름은 종류에 따라 접두사를 나눈다.

  | 접두사 | 용도 | 예시 |
  |---|---|---|
  | `SE_PLATFORM_` | 플랫폼 | `SE_PLATFORM_WINDOWS` |
  | `SE_BUILD_` | 빌드 구성 | `SE_BUILD_DEBUG`, `SE_BUILD_RELEASE` |
  | `SE_WITH_` | 켜고 끌 수 있는 기능 | `SE_WITH_EDITOR`, `SE_WITH_PROFILER` |

- **작성 규칙**
  - 매크로 인자는 괄호로 감싼다.
  - 여러 문장인 매크로는 `do { ... } while (0)`로 감싼다.
  - 한 파일에서만 쓰는 매크로는 다 쓴 뒤 `#undef`한다.

  ```cpp
  #define SE_ASSERT(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            SE::Private::AssertFailed(#condition, __FILE__, __LINE__); \
        } \
    } while (0)
  ```

### 3.10 Template `중요`

- **구현 위치** — 템플릿 구현은 길어도 헤더에 둔다. `.inl`로 분리하지 않는다.
- **Concepts** — 템플릿 인자에 조건이 필요하면 `std::enable_if`(SFINAE) 대신 concept과 `requires`를 쓴다.

  ```cpp
  template<typename T>
    requires std::is_arithmetic_v<T>
  T Clamp(T value, T minValue, T maxValue);

  // 금지
  template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
  T Clamp(T value, T minValue, T maxValue);
  ```

- **Concept 이름** — 직접 만드는 concept에는 `C` 접두사를 붙인다.

  ```cpp
  template<typename T>
  concept CHashable = requires(const T& value)
  {
    { GetTypeHash(value) } -> std::convertible_to<uint32>;
  };

  template<CHashable KeyType, typename ValueType>
  class TSparseMap;
  ```

- **메타프로그래밍**
  - 일반 코드에서는 타입 검사(`std::is_...`), `if constexpr`, concept까지만 쓴다. 템플릿의 재귀나 특수화로 로직을 짜지 않는다. 컴파일 시간 계산은 `constexpr` 함수로 한다.
  - 기반 라이브러리(컨테이너, 델리게이트, 튜플 등)에서는 필요하면 복잡한 메타프로그래밍을 쓸 수 있다. 단, 무엇을 하는 코드인지 주석으로 설명한다.
  - 표준 라이브러리에 이미 있는 기능(`std::tuple`, `std::variant` 등)은 직접 만들기 전에 그대로 쓰는 것을 먼저 검토한다.
  - `template<typename... Args>` 같은 가변 인자 템플릿과 인자 전달(`Args&&...`)은 제한 대상이 아니다.

### 3.11 Lambda `중요`

- **캡처** — 캡처할 변수를 항상 명시한다. `[&]`, `[=]` 같은 자동 캡처는 쓰지 않는다. 저장해 두었다가 나중에 실행하는 람다는 캡처한 대상(`this`, 레퍼런스)이 실행 시점까지 살아 있는지 확인한다.

  ```cpp
  auto isVisible = [&camera, maxDistance](const FRenderObject* object)   // 캡처 대상을 명시 ([&]나 [=]는 쓰지 않음)
  {
    ...
  };
  ```

- **모양** — 람다는 본문이 짧아도 항상 Allman으로 쓴다. 한 줄로 쓰지 않는다(clang-format 설정과 같다).

  ```cpp
  std::sort(lights.begin(), lights.end(), [](const FLight& a, const FLight& b)
  {
    return a.priority > b.priority;
  });
  ```

- **길이** — 람다는 짧은 로직에만 쓴다. 대략 10줄을 넘거나 여러 곳에서 쓰면 멤버 함수나 익명 namespace 함수로 빼는 것을 검토한다.

### 3.12 STL 사용 정책 `중요`

- **컨테이너 / 문자열** — STL을 쓰되, 엔진 별칭으로 감싸서 쓴다. 코드에서는 `std::` 타입 대신 별칭을 쓴다.
  - 멤버 함수는 STL 이름(`push_back`, `size` 등)을 그대로 쓴다. 이 부분은 1.2 Naming의 예외다.

  ```cpp
  template<typename T>
  using TArray = std::vector<T>;

  template<typename KeyType, typename ValueType>
  using THashMap = std::unordered_map<KeyType, ValueType>;

  using FString = std::string;

  TArray<FMesh*> meshes;
  meshes.push_back(mesh);
  ```

- **정수 타입** — `int`, `unsigned`, `long` 대신 크기를 명시한 타입(`int8` ~ `int64`, `uint8` ~ `uint64`)을 쓴다. 별칭은 `<cstdint>`를 거치지 않고 기본 타입(`signed int` 등)에 직접 정의하며, 크기는 `static_assert`로 확인한다(`Core/Public/CoreTypes.h`). 표준 라이브러리가 쓰는 `size_t`는 예외로 허용한다.
- **문자열 인코딩** — 엔진 안의 문자열은 모두 UTF-8(`FString`)로 다룬다. Windows API를 호출하는 Platform 코드에서만 UTF-16(`std::wstring`)으로 변환한다. 변환은 `Platform/Windows/WindowsString.h`의 `FWindowsString`으로 한다.
  - `FWindowsString::UTF8ToWide`는 깨진 UTF-8 바이트를 U+FFFD로 바꾸고 계속한다. 변환 자체가 실패하면 `std::nullopt`를 돌려주므로 값이 있는지 확인하고 쓴다.
  - `FWindowsString::WideToUTF8`은 Windows API가 돌려준 UTF-16을 엔진 문자열로 받을 때 쓴다. 깨진 UTF-16(짝이 없는 서로게이트)을 만났을 때 할 일은 부르는 쪽이 `EInvalidCharacter`로 고른다. 사람이 읽을 글자는 `Replace`(기본, U+FFFD로 바꾸고 계속), 경로처럼 OS에 다시 넘길 값은 `Fail`(`std::nullopt`)이다. Windows는 파일 이름에 깨진 UTF-16을 허용하므로, 바꿔서 쓰면 다른 파일을 가리킨다.
  - 엔진의 UTF-8 경로를 UTF-16으로 바꿀 때 깨진 바이트를 어떻게 다룰지는 정하지 않았다(Deferred Tasks "문자열 변환").

  ```cpp
  FString message = "창 생성 완료";

  // Platform/Windows 내부
  if (const std::optional<std::wstring> wideMessage = FWindowsString::UTF8ToWide(message))
  {
    OutputDebugStringW(wideMessage->c_str());
  }

  // 경로는 OS에 다시 넘기므로 깨진 글자가 있으면 실패시킨다
  const std::optional<FString> path = FWindowsString::WideToUTF8(modulePath, EInvalidCharacter::Fail);
  ```

---

## 4장. Memory & Lifetime

### 4.1 메모리 소유권 `매우 중요`

- **스마트 포인터 별칭** — 3.12 STL 사용 정책과 같이 STL 스마트 포인터를 엔진 별칭으로 감싸서 쓴다.

  ```cpp
  template<typename T>
  using TUniquePtr = std::unique_ptr<T>;   // TSharedPtr, TWeakPtr도 같은 방식

  // MakeUnique / MakeShared는 std::make_unique / std::make_shared를 호출하는 함수로 정의한다.
  ```

- **소유 / 비소유** — 객체를 소유하는 쪽만 `TUniquePtr`로 가진다. 나머지는 일반 포인터나 레퍼런스로 빌려 쓴다. 일반 포인터와 레퍼런스는 항상 비소유이며, 가리키는 객체를 지우지 않는다.

  ```cpp
  class FRenderer
  {
  private:
    TUniquePtr<FRenderDevice> device;   // 소유
  };

  class FRenderPass
  {
  private:
    FRenderDevice* device = nullptr;    // 비소유
  };
  ```

- **`new` / `delete` 금지** — 객체는 `MakeUnique`(공유가 필요하면 `MakeShared`)로 만든다. 컨테이너나 할당자 같은 기반 코드의 placement new만 예외로 허용한다. D3D11 같은 COM 객체는 `ComPtr`로 관리한다.
- **`TSharedPtr`** — 여러 시스템이 함께 소유하고 누가 마지막인지 정할 수 없는 경우에만 쓴다. 편의를 위해 쓰지 않는다. 공유 객체를 관찰만 할 때는 `TWeakPtr`를 쓴다.
- **핸들** — 텍스처, 메시, 게임 오브젝트처럼 여러 곳에서 참조하고 수명이 따로 관리되는 객체는 포인터 대신 핸들(인덱스 + 세대 번호)로 가리키는 것을 원칙으로 한다.

  ```cpp
  FTextureHandle texture = resourceManager.LoadTexture(path);

  if (FTexture* resolved = resourceManager.Resolve(texture))
  {
    ...
  }
  ```

### 4.2 객체 생명주기 `매우 중요`

- **생성 / 파괴 책임** — 객체는 소유자(`TUniquePtr`를 가진 쪽)가 만들고 파괴한다. 소유 관계는 나무 모양이 되며, 실행 엔진 객체는 FEngineLoop가 소유하며, 엔진 내부 객체는 각 담당 소유자가 만들고 파괴한다.

  ```text
  UEngine
   └─ FRenderer              (Engine이 소유)
       ├─ FRenderDevice      (Renderer가 소유)
       └─ FShaderCache       (Renderer가 소유)
  ```

- **초기화 방식** — 무거운 객체는 2단계로 초기화하고, 함수 이름과 검사 방식을 통일한다.
  - 서브시스템과 무거운 객체(GPU 리소스 등): 생성자에서는 실패하지 않는 가벼운 작업만 한다. 실제 초기화는 `Initialize()`, 정리는 `Shutdown()`에서 한다.
  - D3D11 장치 준비의 좁은 예외: `void InitD3DDevice()`는 Unreal 대응 이름과 역할을 사용한다. 현재 본문과 FRenderer의 직접 호출 연결을 구현했다. 창 출력은 Viewport가 준비하며 기존 Device Init은 제거했다. 정상 반환은 장치 준비 완료, 최종 생성 실패는 부분 자원 정리 후 Fatal 계약이며 다른 초기화 API의 반환형을 일반화하지 않는다.
  - 작은 보조 타입(락 가드, 타이머 등): 생성자와 소멸자로 처리한다(RAII).
  - 이름은 `Initialize()` / `Shutdown()` 한 쌍만 쓴다(`Init`, `Startup`, `Deinitialize` 등을 섞지 않는다).
  - 엔진 실행 기반 예외: UEngine의 virtual void Init(IEngineLoop*) / virtual void Start() / virtual void Tick(float, bool) / virtual void PreExit()는 Unreal 대응 역할의 이름을 사용한다. 현재 Init은 비소유 루프 연결, Start는 빈 실행 시작 통로, Tick은 순수 가상, PreExit는 빈 기반 구현이다. UObject 생성 / 파괴와 실행 종료를 구분하고 기반 소멸자는 PreExit를 자동 호출하지 않는다. 일반 서브시스템의 Initialize / Shutdown 규칙을 바꾸지 않으며 UEngine 기반 자체의 상태 / 호출 순서 검사는 아직 없다. UGameEngine은 기존 세션이 있는 Init을 Fatal로 거부하고 PreExit는 준비 전 / 반복 호출을 허용한다. FEngineLoop는 int32 Init / void Exit를 사용하며 정상 Init은 0, 기존 엔진 Init은 Fatal이다. Exit는 PreExit 후 소유 엔진을 파괴하고 준비 전 / 반복 호출을 허용한다. 소멸자는 Exit를 자동 호출하지 않는다.
  - 게임 세션 훅 예외: `UGameInstance`의 `virtual void Init()` / `virtual void Shutdown()`은 Unreal과 같은 이름·역할로 둔다(기본 준비 / 조회 / 정리 구현 완료·게임 실행 연결 전). 기본 Init은 빈 훅이고 Shutdown은 비소유 연결을 해제한다. 성공 / 실패를 반환하는 자원 준비 API와 구분한다. 실패 가능한 실행 준비는 별도의 결과 전달 경로로 처리한다. 초기 컨텍스트 / 빈 월드 준비는 구현했으며 중복 준비는 Fatal로 처리한다. 맵 / GPU 등 실패 가능한 실행 준비와 부분 실패 정리는 후속이다([Architecture](../../../Docs/Architecture.md) 2장). 같은 클래스의 일반 실행 준비 진입점은 non-virtual `void InitializeStandalone()`으로 두고 준비 절차 안에서 `Init()`을 호출한다(초기 단일 컨텍스트 / 빈 월드 준비 구현 완료, UGameEngine Init / PreExit 호출 연결 구현 완료, FEngineLoop 준비 / 종료 호출 연결 완료, 실제 진입점 / 프레임은 후속). 다른 서브시스템·GPU 자원의 이름 규칙을 바꾸는 예외는 아니다.
  - `Initialize()` 전에 다른 함수를 호출하거나, `Shutdown()` 없이 소멸되면 assert로 잡는다(5.2 Assert 참고).

  ```cpp
  class FTexture
  {
  public:
    FTexture() = default;
    ~FTexture();                  // Shutdown()을 호출하지 않았으면 assert

    [[nodiscard]] bool Initialize(const FTextureDesc& desc);
    void Shutdown();

    bool IsInitialized() const { return bInitialized; }

  private:
    bool bInitialized = false;
  };
  ```

- **서브시스템 초기화 순서** — [Architecture](../../../Docs/Architecture.md) 2장을 따른다.
- **비소유 참조** — 비소유 포인터는 대상이 자신보다 오래 산다는 것이 보장될 때만 쓴다(부모, 소유자, 먼저 초기화된 서브시스템 등). 보장되지 않으면 핸들이나 `TWeakPtr`를 쓴다.

---

## 5장. Error Handling & Diagnostics

### 5.1 에러 처리 `매우 중요`

- **에러 전달** — 성공 / 실패는 `bool`(+ `[[nodiscard]]`)로, 없을 수 있는 값은 `std::optional`이나 nullptr로 돌려준다. 실패 원인은 로그로 남긴다. 호출하는 쪽이 원인에 따라 다르게 처리해야 하는 경우에만 에러 코드 enum을 돌려준다.

  ```cpp
  // 원인에 따라 처리가 달라야 할 때
  enum class EFileError : uint8
  {
    None,
    NotFound,
    AccessDenied,
    DiskFull
  };

  [[nodiscard]] EFileError SaveFile(const FString& path, const TArray<uint8>& data);
  ```

- **종류별 처리** — 에러를 세 가지로 나눠 처리한다.

  | 종류 | 예 | 처리 |
  |---|---|---|
  | 프로그래머 실수 | 잘못된 인자, 호출 순서 위반, 불변 조건 위반 | assert로 잡는다(5.2 Assert 참고) |
  | 외부 요인 | 파일 없음, 잘못된 에셋, 사용자 입력 오류 | 에러를 돌려주고 로그를 남긴다. 가능하면 대체물(기본 텍스처 등)로 계속 진행한다 |
  | 복구 불가 | 그래픽스 장치 생성 실패, 메모리 부족 | 치명 로그를 남기고 종료한다 |

### 5.2 Assert `매우 중요`

- **종류** — 네 가지를 둔다.

  | 매크로 | 실패하면 | 쓰는 곳 |
  |---|---|---|
  | `SE_ASSERT(condition)` | 중단 | 프로그래머 실수(5.1 에러 처리 참고) |
  | `SE_ASSERTF(condition, format, ...)` | 메시지와 함께 중단 | 원인 설명이 필요한 프로그래머 실수 |
  | `SE_VERIFY(condition)` | 중단 | 식이 반드시 실행되어야 하는 검사 |
  | `SE_ENSURE(condition)` | 로그를 남기고 계속 진행. 결과를 `bool`로 돌려준다 | 버그지만 안전하게 빠져나갈 수 있는 경우 |

  ```cpp
  SE_ASSERT(device != nullptr);
  SE_ASSERTF(index < count, "index {} out of range", index);

  SE_VERIFY(renderer->Initialize());

  if (!SE_ENSURE(texture != nullptr))
  {
    return;
  }
  ```

- **부작용 금지** — `SE_ASSERT` / `SE_ASSERTF`의 조건식에는 함수 호출이나 대입처럼 반드시 실행되어야 하는 코드를 넣지 않는다. Release에서는 식까지 제거된다. 실행되어야 하면서 결과도 검사해야 하면 `SE_VERIFY`를 쓴다.

  ```cpp
  // 금지: Release에서 Initialize가 호출되지 않음
  SE_ASSERT(renderer->Initialize());
  ```

- **빌드 구성별 동작**

  | 매크로 | Debug | Release (배포용) |
  |---|---|---|
  | `SE_ASSERT`, `SE_ASSERTF` | 검사 | 식까지 제거 |
  | `SE_VERIFY` | 검사 | 식은 실행, 검사만 제거 |
  | `SE_ENSURE` | 검사 | 식은 실행, 보고만 제거 |

- **실패 동작** — 파일 이름, 줄 번호, 조건식, 메시지를 치명 로그로 남긴다. 디버거가 연결되어 있으면 그 자리에서 멈추고(`__debugbreak`), 아니면 종료한다. `SE_ENSURE`는 로그와 디버거 중단만 하고 계속 진행하며, 같은 위치는 처음 한 번만 보고한다.

### 5.3 Logging `중요`

- **레벨** — 7단계를 쓴다.

  | 레벨 | 용도 |
  |---|---|
  | `Fatal` | 복구할 수 없는 오류. 로그를 남기고 종료한다 |
  | `Error` | 기능이 실패함 |
  | `Warning` | 동작은 하지만 문제가 있음 |
  | `Display` | 콘솔과 로그 파일에 모두 남길 주요 정보 |
  | `Log` | 로그 파일에만 남길 일반 정보 |
  | `Verbose` | 자세한 디버깅 정보. 카테고리에서 켰을 때만 출력 |
  | `VeryVerbose` | 매우 자세한 정보(매 프레임 출력 등). 카테고리에서 켰을 때만 출력 |

- **카테고리** — 모든 로그에 카테고리를 붙인다. 카테고리 이름은 `Log` + PascalCase로 쓴다(1.2 Naming의 타입 접두사 규칙의 예외다). 카테고리마다 출력할 레벨을 따로 정할 수 있다.

  ```cpp
  SE_DECLARE_LOG_CATEGORY(LogRenderer);

  SE_LOG(LogRenderer, Warning, "Texture {} not found", path);
  ```

- **메시지 형식** — `std::format`과 같은 `{}` 형식을 쓴다. 인자 타입을 컴파일 시간에 검사한다. `SE_ASSERTF`의 메시지도 같은 형식을 쓴다.
- **빌드 구성별 출력** — Debug에서는 모든 레벨을 남긴다. Release(배포용)에서는 `Error`와 `Fatal`만 남기고 나머지는 컴파일 단계에서 제거한다.

---

## 6장. Tooling & Enforcement

### 6.1 빌드 / 컴파일러 설정 `자동화`

- **플랫폼** — x64만 지원한다.
- **빌드 구성** — Debug(개발용)와 Release(배포용) 두 가지를 쓴다.
- **경고** — 경고 레벨 4(`/W4`)를 쓰고, 모든 구성에서 경고를 에러로 처리한다(`/WX`). 기본으로 꺼져 있는 경고 중 다음을 켠다.

  | 경고 | 내용 | 관련 규칙 |
  |---|---|---|
  | C4668 | 정의되지 않은 매크로를 `#if`에 사용 | 3.9 Macro |
  | C4265 | 가상 함수가 있는데 소멸자가 virtual이 아님 | 2.3 Class 구성 |

  C4458(매개변수가 멤버 이름을 가림, 1.2 Naming)은 `/W4`에 포함되어 있다.
- **외부 헤더** — `<>`로 include한 헤더(표준 라이브러리, Windows SDK, 서드파티)는 외부 헤더로 취급해 경고를 끈다. 엔진 헤더는 `""`로 include하므로(2.2 Header 규칙) 경고 대상에 남는다.
- **예외 / RTTI** — 예외는 컴파일러 옵션으로 끄고 RTTI는 켠다(`/GR`). 예외 사용 경고(C4530)는 에러로 처리되며 RTTI 사용은 허용한다(3.1 C++ 표준 및 기능 정책 참고).
- **빌드 매크로** — 구성마다 프로젝트 전처리기 정의로 `SE_PLATFORM_*`, `SE_BUILD_*`를 `0` / `1`로 정의한다(3.9 Macro 참고). 구성별 값은 [Project Settings](../ProjectSettings.md)에 있다.

### 6.2 자동화 도구 `자동화`

- **`.editorconfig`** — 인코딩, 줄바꿈, 들여쓰기를 에디터에 적용한다([Project Settings](../ProjectSettings.md) 참고).
- **`.clang-format`** — 1.1 Formatting 규칙을 자동으로 적용한다. Visual Studio에 내장된 clang-format은 파일에서 가장 가까운 폴더의 `.clang-format`을 읽는다. 커밋 전에 문서 서식(Ctrl+K, Ctrl+D)을 실행한다.
  - 저장소 루트 `.clang-format`: 엔진 코드의 서식이고, 게임 코드의 권장 기본값이다.
  - `Engine/ThirdParty/.clang-format`: 서드파티의 서식을 끈다.
  - 표현하지 못하는 규칙: "한 줄 표기는 early exit만"은 clang-format이 본문 종류를 구분하지 못한다. 대신 줄 길이 제한이 없어(`ColumnLimit: 0`) 작성한 줄바꿈을 그대로 두므로, 한 줄로 쓴 early exit와 여러 줄로 쓴 블록이 모두 유지된다. 규칙은 코드 리뷰로 확인한다.
  - include 순서(2.2 Header 규칙)도 자동으로 정렬한다.

### 6.3 예외 조항

- **서드파티 코드** — 서드파티 라이브러리는 `Engine/ThirdParty/` 폴더에 둔다. 이 폴더에는 이 문서의 컨벤션과 clang-format을 적용하지 않는다(`Engine/ThirdParty/.clang-format`에서 서식을 끈다). 업데이트할 때 충돌하지 않도록 원본은 가능하면 수정하지 않는다. 서드파티를 감싸는 엔진 코드는 이 문서의 컨벤션을 따른다.

  ```text
  SeonEngine/
  └─ Engine/
     ├─ Source/
     └─ ThirdParty/
        ├─ .clang-format     // DisableFormat: true
        ├─ imgui/
        └─ stb/
  ```

- **컨벤션 예외** — 꼭 필요하면 컨벤션을 어길 수 있다. 대신 그 자리에 `NOTE` 주석으로 이유를 적는다. 서식 예외는 `// clang-format off` / `// clang-format on`으로 감싼다. 같은 예외가 반복되면 이 문서를 고친다.

  ```cpp
  // NOTE: 컨벤션 예외 — 행렬을 표 모양으로 정렬해야 읽기 쉬움
  // clang-format off
  constexpr float Identity[16] =
  {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };
  // clang-format on
  ```

- **컨벤션 변경** — 컨벤션을 바꾸면 기존 코드도 바로 일괄 수정한다. 일괄 수정은 기능 변경과 섞지 않고 별도 커밋으로 나눈다.
