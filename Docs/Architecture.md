# SeonEngine Architecture

엔진 구조에 대한 규칙이다. 코드를 어떻게 쓰는지는 [Code Convention](Conventions/CodeConvention.md)을 따른다.

---

## 1. 모듈과 의존성 방향

- **모듈 구성** — `Core`, `Engine`과 게임 모듈로 시작하고, 코드가 커지면 나눈다(예: `Engine`에서 `Renderer`, `RHI`, `Platform` 분리). 의존은 위에서 아래로만 한다.

  ```text
  SampleGame  (게임 모듈, 콘텐츠 코드)
    │
    ▼
  Engine      (렌더링, 플랫폼, 입력, 씬 등 엔진 기능)
    │
    ▼
  Core        (타입, 컨테이너, 수학, 로그, assert, 메모리)
  ```

  - 하위 모듈은 상위 모듈의 헤더를 include하지 않는다(`Core`는 `Engine`을, `Engine`은 게임 모듈을 모른다).
  - 모듈은 폴더로 나누고, 엔진 모듈 안에는 `Public` / `Private`를 둔다. 게임 모듈은 나누지 않는다(Code Convention 2.1 파일 구성 참고).
  - 게임 모듈은 게임 프로젝트마다 하나씩 두고, 프로젝트 이름을 모듈 이름으로 쓴다(예: `SampleGame`).
- **모듈 위치와 강제 방식** — 엔진 모듈은 `Engine/Source/<모듈>/`, 게임 모듈은 `<게임>/Source/<게임>/`에 둔다(Project Settings 폴더 구조 참고).
  - 엔진 모듈은 `Engine` 프로젝트 하나에 담는다. 엔진 모듈 사이의 의존 방향은 빌드 설정으로 강제하지 않으므로 코드 리뷰에서 확인한다.
  - 게임 모듈은 별도 프로젝트에 담고 include 경로에 엔진 모듈의 `Public`만 넣는다. 게임이 엔진 내부 헤더를 쓰는 것은 빌드가 막는다.
- **같은 계층끼리 의존 금지** — 모듈을 나눈 뒤 같은 계층의 모듈(예: `Renderer`와 `Audio`)은 서로 의존하지 않는다. 함께 동작해야 하면 상위 모듈이 연결한다.
- **역방향 통지** — 하위 모듈이 상위 모듈에 알려야 할 일은 인터페이스나 이벤트(델리게이트)로 전달한다. 상위 모듈의 타입을 직접 참조하지 않는다.

  ```cpp
  // Core — 상위를 모른 채 인터페이스만 정의
  class ILogSink
  {
  public:
  	virtual ~ILogSink() = default;
  	virtual void Write(const NString& message) = 0;
  };

  // Engine — 구현해서 Core에 등록
  class NFileLogSink final : public ILogSink
  {
  public:
  	void Write(const NString& message) override;
  };
  ```

## 2. 서브시스템과 전역 상태

- **싱글톤 금지** — 클래스가 스스로 인스턴스를 들고 있는 싱글톤(`GetInstance()`)은 만들지 않는다. 생성 / 파괴 시점을 제어할 수 없어 아래 초기화 / 종료 순서를 깨뜨린다. 서브시스템은 `NEngine`이 소유하고, 전역에는 `gEngine` 하나만 둔다.
- **초기화 / 종료 순서** — `NEngine::Initialize()` 한 곳에서 초기화 순서를 명시하고, `NEngine::Shutdown()`은 정확히 그 역순으로 정리한다. 전역 / static 객체의 생성자에서는 초기화 작업을 하지 않는다(파일 사이의 전역 초기화 순서는 보장되지 않는다).

  ```cpp
  bool NEngine::Initialize()
  {
  	if (!platform->Initialize()) { return false; }
  	if (!renderer->Initialize(rendererDesc)) { return false; }
  	if (!input->Initialize()) { return false; }
  	return true;
  }

  void NEngine::Shutdown()
  {
  	input->Shutdown();
  	renderer->Shutdown();
  	platform->Shutdown();
  }
  ```

