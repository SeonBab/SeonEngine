# SeonEngine Code Convention

## 관련 문서

- [빌드·개발 설정](../BuildSetup.md) — 프로젝트에 적용한 설정
- [Git Convention](GitConvention.md) — 브랜치, 커밋, 병합
- Asset / Resource Naming Convention — 별도 문서 (예정)

## 적용 범위

| 대상 | 적용 |
|---|---|
| 엔진 코드 (`Engine/`) | **필수**. 이 문서의 모든 규칙을 따른다. 서식은 저장소 루트의 `.clang-format`이 적용한다 |
| 게임 코드 (`Game/`) | **권장**. 엔진과 같은 규칙을 권하지만 강제하지 않는다. 서식은 저장소 루트의 `.clang-format`이 기본으로 적용되고, 게임 폴더에 자기 `.clang-format`을 두면 그쪽이 우선한다 |
| 서드파티 (`Engine/ThirdParty/`) | 적용하지 않는다(6.3 예외 조항) |

## 규칙 변경과 기존 코드 적용

이 문서는 현재 적용할 규칙을 다룬다. 과거 구현·미도입 API를 현재 사용법으로 혼합하지 않는다. 코드와 규칙이 다르면 변경 이유와 실제 계약을 먼저 확인한다. 규칙 변경만으로 기존 코드의 일괄 수정이나 미구현 기능 추가를 승인받은 것으로 보지 않는다. 구체적인 적용 범위와 검증을 제안·승인받은 뒤 수정한다. 기존 코드와의 불일치를 발견했다고 임의의 예외나 일괄 변경으로 처리하지 않는다.

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

이 절의 규칙은 대부분 `.clang-format`이 자동으로 적용한다(6.2 자동화 도구 참고). 도구가 구분하지 못하는 규칙(한 줄 early exit의 범위, 함수 내부의 논리 단위 구분, 반환 전 빈 줄)만 직접 지킨다.

- **들여쓰기** — Tab 사용, 표시 폭 4
- **한 줄 최대 길이** — 제한 없음
- **중괄호 스타일** — Allman. 여는 중괄호는 항상 다음 줄에 둔다.
- **제어문 중괄호** — `if` / `for` / `while` 등은 본문이 한 줄이어도 중괄호를 생략하지 않는다.
- **한 줄 early exit** — 본문이 `return` / `continue` / `break` 한 문장뿐이고 `else`가 없는 `if`에 한해 한 줄 표기를 허용한다. 그 외에는 Allman을 따른다.

  ```cpp
  // 허용
  if (!device) { return; }
  if (!IsVisible(object)) { continue; }

  // 금지: 중괄호 생략
  if (!device)
    return;

  // 금지: early exit가 아닌 본문은 Allman
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
- **인코딩 / 줄바꿈** — UTF-8 (BOM 없음), CRLF. 파일은 newline으로 끝낸다. 관련 프로젝트 설정은 [빌드·개발 설정](../BuildSetup.md) 참고.
- **빈 줄** — 함수 정의 사이, 함수 안의 논리 단위 사이, `#pragma once` / include 묶음 / 선언 사이에 1개를 둔다. 2개 이상 연속으로 쓰지 않고, 여는 `{` 바로 뒤와 닫는 `}` 바로 앞에는 두지 않는다.

  - **함수 내부의 논리 단위** — 하나의 결과를 만드는 연속된 계산·대입은 붙여 쓴다. 준비·계산, 유효성 검사·실패 처리, 실제 처리처럼 목적이 달라지는 지점에는 빈 줄 1개를 둔다. 독립된 검사 블록끼리도 빈 줄로 구분한다. 모든 문장이나 모든 `if` 앞뒤에 기계적으로 넣지는 않는다.

  ```cpp
  const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
  const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
  previousTime = currentTime;

  if (!application.GetClientSize(width, height))
  {
    exitCode = 1;
    break;
  }

  if (width == 0 || height == 0) { continue; }

  world.Tick(deltaSeconds, input, playerInput);
  ```

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
| Macro | UPPER_SNAKE_CASE. 역할·모듈 접두사는 필요에 따라 사용 | `ASSERT`, `PLATFORM_WINDOWS`, `WITH_EDITOR` |

- **외부 구현과 대응하는 이름** — 기본 표기는 위 표를 따른다. 공개 API의 의미와 대응이 확인되어 원본 이름을 유지하기로 정한 경우에만 해당 타입·함수의 좁은 범위에 적용한다. 특정 엔진과 이름이 같다는 이유로 기존 심벌을 개명하지 않는다.

