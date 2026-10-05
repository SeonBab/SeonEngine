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
  class IOutputDevice
  {
  public:
  	virtual ~IOutputDevice() = default;
  	virtual void Write(const FLogRecord& record) = 0;
  };

  // Editor — 구현해서 Core에 등록 (에디터 로그 창이 기록을 받아 보관)
  class FEditorLogOutputDevice final : public IOutputDevice
  {
  public:
  	void Write(const FLogRecord& record) override;
  };
  ```

## 2. 서브시스템과 전역 상태

- **싱글톤 금지** — 클래스가 스스로 인스턴스를 들고 있는 싱글톤(`GetInstance()`)은 만들지 않는다. 생성 / 파괴 시점을 제어할 수 없어 아래 초기화 / 종료 순서를 깨뜨린다. 서브시스템은 `FEngine`이 소유한다. 로그 전달기에만 아래의 명시적 예외를 둔다.
- **로그 전달기 예외** — `FOutputDeviceRedirector`는 `FOutputDeviceRedirector::Get()`(참조 반환) 안의 `static` 객체 하나를 공유하고, 출력 장치 목록은 `TArray`로 관리한다. 생성자는 private이고 복사 / 이동은 막아서 `Get()` 밖에서 전달기를 만들거나 복사하지 못한다. 생성자는 빈 전달기 상태만 준비하며 파일 열기나 같은 로그 경로 호출을 하지 않는다. 엔진 시작 코드가 실제 장치의 준비 / 등록 / 해제 / 종료를 관리한다. 전달기는 장치를 소유하지 않는다.
- **초기화 / 종료 순서** — `FEngine::Initialize()` 한 곳에서 초기화 순서를 명시하고, `FEngine::Shutdown()`은 정확히 그 역순으로 정리한다. 전역 / static 객체의 생성자에서는 파일 열기 등 외부 자원 초기화 작업을 하지 않는다(파일 사이의 전역 동적 초기화 순서에 의존하지 않는다).
- **로그 호출 범위** — 장치 등록 전에는 출력 / 보존을 보장하지 않는다. 종료 로그는 장치 해제 전에 남긴다. 전역 객체의 소멸자에서 로그를 사용하지 않으며, 전달기 소멸 이후 접근은 허용하지 않는다. 현재는 단일 스레드에서 전달하고 `Write` 도중 목록을 변경하지 않는다. 최초 `static` 초기화 보호는 이후 로그 처리의 스레드 안전을 보장하지 않는다.

  ```cpp
  bool FEngine::Initialize()
  {
  	if (!platform->Initialize()) { return false; }
  	if (!renderer->Initialize(rendererDesc)) { return false; }
  	if (!input->Initialize()) { return false; }
  	return true;
  }

  void FEngine::Shutdown()
  {
  	input->Shutdown();
  	renderer->Shutdown();
  	platform->Shutdown();
  }
  ```

- **서브시스템 접근** — 타입으로 조회한다. 서브시스템은 `FEngine::Initialize()`에서 등록한다.

  ```cpp
  FEngine* gEngine = nullptr;

  IRenderer* renderer = gEngine->GetSubsystem<IRenderer>();

  // 금지
  IRenderer& renderer = IRenderer::GetInstance();
  ```

- **전역 변수** — 새 전역 변수는 만들지 않는다. 이 문서에 적힌 엔진 기반 전역(`gEngine`)과 로그 전달기의 공유 상태 예외만 둔다. 상수(`constexpr`)와 `.cpp` 안의 익명 namespace 변수는 허용한다. 필요한 객체는 가능하면 생성자나 함수 인자로 넘겨받는다.

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

- **구현 방식** — 지원 플랫폼이 하나인 동안은 공통 헤더에 선언하고 플랫폼별 `.cpp`에서 구현한다. 플랫폼이 두 개 이상이 되면 `Generic` 공통 구현을 두고 플랫폼 구현이 상속한 뒤 `using`으로 고르는 방식으로 바꾼다. 두 방식 모두 사용하는 코드는 `FPlatformFile`이라는 같은 이름을 쓰므로 전환할 때 사용처를 고치지 않는다.

  ```cpp
  // 플랫폼이 하나일 때
  // PlatformFile.h
  class FPlatformFile
  {
  public:
  	static bool Exists(const FString& path);
  };

  // Windows/WindowsPlatformFile.cpp
  bool FPlatformFile::Exists(const FString& path)
  {
  	...
  }

  // 플랫폼이 둘 이상일 때
  struct FGenericPlatformFile { ... };
  struct FWindowsPlatformFile : FGenericPlatformFile { ... };

  using FPlatformFile = FWindowsPlatformFile;
  ```

- **이름** — 객체를 만들지 않고 정적 함수로 쓰는 플랫폼 서비스는 `FPlatform` + 이름(`FPlatformFile`, 플랫폼 구현은 `FWindowsPlatformFile`)으로 짓는다. 인스턴스를 만들어 쓰는 플랫폼 객체는 `Platform`을 붙이지 않는다(`FWindow`, 플랫폼 구현 파일은 `Windows/WindowsWindow.cpp`, 플랫폼이 둘 이상이 되면 `FGenericWindow` / `FWindowsWindow`).

- **문자열 변환** — Windows API 경계의 UTF-8 → UTF-16 변환은 `Platform/Windows/WindowsString.h`의 `FWindowsString`으로 한다. Windows 구현 파일끼리만 쓰는 도우미라 Private에 두고, HAL(`FPlatform…`)이 아니다.
- **`Windows.h`** — 직접 include하지 않고 `Platform/Windows/WindowsHWrapper.h` 래퍼만 include한다. 래퍼는 Platform과 그래픽스 API 전용 폴더(4장 렌더링 참고)의 `.cpp`(또는 Private 헤더)에서만 include하고, Public 헤더에서는 include하지 않는다.
  - 래퍼는 `WIN32_LEAN_AND_MEAN`, `NOMINMAX`를 정의한 뒤 `Windows.h`를 include하고, 엔진 이름과 충돌하는 매크로(`CreateWindow` 등)를 `#undef`한다.

  ```cpp
  // Platform/Windows/WindowsHWrapper.h
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

## 4. 렌더링

- **상태** — 이 장은 렌더러를 만들 때 지킬 규칙이다. 아직 구현되지 않은 부분은 목표로 읽는다.

  | 구분 | 내용 |
  |---|---|
  | 현재 | 렌더링 코드 없음 |
  | 다음 | 렌더러 인터페이스, D3D11로 만드는 SeonEngine 렌더러 |
  | 장기 | D3D12 백엔드를 시작할 때 RHI 추출, 이후 Vulkan ([Deferred Tasks](DeferredTasks.md) "RHI") |

- **경계 두 개** — 렌더러를 통째로 바꾸는 경계와, SeonEngine 렌더러 아래에서 그래픽스 API를 바꾸는 경계를 나눈다.

  ```text
  Engine (월드, 컴포넌트)
    │  렌더러 인터페이스만 사용
    ▼
  렌더러 인터페이스        IRenderer / IRenderScene / FRenderView      ← 렌더러 교체
    ├─ SeonEngine 렌더러
    │    ▼
    │   RHI (D3D12 때 도입) ─ D3D11 / D3D12 / Vulkan 백엔드          ← 그래픽스 API 교체
    │
    └─ 다른 프로젝트의 렌더러 (예: 외부 렌더러를 감싼 어댑터)
  ```

  - **렌더러 인터페이스**는 엔진과 렌더러 사이의 계약이다. SeonEngine을 쓰는 다른 프로젝트가 자기 렌더러로 이 계약을 구현해 끼울 수 있다. 그 구현과 어댑터는 그 프로젝트에 둔다. SeonEngine은 특정 외부 렌더러를 모른다.
  - **RHI**는 SeonEngine 렌더러 안의 경계다. 다른 렌더러 구현은 RHI를 쓰지 않아도 된다.
  - 렌더러를 교체할 때 엔진 코드의 변경을 최소화하는 것이 목표다. 새 기능(스키닝, 파티클, 여러 뷰 등)이 필요해지면 인터페이스는 확장될 수 있다.
- **의존 방향** — 엔진과 렌더러 구현은 계약(렌더러 인터페이스와 그 자료형)만 공유한다. 엔진은 구현 클래스를 모르고, 렌더러 구현은 엔진 타입(월드, 컴포넌트)을 모른다. 게임이 구현을 골라 연결한다.

  ```text
  Engine ──────→ 렌더러 계약 ←────── SeonEngine 렌더러
                     ↑
           다른 프로젝트의 어댑터 ──→ 외부 렌더러
  ```

  - 계약 헤더는 월드, 컴포넌트, 그래픽스 API 헤더, `Windows.h`를 include하지 않는다. 모듈이 `Engine` 하나인 동안은 이 규칙을 헤더 단위로 지킨다.
- **렌더러 선택** — 게임이 시작할 때 렌더러 구현을 만들어 엔진에 넘긴다. 엔진은 넘겨받은 구현을 소유하고 서브시스템으로 등록한다(2장). 게임이 고르지 않으면 SeonEngine 렌더러를 쓴다.

  ```cpp
  // 설계 예시 (미구현). 넘기는 지점은 게임을 엔진에 연결하는 방식과 함께 정한다
  engineDesc.renderer = MakeUnique<FMyGameRenderer>();
  ```

  - 실행 중 렌더러 교체와 DLL 바이너리 호환은 약속하지 않는다.
- **인터페이스 구성** — 렌더러, 씬, 뷰를 나눈다.

  | 타입 | 역할 | 개수 |
  |---|---|---|
  | `IRenderer` | 렌더러 입구. 초기화 / 종료, 렌더 리소스(메시, 텍스처, 머티리얼) 생성과 해제, 씬 생성, `Render(scene, view)` | 엔진에 하나 (서브시스템) |
  | `IRenderScene` | 무엇을 그리나. 오브젝트와 조명을 등록 / 갱신 / 제거 | 월드마다 하나 (월드가 소유) |
  | `FRenderView` | 어디서 보나. 뷰 행렬, 카메라 파라미터(화각, near / far, 종횡비), 뷰포트 | 매 프레임 만드는 값 (구조체) |

- **등록형** — 오브젝트는 한 번 등록하고 바뀐 것만 갱신한다. 엔진이 매 프레임 전체 목록을 다시 제출하지 않는다. 매 프레임 그리기 요청이 필요한 렌더러는 구현 안에서 등록된 목록으로 요청을 만든다.

  ```cpp
  // 설계 예시 (미구현)

  // 월드 생성 시: 월드가 씬을 소유한다
  TUniquePtr<IRenderScene> renderScene = renderer->CreateScene();

  // 리소스 준비
  FMeshHandle mesh = renderer->LoadMesh("Box.obj");
  FMaterialHandle material = renderer->CreateMaterial(materialDesc);

  // 컴포넌트 등록 시
  FRenderObjectID objectID = renderScene->AddObject(mesh, material, worldMatrix);

  // 트랜스폼이 바뀌었을 때
  renderScene->UpdateTransform(objectID, worldMatrix);

  // 매 프레임
  renderer->Render(*renderScene, view);

  // 컴포넌트 해제 시
  renderScene->RemoveObject(objectID);

  // 리소스를 더 쓰지 않을 때
  renderer->ReleaseMaterial(material);
  renderer->ReleaseMesh(mesh);
  ```

- **머티리얼** — 엔진은 재질의 성질(값과 텍스처)만 넘긴다. 셰이더, 리소스 슬롯 번호, 블렌딩 같은 그리는 방법은 넘기지 않고 렌더러가 정한다. 그래야 같은 머티리얼을 다른 렌더러도 그릴 수 있다.
  - 처음 지원 범위는 작게 잡는다. 후보는 기본 색상과 색상 텍스처다. 외부 렌더러가 표현할 수 있는지 확인한 뒤 확정한다. 금속성, 거칠기 같은 값은 해당 조명 모델을 구현할 때 추가한다.
  - 원본 재질 정보는 엔진 쪽에 둔다. 렌더러는 받은 정보로 렌더링용 머티리얼을 만들어 핸들을 돌려준다. 엔진의 머티리얼과 렌더러 머티리얼 핸들의 대응은 엔진의 렌더링 연동 코드가 관리한다. 렌더러는 엔진의 리소스 시스템을 모른다.
  - 렌더러마다 결과가 완전히 같다는 것은 약속하지 않는다. 같은 성질을 각자의 셰이더로 표현한다.
- **데이터 전달** — 렌더러는 호출 중에 받은 값과 배열을 복사한다. 호출이 끝난 뒤 엔진 메모리를 참조하지 않는다.
- **씬 소유** — `CreateScene()`은 소유 포인터를 돌려주고 월드가 보관한다. 월드가 파괴될 때 씬도 파괴된다. 씬은 렌더러보다 먼저 파괴한다. `IRenderer::Shutdown()` 때 살아 있는 씬이 있으면 assert로 막는다.
- **필수 기능과 선택 기능** — 모든 구현은 필수 기능을 지원한다. 선택 기능은 렌더러가 지원 여부를 알려 주고, 엔진은 확인한 뒤 쓴다.

  | 구분 | 기능 |
  |---|---|
  | 필수 | 씬 하나, 뷰 하나, 정적 메시, 머티리얼 처음 지원 범위, 오브젝트 등록 / 갱신 / 제거, 창 크기 변경 |
  | 선택 | 씬 여러 개, 뷰 여러 개, 그 밖에 이후 추가되는 기능 |

  - 지원하지 않는 선택 기능을 확인 없이 호출하면 프로그래머 실수로 본다(assert).
- **리소스 핸들과 수명**
  - 외부 핸들은 인덱스 + 세대 번호로 만들고, `Load*` / `Create*` 호출마다 따로 발급한다. 같은 리소스를 두 번 로드해도 핸들은 둘이다.
  - `Release(H)`는 그 핸들 하나만 즉시 무효화한다. 같은 리소스를 가리키는 다른 핸들과 이미 등록된 오브젝트는 계속 쓸 수 있다. 등록된 오브젝트가 쓰는 리소스는 그 오브젝트를 제거할 때까지 유지된다.
  - 무효화된 핸들로 등록하거나 조회하면 assert로 막는다(프로그래머 실수).
  - `Release`는 GPU 메모리를 즉시 반환한다고 약속하지 않는다. 실제 해제는 GPU 사용이 끝난 뒤 구현이 한다.
- **오류 처리** — Code Convention 5.1의 분류를 따른다. Release 빌드에서는 assert 검사가 제거되므로, 외부 실패를 assert에만 맡기지 않는다.

  | 실패 | 예 | 처리 |
  |---|---|---|
  | 프로그래머 실수 | 잘못되거나 해제된 핸들 사용, 확인 없이 선택 기능 호출, 씬이 남은 채 종료 | assert |
  | 외부 실패 | 메시 / 텍스처 파일 없음, 셰이더 컴파일 실패 | 무효 핸들을 돌려주고 로그를 남긴다. 그릴 때는 렌더러의 대체 리소스(기본 텍스처 등)를 쓴다 |
  | 복구 불가 | 장치 / 스왑체인 생성 실패, 렌더러 초기화 실패 | 치명 로그를 남기고 종료한다 |

- **초기화와 종료** — 2장의 순서를 따른다.
  - 렌더러 초기화 실패는 복구 불가로 다룬다. 엔진은 `Shutdown()`을 부르지 않고 치명 로그를 남긴 뒤 종료한다. 그래서 구현은 부분 초기화 상태의 `Shutdown()`을 안전하게 만들 필요가 없다.
  - 렌더러를 종료한 뒤 다시 초기화하는 것은 지원하지 않는다.
- **창과 출력 대상** — 창은 플랫폼이 소유한다. 렌더러는 초기화할 때 출력 대상(창의 네이티브 핸들과 크기)을 받고, 창 크기가 바뀌면 엔진이 렌더러에 알린다. 넘기는 형태(엔진 창 객체 / 네이티브 핸들 `void*`)는 구현할 때 정한다.
- **경계 규칙** — 렌더러를 교체할 때 엔진 코드의 변경을 최소화하기 위해 지킨다.
  - 인터페이스에 특정 렌더러의 개념과 ID 타입을 쓰지 않는다. 엔진 쪽 핸들을 따로 정의하고, 구현이 자기 ID로 바꾼다.
  - 인터페이스에 그래픽스 API 타입과 리소스 바인딩 개념(루트 슬롯 번호, 레지스터 번호)을 쓰지 않는다.
  - 셰이더 컴파일 방식을 인터페이스에 드러내지 않는다.
  - 렌더러와 렌더 씬은 메인 스레드에서만 호출한다. assert로 확인한다.
  - 렌더 씬 등록 / 갱신 / 제거는 컴포넌트의 등록 / 해제 / 트랜스폼 변경 지점에서만 한다. 게임 코드가 렌더 씬을 직접 호출하지 않는다.
  - 오브젝트 ID는 불투명한 값으로 둔다. 엔진은 ID가 내부에서 무엇을 가리키는지 모른다.
  - 투영 행렬은 렌더러가 `FRenderView`의 카메라 파라미터로 만든다. 엔진은 완성된 투영 행렬을 넘기지 않는다. 깊이 범위, reversed-Z, 지터처럼 백엔드나 렌더링 기법에 따라 달라지는 부분을 렌더러가 정하게 하기 위해서다.
  - 인터페이스에 `HWND` 등 Windows 타입을 쓰지 않는다(3장 `Windows.h` 참고).
- **SeonEngine 렌더러 내부 규칙**
  - D3D11 타입(`ID3D11Device` 등)과 `d3d11.h`는 렌더러 안의 D3D11 전용 폴더에서만 쓴다. RHI를 추출할 때 이 경계가 백엔드가 된다.
  - 렌더러 로직(패스, 정렬)과 그래픽스 API 호출을 한 클래스에 섞지 않는다.
  - 실제로 그리는 단위(메시 + 머티리얼 + 행렬 + 패스, 이하 "그릴 항목")를 따로 정의한다. 등록된 오브젝트는 이 단위로 바뀌어 정렬과 드로우에 쓰인다.
  - 셰이더에 행렬을 넘길 때의 변환(전치 또는 HLSL `row_major`)은 렌더러 안 한 곳에서만 한다.

  ```text
  Engine/
  ├─ Public/
  │  └─ Renderer/                렌더러 계약 (IRenderer, IRenderScene, FRenderView, 핸들)
  └─ Private/
     └─ Renderer/
        ├─ Renderer.cpp          SeonEngine 렌더러. 엔진 타입만 사용
        └─ D3D11/
           └─ D3D11Device.cpp    ID3D11Device 등은 여기에서만
  ```

## 5. 월드와 씬

- **용어** — 같은 단어가 두 뜻으로 쓰이지 않게 나눈다.

  | 용어 | 뜻 | 이름 |
  |---|---|---|
  | World | 게임의 원본. 액터, 컴포넌트, 게임 로직 | 월드 타입 하나 (이름은 월드 구현 때 확정) |
  | `<시스템>Scene` | 각 시스템이 월드를 자기 용도로 본 사본 | `IRenderScene`(렌더러 인터페이스, 4장), 나중에 `FPhysicsScene` |
  | Level | 저장하고 불러오는 콘텐츠 단위 (맵) | 필요해지면 만든다 |

  - 접두어 없는 `FScene`은 쓰지 않는다. 씬 이름에는 항상 시스템 이름을 붙인다.
  - 물리 쪽 씬은 충돌 검사와 시뮬레이션을 함께 담당하고 `Physics`로 이름을 통일한다.
- **오브젝트 모델** — 월드에 놓이는 오브젝트는 `FActor` 타입 하나로 두고, 기능은 컴포넌트 클래스(데이터 + 동작)를 붙여 만든다. 액터를 종류별로 상속해 기능을 넣지 않는다(Unreal 5의 `AActor` 서브클래스 방식과 다르다). ECS(ID 엔티티 + 데이터 컴포넌트 + 시스템)는 쓰지 않는다.
- **소유와 통신** — 월드가 시스템별 씬을 소유한다. 컴포넌트는 자기와 관련된 씬에만 등록한다(메시 컴포넌트는 렌더 씬, 충돌 컴포넌트는 물리 씬). 씬끼리는 직접 통신하지 않는다. 물리 결과는 월드의 컴포넌트에 반영되고, 컴포넌트가 렌더 씬을 갱신한다(1장 "같은 계층끼리 의존 금지").

## 6. 수학과 좌표계

- **좌표계** — Unreal의 새 규약인 LUF(Left-Up-Forward)를 쓴다. 엔진 전체(게임 코드, 에셋, 렌더러 인터페이스)가 이 규약을 쓴다.

  | 항목 | 규약 |
  |---|---|
  | 축 | X 왼쪽, Y 위, Z 앞 (LUF) |
  | 손잡이 | 오른손 |
  | 회전 방향 | 오른손 법칙. 축의 + 방향에서 원점을 볼 때 반시계가 + |
  | 앞면 | 앞에서 볼 때 정점 순서가 반시계(CCW)인 삼각형 |
  | 뷰 공간 | 뷰 행렬은 카메라 월드 트랜스폼의 역행렬이다. 뷰 공간도 LUF(카메라 기준 X 왼쪽, Y 위, Z 앞)다 |
  | 벡터 곱 | 행 벡터. `v * M`, 변환은 적용 순서대로 왼쪽에서 오른쪽으로 곱한다 (`world * view * projection`) |
  | 행렬 저장 | 행 우선 (이동 값이 마지막 행) |
  | 단위 | 1 = 1 cm |
  | 정밀도 | `float` |

  - 다른 규약을 쓰는 데이터는 불러오는 곳 한 곳에서 변환한다. glTF는 축이 같아 단위(m → cm)만 바꾼다.
  - 그래픽스 API 쪽 차이(D3D의 왼손 뷰 공간과 앞면 판정, HLSL 행렬 배치, 깊이 범위)는 렌더러가 처리한다.
- **수학 타입** — 엔진 타입(`FVector2`, `FVector3`, `FVector4`, `FQuat`, `FMatrix4`, `FTransform`, 함수 모음 `FMath`)을 `Core/Public/Math/`에 둔다. 엔진 코드와 공개 API는 이 타입만 쓴다.
  - 계산은 DirectXMath로 구현한다. DirectXMath는 Windows SDK판이 아니라 vcpkg로 받은 것을 쓴다(Project Settings "vcpkg"). 함수를 하나씩 직접 구현으로 바꿀 수 있다. 사용처는 엔진 타입만 쓰므로 바꿀 때 고치지 않는다.
  - 타입은 `float` 멤버만 가진다. SIMD 타입(`XMVECTOR`, `XMMATRIX`)은 정렬 제약이 있어 멤버로 두지 않고, 계산할 때만 쓴다.
  - DirectXMath는 수학 타입의 구현 파일에서만 include한다. 공개 헤더에 `DirectX` 타입이 드러나지 않게 한다. 인라인이 필요할 만큼 호출 비용이 문제가 되면 그때 다시 검토한다.
- **트랜스폼 저장** — 컴포넌트와 에셋은 위치, 회전, 크기를 `FTransform`으로 저장한다. 행렬은 필요할 때 만든다(렌더 씬에 넘길 때 등).
- **위치와 방향 변환** — 점은 `TransformPosition`(이동 적용), 방향은 `TransformVector`(이동 무시)로 나눠 변환한다. 행렬 곱셈 연산자는 수학 타입과 렌더러 안에서 쓸 수 있다.