- **서브시스템 접근** — 타입으로 조회한다. 서브시스템은 `NEngine::Initialize()`에서 등록한다.

  ```cpp
  NEngine* gEngine = nullptr;

  NRenderer* renderer = gEngine->GetSubsystem<NRenderer>();

  // 금지
  NRenderer& renderer = NRenderer::GetInstance();
  ```

- **전역 변수** — 새 전역 변수는 만들지 않는다. 이 문서에 적힌 엔진 기반 전역(`gEngine`)만 둔다. 상수(`constexpr`)와 `.cpp` 안의 익명 namespace 변수는 허용한다. 필요한 객체는 가능하면 생성자나 함수 인자로 넘겨받는다.

## 3. 플랫폼 추상화

- **분기 코드 위치** — 플랫폼별 코드는 `Platform/` 폴더에만 둔다. 그 외에는 Core의 플랫폼 매크로 정의 헤더만 `#if SE_PLATFORM_*` 분기를 쓸 수 있다. 일반 코드는 플랫폼에 따라 나뉘지 않는다.

  ```text
  Engine/
  └─ Private/
     └─ Platform/
        ├─ PlatformFile.h
        └─ Windows/
           └─ WindowsPlatformFile.cpp
  ```

- **구현 방식** — 지원 플랫폼이 하나인 동안은 공통 헤더에 선언하고 플랫폼별 `.cpp`에서 구현한다. 플랫폼이 두 개 이상이 되면 `Generic` 공통 구현을 두고 플랫폼 구현이 상속한 뒤 `using`으로 고르는 방식으로 바꾼다. 두 방식 모두 사용하는 코드는 `NPlatformFile`이라는 같은 이름을 쓰므로 전환할 때 사용처를 고치지 않는다.

  ```cpp
  // 플랫폼이 하나일 때
  // PlatformFile.h
  class NPlatformFile
  {
  public:
  	static bool Exists(const NString& path);
  };

  // Windows/WindowsPlatformFile.cpp
  bool NPlatformFile::Exists(const NString& path)
  {
  	...
  }

  // 플랫폼이 둘 이상일 때
  struct NGenericPlatformFile { ... };
  struct NWindowsPlatformFile : NGenericPlatformFile { ... };

  using NPlatformFile = NWindowsPlatformFile;
  ```

- **`Windows.h`** — 직접 include하지 않고 `Platform/Windows/WindowsHeaders.h` 래퍼만 include한다. 래퍼는 Platform과 그래픽스 API 전용 폴더(4장 렌더링 백엔드 참고)의 `.cpp`(또는 Private 헤더)에서만 include하고, Public 헤더에서는 include하지 않는다.
  - 래퍼는 `WIN32_LEAN_AND_MEAN`, `NOMINMAX`를 정의한 뒤 `Windows.h`를 include하고, 엔진 이름과 충돌하는 매크로(`CreateWindow` 등)를 `#undef`한다.

  ```cpp
  // Platform/Windows/WindowsHeaders.h
  #pragma once

  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>

  #undef CreateWindow
  ```

- **진입점** — 플랫폼 진입점은 플랫폼별 준비만 하고 플랫폼과 관계없는 `EngineMain()`을 호출한다. 엔진 초기화, 메인 루프, 종료는 `EngineMain()` 쪽에 둔다. 진입점은 엔진 모듈에 있고, 게임 모듈에는 진입점을 두지 않는다.

  ```text
  Engine/Private/
  ├─ Launch/
  │  └─ EngineMain.h / .cpp          int EngineMain()
  └─ Platform/Windows/
     └─ WindowsLaunch.cpp            WinMain → EngineMain()
  ```

  - Windows 진입점은 `wWinMain`이 아니라 `WinMain`을 쓴다. 진입점이 정적 라이브러리 안에 있으면 링커가 진입점 종류를 알아내지 못하고 기본값(`WinMain`)을 쓰기 때문이다. 유니코드 명령줄이 필요하면 `GetCommandLineW()`로 읽는다.

## 4. 렌더링 백엔드

- **그래픽스 API** — D3D11로 시작한다.
- **API 코드 격리** — D3D11 타입(`ID3D11Device` 등)과 `d3d11.h`는 렌더러 안의 D3D11 전용 폴더에서만 쓴다. 렌더러의 나머지 코드와 그 위의 코드는 엔진 타입만 쓴다.

  ```text
  Engine/
  └─ Private/
     └─ Renderer/
        ├─ Renderer.cpp          // 엔진 타입만 사용
        └─ D3D11/
           └─ D3D11Device.cpp    // ID3D11Device 등은 여기에서만
  ```