- **타입 접두사** — 모든 타입에 접두사를 붙이며, 각 글자는 타입의 종류를 나타낸다. 표기는 Unreal 관례를 따른다.

  | 접두사 | 뜻 | 대상 |
  |---|---|---|
  | `F` | — | 다른 접두사가 적용되지 않는 일반 class / struct (값 타입, 소유권 등 특정 성질을 뜻하지 않음) |
  | `A` | Actor 계열 | `AActor`와 그 파생 클래스. AActor는 UObject를 상속하지만 Actor 계열은 A를 유지한다. 접두사 자체는 GC나 타입별 메타데이터 지원을 뜻하지 않음 |
  | `U` | UObject 계열 | UObject 및 그 파생 클래스. 상속 관계를 나타내며 GC·타입별 메타데이터의 자동 생성·객체 등록을 뜻하지 않음 |
  | `T` | Template | 템플릿 class / struct |
  | `I` | Interface | 순수 가상 함수로만 이루어진 class |
  | `E` | Enum | 열거형 |
  | `C` | Concept | C++20 concept (3.10 Template 참고) |

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
    {
        vertexCount = initialVertexCount;
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
- **문장과 기호** - 주석은 일반 문장과 기본 문장부호로 쓴다. 가운뎃점이나 화살표 같은 특수문자로 나열이나 흐름을 축약하거나 장식하지 않는다. 나열은 쉼표나 조사로, 순서와 좌표 변환은 말로 풀어 쓴다. 예를 들어 `위치, 회전, 크기`, `생성과 해제`, `로컬에서 월드로 변환하는 행렬`처럼 쓴다. 수식, 코드 식별자, 연산자와 주석 문법 및 기존 구분선은 필요한 경우 그대로 사용한다.
- **원칙** — 이유, 제약, 주의사항(Why)을 우선으로 쓴다. 흐름 구분이나 복잡한 로직의 요약처럼 읽는 데 도움이 되는 설명(What)은 자유롭게 쓸 수 있다. 단, 코드 한 줄을 그대로 풀어 쓰는 주석은 쓰지 않는다.
- **변수 주석** — 이름과 타입을 명확히 정하고, 그래도 사용에 필요한 의미가 충분히 드러나지 않으면 멤버 변수와 지역 변수에도 주석을 단다. 단위와 좌표계, 특수한 값의 의미, 수명과 소유 관계, 상태의 의미와 전환 조건, 허용 범위와 제약의 이유 등을 설명한다. 이름을 그대로 반복하거나 모든 변수에 기계적으로 주석을 붙이지 않는다. 필요한 계약은 선언이나 관련 처리 코드 옆에서 이해할 수 있게 남긴다.
- **문서 참조 금지** — 주석에 규칙 문서나 설계 및 인계 문서의 이름, 경로, 절 제목과 번호, 기록 날짜를 근거로 남기지 않는다. 문서 구성이 바뀌면 주석이 틀어지고 저장소 외부 문서를 열어야 이해할 수 있다. 필요한 이유, 제약과 계약은 주석 자체로 이해되도록 직접 쓰며 문서 참조만 있는 주석은 두지 않는다.

  ```cpp
  // 금지: 코드를 그대로 풀어 씀
  // 카운트를 1 증가시킨다
  ++frameCount;

  // 권장: 이유를 설명함
  // GPU가 이전 프레임 버퍼를 아직 읽고 있을 수 있어 펜스를 기다린다
  WaitForFence(previousFrame);

  // 권장: 변수의 사용 조건을 설명함
  // 음수이면 시간 제한 없이 유지한다.
  float remainingSeconds = -1.0f;

  // 허용: 흐름 구분
  // 1. 가시성 판정
  ...
  // 2. 정렬
  ...
  ```

- **태그** — `태그(이름): 내용` 형식으로 쓴다(`// TODO(seon): 밉맵 생성 추가`). `NOTE`는 담당자가 없으므로 이름을 생략한다. 태그는 다음 네 가지만 쓴다.
  - 현재 동작의 이유·제약·계약은 일반 주석으로 설명한다. 의도한 현재 한계라는 이유만으로 `TODO`를 붙이지 않는다.
  - 향후 조치가 필요한 미완료 작업에는 `TODO`, 알려진 버그에는 `FIXME`, 임시 처리에는 `HACK`을 붙인다. 이런 상태를 일반 설명 주석으로만 남기지 않는다.
  - `NOTE`는 놓치면 잘못 사용하기 쉬운 주의사항에 붙인다. 모든 이유·제약 설명에 붙이지 않으며, 6.3의 컨벤션 예외에는 기존 규칙대로 반드시 붙인다.

  | 태그 | 용도 |
  |---|---|
  | `TODO` | 해야 할 일 |
  | `FIXME` | 알려진 버그 |
  | `NOTE` | 놓치면 잘못 사용하기 쉬운 주의사항, 컨벤션 예외의 이유 |
  | `HACK` | 임시 처리 (제거 조건을 함께 적는다) |

- **주석 종류** — 설명하는 범위에 따라 문법을 구분한다.

  | 범위 | 문법 | 위치 |
  |---|---|---|
  | 클래스 / 구조체 등 큰 단위 | 구분선(`/` 80개) 사이에 `//` | 타입 선언 바로 위 |
  | 함수 | `/** */` | 헤더의 선언 바로 위 (`.cpp` 정의에는 반복하지 않는다) |
  | 변수 | `//` | 선언 바로 위. 짧은 설명은 선언 줄 끝에도 허용 |
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

- **파일 단위** — 타입을 정의하는 파일에는 파일 이름과 같은 주 타입을 하나 둔다. 그 타입에서만 쓰는 작은 보조 타입(Desc 구조체, enum 등)은 같은 파일에 둘 수 있다. WindowsApplication.h/.cpp는 단일 창의 수명과 메시지 처리를 관리하는 FWindowsApplication을 둔다.
- **현재 디렉터리 구조 (2026-10-23 정정)** — 루트 SeonEngine.vcxproj 하나에서 엔진은 Engine/Source, 게임은 Game/Source 아래에 둔다. 엔진은 Windows 등 역할별 폴더를 사용하고 Public/Private·별도 모듈 계층은 아직 나누지 않는다. 이 헤더 중 게임이 호출하는 함수를 공개 API로 보고 1.4의 문서 규칙을 적용한다.

  ```text
  Engine/Source/Windows/
  ├─ WindowsApplication.h
  └─ WindowsApplication.cpp
  ```

- **게임 코드와 진입점 소유 (2026-10-23 정정)** — 게임 제작자는 Game/Source 아래 게임 코드를 작성하고 엔진 API·확장 지점을 사용한다. main은 Engine/Source/Launch/EngineMain.cpp에 두며 엔진 개발 영역에서 초기화·반복·종료를 관리한다. 게임에서 별도 main을 정의하거나 엔진 진입점·플랫폼 구현을 게임별로 수정하지 않는다. 엔진에서 구체 게임 코드나 헤더를 참조하지 않는다. 게임 초기화는 게임 쪽 GameInstance 파생 구현에서 제공한다. 이 규칙은 OS 파일 접근 권한 강제가 아니다.

  ```text
  Engine/Source/Launch/EngineMain.cpp  # 엔진이 소유하는 main
  Game/Source/                        # 게임 전용 소스
  ```

- **공개·내부 경계** — 현재는 Public/Private 모듈 계층을 필수로 두지 않는다. 향후 공개·내부 경계를 나눌 요구가 생기면 프로젝트 구성과 함께 별도로 제안한다.
- **종료 코드 타입** — 표준 main의 종료 코드는 int를 쓴다. 일반 엔진 데이터의 정수 타입 규칙과 구분한다.

### 2.2 Header 규칙 `필수`

- **중복 include 방지** — 모든 헤더의 첫 줄에 `#pragma once`를 쓴다. include guard는 쓰지 않는다.
- **include 순서** — 아래 순서로 그룹을 나누고, 그룹 사이에 빈 줄 1개를 둔다. 그룹 안에서는 알파벳순으로 정렬한다.
  1. 자기 헤더 (`Renderer.cpp`라면 `Renderer.h`)
  2. 엔진 헤더
  3. 서드파티 헤더
  4. 표준 라이브러리 헤더

- **include 경로 (2026-10-15 정정)** — 엔진 헤더는 Engine/Source 기준으로 쓴다(`#include "Windows/WindowsApplication.h"`). cpp의 자기 헤더는 같은 폴더의 파일명으로 쓴다(`#include "WindowsApplication.h"`). 게임 내부 헤더 경로는 실제 필요가 생기는 단계에서 등록한다.
  - 엔진 헤더는 `""`, 표준 라이브러리와 서드파티 헤더는 `<>`로 include한다.
  - `../` 같은 상대 경로는 쓰지 않는다.
  - 헤더 이름은 엔진 전체에서 고유해야 한다. (다른 모듈에 같은 이름의 헤더를 두지 않는다)

  ```cpp
  #include "Windows/WindowsApplication.h" // Engine/Source 기준

  #include <vector>

  // 금지
  #include "../Public/Renderer.h"
  ```

- **전방 선언** — 헤더에서 포인터, 레퍼런스, 함수 선언에만 쓰는 타입은 include하지 않고 전방 선언한다. 값 멤버, 부모 클래스, 헤더 안의 inline 함수에서 멤버를 사용하는 타입은 include한다. 전방 선언은 클래스처럼 전방 선언할 수 있는 타입에만 적용한다. 타입 별칭(`FString`, `TArray` 등)은 포인터나 레퍼런스로만 써도 별칭을 제공하는 헤더를 include한다.
- **직접 쓰는 것만 include** — 파일은 자기가 직접 쓰는 헤더를 include한다. 다른 헤더를 통해 간접적으로 들어오는 헤더에 기대지 않는다. `.cpp`에서만 필요한 include는 헤더가 아니라 `.cpp`에 둔다.
- **Windows SDK 입구 (2026-10-15)** — Windows 구현은 WindowsHWrapper.h를 통해 Windows.h를 포함한다. 래퍼는 WIN32_LEAN_AND_MEAN과 NOMINMAX를 Windows.h보다 먼저 정의한다. FWindowsApplication이 HINSTANCE/HWND를 멤버로 보관하므로 Windows 전용 WindowsApplication.h에서도 래퍼를 포함한다(2026-10-16 정정). 이 헤더를 포함하는 코드에도 SDK 선언이 노출된다. 다른 SDK 헤더를 무조건 래퍼에 모으거나 이전 엔진의 TCHAR·이름 충돌 처리를 자동 복원하지 않는다.
- **`using namespace`** — 헤더에서는 쓰지 않는다. `.cpp`에서도 `using namespace std;`는 쓰지 않는다(Windows 헤더의 `byte`와 `std::byte`가 충돌한다). `.cpp`에서 `using std::vector;`처럼 이름 하나씩 가져오는 것은 허용한다.

### 2.3 Class 구성 `필수`

- **기능별 구역** — 관련 함수와 상태를 `#pragma region 기능명` / `#pragma endregion`으로 함께 묶는다. 생성·소멸은 첫 `Lifecycle` 구역에 둔다. 나머지 구역 이름은 `Mesh`, `Material`, `RenderState`처럼 실제 책임을 드러내는 PascalCase로 정한다. 함수 하나마다 구역을 만들거나 구역을 중첩하지 않는다. 구분할 기능이 없는 작은 클래스와 단순 데이터 struct에는 불필요한 구역을 만들지 않는다.
- **구역 안의 선언 순서** — 필요한 타입 별칭·중첩 타입을 사용하는 선언보다 앞에 두고, 함수 블록 다음에 변수 블록을 둔다. 함수 블록은 `public` → `protected` → `private`, 변수 블록은 `protected` → `private` 순서로 배치한다. 아래 멤버 변수 접근 규칙에서 허용한 공개 이벤트·상수가 있으면 변수 블록의 `public`을 먼저 둔다. 멤버가 없는 접근 단계는 생략한다. 생성자·소멸자는 Lifecycle의 해당 접근 단계에서 다른 함수보다 먼저 둔다.
- **접근 지정자 반복** — 각 기능 구역과 함수·변수 블록의 시작에 필요한 접근 지정자를 명시한다. 같은 구역에서도 private 함수 뒤에 private 변수가 이어지면 `private:`를 다시 써서 경계를 나타낸다. `#pragma endregion`은 접근 권한을 초기화하지 않으며, 접근 권한 자체를 바꾸기 위해 멤버를 공개하지 않는다.
- **선언과 정의 대응** — `.cpp`에서도 같은 기능 구역 이름과 순서로 정의를 묶고 구역 안의 함수 순서를 헤더에 맞춘다. 헤더에만 있는 타입·변수나 inline 정의 때문에 `.cpp`에 빈 구역을 만들지는 않는다. 기존 함수 계약·변수 수명 주석은 유지하고, 구역 이름을 반복하는 구분용 주석은 덧붙이지 않는다. 기존 코드의 일괄 재배치는 별도 작업으로 다루며, 멤버 변수 순서 변경 시 초기화·역순 파괴·레이아웃 영향을 확인한다.

  ```cpp
  class FRenderer
  {
  #pragma region Lifecycle

  public:
    FRenderer();
    ~FRenderer();

  #pragma endregion

  #pragma region Rendering

  public:
    void Render();

  protected:
    virtual void OnResize();

  private:
    void CreateDevice();

  private:
    FRenderDevice* device = nullptr;

  #pragma endregion
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
  - 외부에서 구독하는 이벤트(델리게이트). 소유 클래스만 발생시킬 수 있는 이벤트 타입을 쓴다.
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

- **멤버 초기화** — 멤버의 기본값은 선언부에 둔다. 생성자에서 본문 대입이 가능한 값은 본문에서 설정하며, 조건에 따른 선택은 `if`로 풀어 흐름을 읽기 쉽게 작성한다(1.2 Naming의 `FMesh` 예시 참고).
  - 기반 클래스 생성자에 인수를 전달하거나 `const` 멤버·기본 생성자가 없는 타입을 초기화할 때는 초기화 리스트를 사용한다. 본문 대입을 위해 타입의 제약을 없애지 않는다.
  - 기본 생성 후 대입하면 불필요한 자원 생성이나 유의미한 비용이 발생하는 타입도 초기화 리스트로 직접 초기화한다.
  - 본문 대입은 멤버 초기화가 끝난 뒤 수행된다. 직접 초기화와 같은 언어 동작으로 보지 않으며, 값을 설정하기 전에 해당 멤버를 읽지 않는다.
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
- **인자 전달** — 기본형, enum, 포인터, 16바이트 이하의 작은 struct는 값으로 받는다. 그 외에는 `const&`로 받는다. 소유권을 무조건 넘겨받을 때는 값으로 받고 `std::move`한다. 실패 시 호출자의 소유를 유지하는 API는 스마트 포인터를 비const 레퍼런스로 받아 성공한 경우에만 이동하며, 입력 비움과 실패 보존 계약을 명시한다.

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

- **`[[nodiscard]]`** — 실패 확인이나 반환 자원 처리를 빠뜨리기 쉬운 함수에 사용한다. 성공/실패와 새 자원을 반환하는 함수가 대표 대상이다. getter에는 일괄 강제하거나 금지하지 않으며 반환값을 사용하는 계약에 따라 판단한다. 기존 표기를 바꿀 때는 실제 호출과 의미를 확인하고 승인 범위에 포함한다.
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
- **비소유 멤버 표현** — 기본은 포인터로 저장하며 필수 대상은 생성자에서 레퍼런스로 받아 주소를 보관한다. 현재 UActorComponent.owner와 FWindowsApplication.inputState는 생성 시 고정되는 필수 비소유 관계이므로 레퍼런스 멤버를 유지한다. 두 타입은 복사와 대입을 금지하며, 이 좁은 예외를 다른 타입에 자동 확대하지 않는다. 레퍼런스 멤버는 재결합할 수 없고 기본 대입 연산자에 제약을 주지만, 레퍼런스 멤버만을 이유로 모든 컨테이너 보관이 불가능하다고 판단하지 않는다.

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

- **C 스타일 캐스트 금지** — `(T)value`와 `T(value)` 형태의 캐스트는 쓰지 않고 C++ 캐스트를 쓴다. 다형 객체의 실제 타입을 확인하는 변환에는 `dynamic_cast`를 사용할 수 있다. 포인터 변환 실패 시 반환되는 `nullptr`를 처리한다. 실패 가능한 변환은 예외를 던지는 참조 변환 대신 포인터 변환과 검사로 처리한다.
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
- **비트 플래그** — 이름은 `Flags`로 끝낸다. `None = 0`을 두고 값은 `1 << n`으로 쓴다. 비트 연산자는 `ENUM_CLASS_FLAGS` 매크로로 정의한다.

  ```cpp
  enum class ETextureUsageFlags : uint8
  {
    None = 0,
    ShaderRead = 1 << 0,
    RenderTarget = 1 << 1,
    DepthStencil = 1 << 2
  };
  ENUM_CLASS_FLAGS(ETextureUsageFlags)

  ETextureUsageFlags usage = ETextureUsageFlags::ShaderRead | ETextureUsageFlags::RenderTarget;
  ```

- **특수 값** — 값의 개수가 필요하면 마지막에 `Count`를 둔다. "없음"이 필요하면 맨 앞에 `None`을 둔다. 맨 앞 값이 "없음"이 아니라 그 자체로 뜻이 있는 값이면 그 뜻의 이름을 쓴다(예: "로그 끔"인 `ELogVerbosity::NoLogging`).

### 3.8 상수 / Literal `중요`

- **매직 넘버** — 의미가 있는 숫자는 `constexpr` 상수로 이름을 붙인다. `0`, `1`, `-1`, 절반을 뜻하는 `0.5f`처럼 뜻이 자명한 값은 그대로 쓸 수 있다.
- **매직 문자열** — 에셋 경로, 셰이더 진입점 이름, 설정 키처럼 두 곳 이상에서 쓰거나 오타가 런타임에야 드러나는 문자열은 상수로 만든다. 로그 메시지처럼 한 번만 쓰는 문자열은 그대로 쓴다.
- **리터럴** — float 값은 `1.0f`처럼 소수점과 `f`를 모두 쓴다(`1.0`은 double, `1.f`는 쓰지 않음). 접미사는 소문자로 쓴다. 큰 숫자는 `'`로 자리를 나눌 수 있다.

### 3.9 Macro `중요`

- **이름** — UPPER_SNAKE_CASE를 사용하며 `SE_` 접두사를 일괄 요구하지 않는다(1.2 Naming 참고). 역할이나 소속을 구분할 때는 `PLATFORM_`, `BUILD_`, `WITH_` 또는 모듈 이름처럼 의미 있는 접두사를 사용한다. 외부 코드와 충돌하면 구체적인 역할·모듈·프로젝트 이름으로 구분한다.
- **리플렉션 선언 표기** — ReflectionMacros.h의 `UCLASS()`·`UPROPERTY()`·`GENERATED_BODY()`는 선언 기반 생성의 고정 표기다. UPROPERTY/UCLASS는 UPPER_SNAKE_CASE 규칙의 좁은 예외이며 사용자 정의 일반 매크로로 확대하지 않는다. 현재 옵션 인수를 받지 않는다. generated.h는 원본 헤더의 마지막 include로 두고 GENERATED_BODY 다음에 접근 지정자를 명시한다. 생성된 SE_REFLECTION 계열 매크로와 SEGetPropertyValue_/SESetPropertyValue_ 함수는 도구가 관리하므로 수동 선언·재정의하지 않는다. 생성 내부 함수 이름의 밑줄은 자동 생성 식별자를 구분하는 좁은 예외다. GENERATED_BODY는 클래스 멤버 선언을 펼치는 매크로이므로 실행 문장용 do/while로 감싸지 않는다.
- **기존 이름** — 이 명명 규칙 변경만으로 기존 매크로를 일괄 개명하지 않는다. 실제 API나 빌드 설정을 설명할 때는 현재 이름을 사용하고, 이름을 바꾸는 작업에서는 정의·사용처·프로젝트 설정·관련 문서를 함께 갱신한다.
- **사용 범위** — 매크로로만 할 수 있는 일에만 쓴다.
  - 플랫폼 / 빌드 구성 분기
  - 파일 이름과 줄 번호가 필요한 assert, 로그
  - 반복 코드 생성 (`ENUM_CLASS_FLAGS`, 리플렉션 등록 등)

  상수는 `constexpr`, 함수는 inline 함수나 템플릿으로 쓴다(`#define MAX_LIGHTS 16` 같은 매크로 상수는 쓰지 않는다).

- **분기 매크로** — 항상 `0` 또는 `1`로 정의하고 `#if`로 검사한다. 꺼진 기능도 정의를 생략하지 않고 `0`으로 둔다(`#define WITH_EDITOR 0` → `#if WITH_EDITOR`). `#ifdef`는 쓰지 않는다. 예외로, 현재 구현의 `SE_PLATFORM_HEADER_NAME`과 같은 헤더 경로 생성용 이름 매크로처럼 0 / 1이 아닌 필수 입력이 정의되었는지 확인할 때는 `#if !defined(MACRO)` → `#error`를 쓸 수 있다. 이름은 종류에 따라 접두사를 나눈다.

  | 접두사 | 용도 | 예시 |
  |---|---|---|
  | `PLATFORM_` | 플랫폼 | `PLATFORM_WINDOWS` |
  | `BUILD_` | 빌드 구성 | `BUILD_DEBUG`, `BUILD_RELEASE` |
  | `WITH_` | 켜고 끌 수 있는 기능 | `WITH_EDITOR`, `WITH_PROFILER` |

- **생성기 분석 분기** — SE_REFLECTION_ANALYSIS는 기본0, 도구 분석에서1이다. 일반 제품 컴파일에1을 전달하지 않는다. __clang__ 같은 컴파일러 제공 식별자는 값이 아닌 정의 여부로 분석 도구의 지원 환경을 확인한다. SE_REFLECTION_FILE_ID는 생성 파일의 토큰 식별자이므로0/1 분기 매크로가 아니며 다음 생성 헤더가 #undef 후 연결한다.

- **작성 규칙**
  - 매크로 인자는 괄호로 감싼다.
  - 여러 문장인 매크로는 `do { ... } while (0)`로 감싼다.
  - 한 파일에서만 쓰는 매크로는 다 쓴 뒤 `#undef`한다.

  ```cpp
  #define ASSERT(condition) \
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
- **멤버 함수 배치 (2026-10-16)** — class/struct의 템플릿 멤버 함수는 내부에 선언하고, 정의는 같은 헤더의 타입 정의 아래에 선언 순서대로 둔다. 한 줄짜리 단순 함수는 내부 정의를 허용한다. 바깥 정의에도 `template`과 동일한 `requires`를 적고 `타입명::함수명`으로 소속을 표시한다. 클래스 템플릿의 멤버라면 소속 타입의 템플릿 인자도 적는다. 일반적인 템플릿 정의를 `.cpp`로 옮기지 않는다.
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

- **Windows 문자열 경계** — Windows 전용 설정의 FWindowDescription.title은 std::wstring으로 소유한다. 엔진 공통 문자열과 인코딩 변환 계층을 자동 도입하는 근거로 삼지 않는다.

- **컨테이너 / 문자열** — STL을 내부 구현으로 쓰고, 코드에서는 엔진 타입을 사용한다.
  - `TArray` / `FString`은 STL 별칭이며 멤버 함수는 STL 이름(`push_back`, `size` 등)을 그대로 쓴다. 이 부분은 1.2 Naming의 예외다.
  - `THashMap`은 래퍼 클래스다. `Add`는 추가 / 기존 값 교체 후 값 참조, `Find`는 값 포인터 / 미등록 nullptr, `Reset`은 항목 제거 / 버킷 공간 유지 계약을 제공한다. 외부에 STL 반복자와 멤버 함수를 노출하지 않는다. 현재 const 참조 입력만 제공하며 전체 Unreal TMap API는 지원하지 않는다.

  ```cpp
  template<typename T>
  using TArray = std::vector<T>;

  // Core/Containers/HashMap.h의 래퍼를 사용한다.
  THashMap<uint32, FString> names;
  names.Add(1, "One");
  const FString* name = names.Find(1);
  names.Reset();

  using FString = std::string;

  TArray<FMesh*> meshes;
  meshes.push_back(mesh);
  ```

- **정수 타입** — 엔진 데이터에는 크기를 명시한 int8부터 int64, uint8부터 uint64를 사용한다. Core/CoreTypes.h가 기본 정수 타입의 별칭과 크기 검사를 제공한다. 실수는 float/double을 그대로 쓰고 표준 라이브러리의 size_t와 표준 main·외부 API의 인수/반환 타입은 해당 계약을 따른다.
- **문자열 별칭과 인코딩** — Core/Containers/SeonString.h의 FString = std::string, FStringView = std::string_view를 사용한다. FString은 소유, FStringView는 비소유이며 원본 수명을 연장하지 않는다. 엔진 문자열은 UTF-8로 취급하되 별칭 자체가 인코딩을 검사하거나 변환하지는 않는다. std::format 결과는 FString과 같은 타입이다. Windows의 wide 문자열은 해당 플랫폼 API 경계에서 사용하며, 현재 없는 FWindowsString 변환 API를 호출하거나 새로 추가하지 않는다.

---

## 4장. Memory & Lifetime

### 4.1 메모리 소유권 `매우 중요`

- **스마트 포인터 별칭** — 3.12 STL 사용 정책과 같이 STL 스마트 포인터를 엔진 별칭으로 감싸서 쓴다.

  ```cpp
  template<typename T>
  using TUniquePtr = std::unique_ptr<T>;   // TSharedPtr도 표준 스마트 포인터의 별칭

  // MakeUnique / MakeShared는 std::make_unique / std::make_shared를 호출하는 함수로 정의한다.
  ```

- **소유 / 비소유** — 단독 소유는 기본적으로 `TUniquePtr`로 표현한다. 나머지는 일반 포인터나 레퍼런스로 빌려 쓴다. 외부에 전달하는 일반 포인터와 레퍼런스는 비소유이며, 가리키는 객체를 지우지 않는다. 직접 할당을 관리하는 경우는 아래 규칙을 따른다.

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

- **일반 C++ 객체의 할당** — 엔진 / 게임 위치가 아니라 객체의 수명 관리 방식으로 구분한다. 일반 C++ 객체는 양쪽 모두 `new` / `delete`를 사용할 수 있다. 기본은 `MakeUnique`(공유가 필요하면 `MakeShared`)이며, private 생성자 팩토리에서는 `new` 결과를 즉시 스마트 포인터에 넘길 수 있다. 직접 수명을 관리할 때는 생성 / 해제 담당과 모든 종료 / 실패 경로를 명확히 한다. `delete`는 해당 할당의 소유자만 수행하며 스마트 포인터 소유 객체나 빌린 포인터를 직접 삭제하지 않는다. 컨테이너 / 할당자 내부의 placement new는 해당 저장 공간의 수명 계약을 따른다. D3D11 같은 COM 객체는 계속 `ComPtr`로 관리한다.
- **엔진 관리 객체** — 등록과 수명을 관리하는 시스템의 생성 / 제거 계약을 우회하지 않는다. 현재 월드 액터는 `SpawnActor` / `DestroyActor`를 사용하며, 반환받은 비소유 주소를 직접 삭제하지 않는다. 컴포넌트 등 소유권을 이전한 객체도 해당 소유자가 정리한다.
- **Unreal UObject 규칙과 현재 한계** — Unreal의 GC 관리 UObject는 엔진 / 게임 모두 직접 `new` / `delete`하지 않고 전용 생성 API와 GC로 관리한다. 자체 엔진에는 아직 GC / NewObject가 없으므로 U 접두사만으로 이 규칙을 일괄 적용하지 않는다. 현재는 각 타입의 스마트 포인터 / 생성 / 제거 계약을 따르고, GC 도입 시 관리 대상 타입의 규칙을 갱신한다.
- **`TSharedPtr`** — 여러 시스템이 함께 소유하고 누가 마지막인지 정할 수 없는 경우에만 쓴다. 편의를 위해 쓰지 않는다. 관찰만 할 때는 비소유 수명 보장이나 약한 참조가 필요한지 먼저 판단한다. 현재 TWeakPtr 별칭은 없으므로 제공된다고 가정하지 않고, 약한 참조가 필요하면 해당 수명 계약과 구현을 별도 제안한다.
- **참조와 핸들** — 현재 타입의 실제 소유·반환 계약을 따른다. 공유 자산은 TSharedPtr로 수명을 유지하고 일반 조회 포인터는 비소유로 취급한다. 자산이 살아 있어도 내부 RenderData 교체로 기존 주소가 무효가 될 수 있으므로 해당 사용을 먼저 끝낸다. 자원 관리자·세대 번호 핸들은 현재 공통 구현이 아니며, 참조가 여러 곳에 있다는 이유만으로 미구현 핸들 API를 사용하거나 기존 참조를 바꾸지 않는다. 핸들이 필요한 요구가 생기면 생성·조회·무효화·해제 경로를 함께 설계한다.

### 4.2 객체 생명주기 `매우 중요`

- **생성 / 파괴 책임** — 소유자가 객체를 만들고 파괴하며 단독 소유의 기본 표현은 TUniquePtr다. 실행 루프는 창·Renderer·실행 엔진을, 실행 엔진은 자신의 World와 필요한 하위 객체를 소유한다. 빌린 참조 대상보다 참조 사용을 먼저 끝내고 정리한다. 공유 소유와 비소유를 소유 트리로 잘못 표현하지 않는다.
- **초기화 방식** — 실패 가능한 무거운 준비는 생성과 분리하고 결과·부분 실패·재호출·정리 계약을 공개 선언에 명시한다. 작은 가드 등 실패 없는 보조 타입은 RAII를 사용한다. 소멸자가 명시적 종료 훅을 자동 호출하는지는 타입마다 확인한다.
- **준비·정리 이름** — 현재 타입의 공개 API를 사용하고 이름을 일괄 변경하지 않는다. FD3D11Device는 bool Initialize()/void Shutdown(), UEngine은 Initialize/Start/Tick/PreExit, UGameInstance는 bool Initialize(UWorld&, FGameClient&, FRenderer&)를 제공한다. 제거된 InitD3DDevice·InitializeStandalone·Init/Shutdown 게임 훅을 현재 API로 사용하지 않는다. 새 API는 실제 역할과 기존 호출 흐름을 확인해 제안한다.
- **서브시스템 순서** — 의존 대상부터 준비하고 의존하는 쪽부터 정리한다. 준비 실패 시 해당 단계에서 확보한 자원만 정리하며 성공한 부분의 보존·롤백 여부는 API 계약에 따른다. 전역/static 생성자에서 파일·장치 같은 외부 자원을 준비하지 않는다.
- **비소유 참조** — 대상이 사용 종료까지 살아 있음을 보장한다. 보장할 수 없으면 참조 방식을 다시 설계하고, 이미 제공되는 TWeakPtr나 핸들이 있을 때 그 계약을 따른다. 미구현 수명 관리 기능을 가정하지 않는다.

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

- **컴파일러 예외 지원과 실패 처리의 구분** — 빌드는 `/EHsc`로 표준 C++ 예외를 지원하지만 엔진의 기본 실패 전달은 위 반환값·로그 규칙을 유지한다. 예외 지원을 이유로 실행 루프·Windows 메시지·종료 경로에 try/catch를 추가하거나 기존 bool API를 예외 방식으로 전환하지 않는다. 외부 라이브러리의 예외 발생·내부 처리 계약은 도입 시 확인한다. 잡히지 않은 예외의 복구나 정상 자원 정리는 컴파일러 옵션만으로 보장되지 않는다.

- **종류별 처리** — 에러를 세 가지로 나눠 처리한다.

  | 종류 | 예 | 처리 |
  |---|---|---|
  | 프로그래머 실수 | 잘못된 인자, 호출 순서 위반, 불변 조건 위반 | 현재 API의 검사·반환 계약을 따른다. 미구현 진단 매크로를 호출하지 않는다(5.2 참고) |
  | 외부 요인 | 파일 없음, 잘못된 에셋, 사용자 입력 오류 | 에러를 돌려주고 로그를 남긴다. 가능하면 대체물(기본 텍스처 등)로 계속 진행한다 |
  | 실행 지속 불가 | 장치 준비 실패 등 | API의 실패를 호출부로 전달하고 부분 자원 정리 후 실행을 종료한다. 메모리 부족 복구와 치명 로그 API가 자동 제공된다고 가정하지 않는다 |

### 5.2 Assert `중요`

현재 엔진에는 ASSERT/ASSERTF/VERIFY/ENSURE 공통 매크로 구현이 없다. static_assert처럼 실제 제공되는 C++ 기능과 각 API의 검사·로그·반환 계약을 사용한다. 과거 매크로 정책을 현재 사용법으로 적용하거나 규칙 정리를 이유로 매크로를 추가하지 않는다. 공통 진단 기능을 도입할 때는 조건식 부작용, Debug/Release 평가 여부, 로그·중단·종료 동작과 호출부를 함께 제안한다.

### 5.3 Logging `중요`

#### 현재 로깅 API

- `Core/Logging/Log.h`의 `ELogLevel::Info`, `Warning`, `Error`와 `WriteLog(category, level, format, args...)`를 사용한다. 세 레벨은 모두 stderr에 `[카테고리][레벨] 메시지`와 마지막 줄바꿈 하나를 출력한다. Debug/Release의 출력 정책은 같다.
- 공개 WriteLog는 std::format_string으로 형식 문자열과 인자 타입을 컴파일할 때 검사하고 내부에서 std::format으로 메시지를 만든다. 완성된 FString/FStringView나 실행 중 생성한 문자열은 `WriteLog(category, level, "{}", message)`로 전달한다. 이때 message 안의 중괄호·%는 다시 해석하지 않는다. 리터럴 자체의 중괄호는 `{{`·`}}`로 쓴다.
- 카테고리는 FStringView로 받고 완성된 메시지는 내부 SE::Private::WriteLogMessage에 FStringView로 즉시 전달한다. Core/Containers/SeonString.h의 별칭을 사용하며 TCHAR는 추가하지 않는다. 동적 문자열을 형식 문자열 자리에 직접 넣는 이전 호출 방식은 지원하지 않는다.
- 카테고리는 기능 영역을 나타내는 문자열이다(예: `Windows`). 빈 카테고리는 `General`로 표시한다. 카테고리와 메시지는 호출 중에만 빌려 쓰며 보관하지 않는다. 등록·이름 검증·카테고리별 필터는 제공하지 않는다.
- 전역 로거 객체·장치 등록·필터·Fatal·파일·디버거 출력은 현재 제공하지 않는다.
- 오류 코드 저장, 자원 정리, 반환과 프로그램 종료는 호출부 책임이다. 로그 함수는 void이며 출력 실패를 보고하거나 재시도하지 않는다. 호출 간 전체 로그 줄의 원자성·스레드 간 순서를 보장하지 않는다.

```cpp
WriteLog("Windows", ELogLevel::Error, "CreateWindowExW failed (error {})", errorCode);
```

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
- **예외 / RTTI** — 표준 C++ 예외 지원은 `/EHsc` (`ExceptionHandling=Sync`)로 켠다. `_HAS_EXCEPTIONS`는 직접 정의하지 않아 MSVC 표준 라이브러리의 기본 설정을 따른다. `/EHa`는 사용하지 않는다. 엔진의 실패 전달 방식은 5.1을 따르며 컴파일러 지원과 별도로 정한다. RTTI는 켠다(`/GR`). Visual Studio 디버거의 예외 감지·중단 설정은 이 컴파일러 옵션과 별개다.
- **빌드 매크로** — 구성마다 프로젝트 전처리기 정의로 `SE_PLATFORM_*`, `SE_BUILD_*`를 `0` / `1`로 정의한다(3.9 Macro 참고). 구성별 값은 [빌드·개발 설정](../BuildSetup.md)에 있다.

### 6.2 자동화 도구 `자동화`

- **`.editorconfig`** — 인코딩, 줄바꿈, 들여쓰기를 에디터에 적용한다([빌드·개발 설정](../BuildSetup.md) 참고).
- **`.clang-format`** — 1.1 Formatting 규칙을 자동으로 적용한다. Visual Studio에 내장된 clang-format은 파일에서 가장 가까운 폴더의 `.clang-format`을 읽는다. 커밋 전에 문서 서식(Ctrl+K, Ctrl+D)을 실행한다.
  - 저장소 루트 `.clang-format`: 엔진 코드의 서식이고, 게임 코드의 권장 기본값이다.
  - `Engine/ThirdParty/.clang-format`: 서드파티의 서식을 끈다.
  - 표현하지 못하는 규칙: "한 줄 표기는 early exit만"은 clang-format이 본문 종류를 구분하지 못한다. 대신 줄 길이 제한이 없어(`ColumnLimit: 0`) 작성한 줄바꿈을 그대로 두므로, 한 줄로 쓴 early exit와 여러 줄로 쓴 블록이 모두 유지된다. 규칙은 코드 리뷰로 확인한다.
  - 함수 내부의 논리 단위 구분과 반환 전 빈 줄(1.1)은 코드의 의미를 기준으로 직접 배치하고 리뷰한다. `MaxEmptyLinesToKeep: 1`은 연속 빈 줄의 최대 개수만 제한하며, 논리 단위 사이에 필요한 빈 줄을 자동으로 추가하지 않는다.
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
  // NOTE: 컨벤션 예외: 행렬을 표 모양으로 정렬해야 읽기 쉬움
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

- **컨벤션 변경** — 변경한 규칙과 기존 코드 적용 범위를 구분한다. 일괄 적용은 대상·영향·검증을 별도로 제안·승인받아 진행하며 기능 변경과 섞지 않는다. 문서 정리만 승인받았다면 제품 코드는 유지한다. 커밋은 요청받은 경우에만 수행하고 기존 코드의 남은 적용 범위는 별도 작업으로 명시한다.