## 5. 월드와 씬

- **용어** — 같은 단어가 두 뜻으로 쓰이지 않게 나눈다.

  | 용어 | 뜻 | 이름 |
  |---|---|---|
  | World | 게임의 원본. 액터, 컴포넌트, 게임 로직 | 월드 타입 하나 (이름은 월드 구현 때 확정) |
  | `<시스템>Scene` | 각 시스템이 월드를 자기 용도로 본 사본 | `NRenderScene`, 나중에 `NPhysicsScene` |
  | Level | 저장하고 불러오는 콘텐츠 단위 (맵) | 필요해지면 만든다 |

  - 접두어 없는 `NScene`은 쓰지 않는다. 씬 이름에는 항상 시스템 이름을 붙인다.
  - 물리 쪽 씬은 충돌 검사와 시뮬레이션을 함께 담당하고 `Physics`로 이름을 통일한다.
- **소유와 통신** — 월드가 시스템별 씬을 소유한다. 컴포넌트는 자기와 관련된 씬에만 등록한다(메시 컴포넌트는 렌더 씬, 충돌 컴포넌트는 물리 씬). 씬끼리는 직접 통신하지 않는다. 물리 결과는 월드의 컴포넌트에 반영되고, 컴포넌트가 렌더 씬을 갱신한다(1장 "같은 계층끼리 의존 금지").

## 6. 수학과 좌표계

- **좌표계** — Unreal과 같다. 엔진 전체(게임 코드, 에셋, 렌더러 공개 API)가 이 규약을 쓴다.

  | 항목 | 규약 |
  |---|---|
  | 축 | X 앞, Y 오른쪽, Z 위 |
  | 손잡이 | 왼손 |
  | 벡터 곱 | 행 벡터. `v * M`, 변환은 적용 순서대로 왼쪽에서 오른쪽으로 곱한다 (`world * view * projection`) |
  | 행렬 저장 | 행 우선 (이동 값이 마지막 행) |
  | 단위 | 1 = 1 cm |
  | 정밀도 | `float` |

  - 다른 축 규약을 쓰는 데이터(OBJ, glTF 등 Y-up 에셋)는 불러오는 곳 한 곳에서 변환한다.
  - 그래픽스 API 쪽 차이(HLSL 행렬 배치, 깊이 범위)는 렌더러가 처리한다.
- **수학 타입** — 엔진 타입(`NVector2`, `NVector3`, `NVector4`, `NQuat`, `NMatrix4`, `NTransform`, 함수 모음 `NMath`)을 `Core/Public/Math/`에 둔다. 엔진 코드와 공개 API는 이 타입만 쓴다.
  - 계산은 DirectXMath로 구현한다. DirectXMath는 Windows SDK판이 아니라 vcpkg로 받은 것을 쓴다(Project Settings "vcpkg"). 함수를 하나씩 직접 구현으로 바꿀 수 있다. 사용처는 엔진 타입만 쓰므로 바꿀 때 고치지 않는다.
  - 타입은 `float` 멤버만 가진다. SIMD 타입(`XMVECTOR`, `XMMATRIX`)은 정렬 제약이 있어 멤버로 두지 않고, 계산할 때만 쓴다.
  - DirectXMath는 수학 타입의 구현 파일에서만 include한다. 공개 헤더에 `DirectX` 타입이 드러나지 않게 한다. 인라인이 필요할 만큼 호출 비용이 문제가 되면 그때 다시 검토한다.
- **트랜스폼 저장** — 컴포넌트와 에셋은 위치, 회전, 크기를 `NTransform`으로 저장한다. 행렬은 필요할 때 만든다(렌더 씬에 넘길 때 등).
- **위치와 방향 변환** — 점은 `TransformPosition`(이동 적용), 방향은 `TransformVector`(이동 무시)로 나눠 변환한다. 행렬 곱셈 연산자는 수학 타입과 렌더러 안에서 쓸 수 있다.
