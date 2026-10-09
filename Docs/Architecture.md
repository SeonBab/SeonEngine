# SeonEngine Architecture

엔진 구조에 대한 규칙이다. 코드를 어떻게 쓰는지는 [Code Convention](Conventions/CodeConvention.md)을 따른다.

---

## 1. 모듈과 의존성 방향

- **객체 기반 모듈 (최소 UObject 구현 완료)** — UObject는 CoreUObject에 배치한다. 의존 방향은 게임 → Engine → CoreUObject → Core이며 하위 모듈은 상위 모듈을 모른다. 헤더는 Engine/Source/CoreUObject/Public/UObject/Object.h, 구현 파일은 Engine/Source/CoreUObject/Private/UObject/Object.cpp로 둔다. 엔진 프로젝트 / Engine.lib 하나에 함께 빌드하며 별도 라이브러리나 DLL은 만들지 않는다. Object.h / Object.cpp와 프로젝트 / include 경로를 등록했다. Engine은 CoreUObject의 Public / Private, SampleGame은 Public만 사용한다. 구현과 검증은 EditorDecisions 1.12를 참고한다. 근거는 [EditorDecisions 1.11.12](../../Docs/EditorDecisions.md#11112-uobject-파일--모듈-배치-확정--구현-전)에 있다.

- **모듈 구성** — `Core`, `CoreUObject`, `Engine`과 게임 모듈로 시작하고, 코드가 커지면 나눈다(예: `Engine`에서 `Renderer`, `RHI`, `Platform` 분리). 의존은 위에서 아래로만 한다.

  ```text
  SampleGame  (게임 모듈, 콘텐츠 코드)
    │
    ▼
  Engine      (렌더링, 플랫폼, 입력, 씬 등 엔진 기능)
    │
    ▼
  CoreUObject (객체 기반, 현재 최소 UObject)
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
- **현재 모듈 도입 범위 (확정 / 구현 전)** — 엔진은 한 개의 정적 Engine.lib와 폴더별 의존 경계를 유지한다. 게임 프로젝트 연결 계약부터 작은 단위로 설계하고, 기능별 별도 라이브러리·범용 모듈 관리자·DLL 로딩·재로딩은 실제 필요가 생길 때 별도 도입한다. 게임 코드를 연결하는 계약과 프로젝트 / 게임 설정 제공, 게임 세션 실행을 구분하며 게임 프로젝트에 엔진 생성·메인 루프 구현을 요구하지 않는다. 구체 연결 API와 수명은 설계 중이다. 비교 근거와 재검토 조건은 [EditorDecisions 1.9](../../Docs/EditorDecisions.md#19-모듈-도입-범위와-비용-비교-2026-10-07--진행-범위-확정)에 있다.
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

- **싱글톤 금지** — 클래스가 스스로 인스턴스를 들고 있는 싱글톤(`GetInstance()`)은 만들지 않는다. 생성 / 파괴 시점을 제어할 수 없어 아래 초기화 / 종료 순서를 깨뜨린다. 서브시스템은 `UEngine`이 소유한다. 로그 전달기에만 아래의 명시적 예외를 둔다.
- **로그 전달기 예외** — `FOutputDeviceRedirector`는 `FOutputDeviceRedirector::Get()`(참조 반환) 안의 `static` 객체 하나를 공유하고, 출력 장치 목록은 `TArray`로 관리한다. 생성자는 private이고 복사 / 이동은 막아서 `Get()` 밖에서 전달기를 만들거나 복사하지 못한다. 생성자는 빈 전달기 상태만 준비하며 파일 열기나 같은 로그 경로 호출을 하지 않는다. 엔진 시작 코드가 실제 장치의 준비 / 등록 / 해제 / 종료를 관리한다. 전달기는 장치를 소유하지 않는다.
- **초기화 / 종료 순서** — 공통 실행 기반의 준비·전체 종료 순서는 `FEngineLoop`가 총괄하고, 게임 / 에디터의 실행 초기화·갱신·정리는 `UEngine` 계열의 계약으로 연결한다(방향 확정 / 구현 전). 각 담당자는 성공한 초기화 단계에 대응하는 순서로 정리하며, 구체 API와 부분 실패 책임은 설계 중이다. 전역 / static 객체의 생성자에서는 파일 열기 등 외부 자원 초기화 작업을 하지 않는다(파일 사이의 전역 동적 초기화 순서에 의존하지 않는다).

- **게임 / 에디터 실행 방향 (확정 / 구현 전)** — Unreal식으로 공통 실행 루프(`FEngineLoop`)와 공통 엔진 기반(`UEngine`), 게임용(`UGameEngine`) / 에디터용(`UEditorEngine`) 실행 객체를 분리한다. 별도 Editor 실행 파일도 같은 엔진 기반을 사용한다. 에디터 패널은 Editor에 두고 GPU 출력은 렌더러 내부에서 처리한다. 현재 EngineMain은 기존 창 / 렌더러 루프다. UEngine / UGameEngine과 FEngineLoop의 기본 세션 준비 / 종료는 구현했고 실제 진입점 / 프레임 연결과 UEditorEngine은 아직 없다. 최소 실행 API, 소유권 / 실패 정리 / 최소화 시 갱신 계약부터 확정하고 단계적으로 옮긴다. DLL / 모듈 로더 / GC / 리플렉션 / 전역 GEngine을 함께 채택하는 규칙은 아니다. 근거와 구현 순서는 [EditorDecisions 1.7](../../Docs/EditorDecisions.md#17-최종-방향--unreal식-엔진-실행-구조-2026-10-07)에 있다.
- **최소 객체 기반 (UObject·UEngine·UGameInstance 구현 완료 / 실제 실행 연결 전)** — CoreUObject의 UObject를 구현했다. public explicit UObject(UObject* inOuter = nullptr)는 cpp의 초기화 목록 outer(inOuter)로 소속만 저장한다. private UObject* outer는 비소유이며 생성 후 변경 API를 제공하지 않는다. public UObject* GetOuter() const는 헤더의 한 줄 getter로 저장 값을 반환하고 소속이 없으면 nullptr다. 대상은 수정 가능하며 수명 / 타입 검사는 하지 않는다. public virtual ~UObject() = default;는 기반 포인터를 통한 파생 파괴를 지원하지만 Outer 대상을 파괴하거나 Shutdown을 호출하지 않는다. 복사 생성 / 대입과 이동 생성 / 대입을 금지하고 객체 주소를 유지하는 TUniquePtr 소유권 이동은 허용한다. 일반 C++ 직접 생성은 허용하며 리플렉션 / GC / 생성 문맥 검사는 없다. UEngine은 이 UObject 기반으로 구현했다. UGameInstance의 생성자 / 기본 훅도 구현했다. UGameEngine의 기본 세션 소유 / 준비 / 종료를 구현했고 FEngineLoop의 준비 / 종료 호출도 연결했으며 향후 UEditorEngine 및 실제 진입점 / 프레임 연결은 아직 없다. UGameInstance의 UEngine& 생성자는 UObject(&inEngine)로 소속을 연결하며 실행 초기화는 하지 않는다. FEngineLoop / FWorldContext와 기존 FWorld / FActor / FActorComponent는 이번 기반에 편입하지 않는다. 근거와 구현 / 검증 상태는 [EditorDecisions 1.11~1.12](../../Docs/EditorDecisions.md#112-최소-uobject-구현과-검증-2026-10-07)에 있다.

- **실행 책임 배치 (방향 확정 / 연결 전)** — 진입점은 루프 시작·반복·종료로 연결하고 FEngineLoop는 공통 실행 기반 준비·엔진 생성·프레임 진행·전체 종료를 총괄한다. UEngine은 공통 계약·WorldContext, UGameEngine은 게임 창·뷰포트·GameInstance 구성과 게임 실행을 담당한다. 실제 OS / GPU 작업은 플랫폼·렌더러가 수행한다. Init 준비와 Start 실행 시작을 구분한다. FEngineLoop는 엔진 Init 뒤 Start를 호출하지만 현재 Start는 빈 기반 통로다. 실제 맵 / 게임 실행 시작은 후속이다. 현재 진입점의 창 / 렌더러 생성·기존 EngineMain을 목표 배치로 고정하지 않으며 실제 연결은 아직 없다. 구체 소유 타입·공통 GPU 기반과 게임별 출력 대상의 경계·시간 갱신·실패 / 종료 순서는 후속 계약이다. TUniquePtr를 사용하되 GC·Slate·RHI 전체 구현을 선행 조건으로 두지 않는다. 근거와 미채택 임시 배치는 [EditorDecisions 1.15](../../Docs/EditorDecisions.md#115-unreal-목표에-맞춘-실행-책임-배치-2026-10-08--방향-확정연결-전)에 있다.
- **공통 엔진 실행 기반 (최소 구현 완료 / 실제 실행 연결 전)** — Engine/Public/Engine/Engine.h의 UEngine은 UObject 파생 추상 클래스다. 생성자는 null Outer를 준비하고 virtual void Init(IEngineLoop*)는 유효 루프를 private IEngineLoop* engineLoop = nullptr;에 비소유 저장한다. null 인수는 Fatal이다. virtual void Tick(float deltaSeconds, bool bIdleMode) = 0;은 파생 실행을 요구하며 시간 측정 / 유휴 정책은 제공하지 않는다. virtual void Start()는 준비 이후 별도의 실행 시작 통로이며 기본 구현은 비어 있고 Init / 생성자가 자동 호출하지 않는다. virtual void PreExit()의 현재 기반 구현은 빈 동작이고 UEngine 소멸자는 잔여 컨텍스트 연결 / 소유 월드를 정리하되 PreExit나 세션 Shutdown을 호출하지 않는다. 파생 Init은 기반을 먼저, 파생 PreExit는 필요한 정리 후 기반을 호출한다. 루프는 사용하는 동안 유효해야 한다. 중복 호출 / 상태 머신 / 부분 초기화 실패 계약은 후속이다. Engine.h는 IEngineLoop 전방 선언을 유지하되 Public/Engine/EngineLoop.h에 실제 최소 인터페이스를 정의했다. FEngineLoop의 엔진 준비 / 종료와 UGameEngine의 세션 준비 / 종료는 구현했고 프레임은 아직 연결하지 않았다. 직접 루프 멤버 접근이 필요해지면 public 전환을 재검토한다. 설계와 검증은 [EditorDecisions 1.13~1.14](../../Docs/EditorDecisions.md#114-uengine-최소-실행-기반-구현-2026-10-08)에 있다.

- **게임 세션 기반 (최소 준비 / 조회 / 정리 구현 완료·실행 연결 전)** — UGameInstance의 UEngine& 생성자는 엔진을 비소유 Outer로 연결하고 실행 준비를 하지 않는다. GetEngine은 RTTI 타입 확인 후 비소유 반환하며 불일치하면 Fatal이다. GetWorld는 준비 전 / 연결 해제 후 null이다. non-virtual void InitializeStandalone()은 컨텍스트 요청·세션 연결·빈 월드 생성 / 연결 후 virtual void Init()을 호출한다. 한 세션 객체는 한 번만 준비하고 중복 / 재진입 / 제거 후 재준비는 Fatal이다. 새 세션은 기존 컨텍스트 제거 후 준비 가능하다. 파생 Shutdown은 프로젝트 정리 후 기반을 호출하고 기반은 비소유 연결만 해제한다. 소멸자는 내부 연결만 해제하며 Shutdown을 자동 호출하지 않는다. UGameEngine이 기본 GameInstance를 단독 소유하고 Init / PreExit에서 준비 / 종료를 호출한다. FEngineLoop에서 Init / Start / PreExit를 호출하지만 Start는 빈 통로이며 실제 프레임과 프로젝트별 세션 선택은 후속이다. 세션 훅은 void이며 맵 / GPU 준비 결과를 뜻하지 않는다. 근거와 검증은 [EditorDecisions 1.18](../../Docs/EditorDecisions.md#118-컨텍스트월드세션-준비--조회--정리-구현-2026-10-08)에 있다.
- **실행 루프 (엔진 준비 / 종료 구현 완료)** — 공개 IEngineLoop는 가상 소멸자와 int32 Init만 제공한다. Private/Launch의 FEngineLoop는 TUniquePtr<UEngine>를 소유하고 Init에서 UGameEngine 생성 → 엔진 Init(this) → Start → 0 반환을 수행한다. 기존 엔진이 있는 Init은 Fatal이다. Exit는 PreExit 후 엔진 reset이며 준비 전 / 반복 호출을 허용한다. 루프 복사 / 이동은 금지하고 GetEngine은 내부 비소유 조회로 Exit 후 무효다. 소멸자는 Exit를 자동 호출하지 않으며 정상 실행 정리는 명시적 Exit를 사용한다. 실제 EngineMain / 전역 루프·Tick / 시간·창 / GPU 연결은 후속이다. 근거와 검증은 [EditorDecisions 1.20](../../Docs/EditorDecisions.md#120-실행-루프의-엔진-수명-연결-2026-10-08--준비종료-구현-완료)에 있다.
- **게임용 엔진 (기본 세션 준비 / 종료 구현 완료)** — UGameEngine은 UEngine 파생이며 private TUniquePtr<UGameInstance>를 소유한다. Init은 기존 세션이 있으면 Fatal, 기반 Init 후 기본 세션 생성 / InitializeStandalone을 호출한다. GetGameInstance는 비소유 조회이며 준비 전 / 종료 후 null이다. PreExit는 월드 주소 저장 → Shutdown → DestroyWorldContext → 세션 reset → 기반 PreExit 순서이며 준비 전 / 반복 종료를 허용한다. 소멸자는 종료 훅을 자동 호출하지 않는다. 세션의 별도 Shutdown / 컨텍스트 관리는 일반 게임 코드의 책임이 아니며 정상 종료는 PreExit를 사용한다. Tick은 빈 구현이고 Start는 빈 기반 통로를 상속한다. FEngineLoop의 준비 / 종료 호출은 연결했고 실제 EngineMain·프레임·창 / GPU·프로젝트별 세션 설정은 아직 연결하지 않았다. 구현과 검증은 [EditorDecisions 1.19](../../Docs/EditorDecisions.md#119-ugameengine의-기본-세션-준비--명시적-종료-연결-2026-10-08)에 있다.
- **월드 실행 관리 (초기 단일 컨텍스트 구현 완료)** — UEngine은 TUniquePtr<FWorldContext> 하나를 소유하고 컨텍스트는 TUniquePtr<FWorld> 현재 월드를 소유한다. 세션과 컨텍스트 / 월드의 연결은 비소유다. CreateNewWorldContext는 생성 / 소유 참조를 반환하고 기존 컨텍스트가 있으면 Fatal이다. DestroyWorldContext(FWorld*)는 일치하는 현재 월드의 연결을 해제한 뒤 컨텍스트 / 월드를 제거하며 일치하지 않거나 컨텍스트가 없으면 변경하지 않는다. 제거는 Shutdown을 자동 호출하지 않는다. 정상 Shutdown 중 필요한 월드는 유지하고 이후 실행 담당이 제거한다. FWorld의 SetGameInstance / GetGameInstance는 비소유 연결 / 조회다. GameInstance / 컨텍스트 제거 중 연결을 정리해 잔여 포인터를 막고 UEngine 소멸자는 잔여 컨텍스트를 정리하지만 정상 실행 종료 훅을 대체하지 않는다. 조회 포인터 / 컨텍스트 참조는 대상 제거 후 무효다. UGameEngine의 세션·월드 종료는 구현했고 여러 월드 목록·GC·맵 로딩·루프 / GPU 전체 종료는 후속이며 Unreal GC 메모리 관리와 동일하지 않다.

- **로그 호출 범위** — 장치 등록 전에는 출력 / 보존을 보장하지 않는다. 종료 로그는 장치 해제 전에 남긴다. 전역 객체의 소멸자에서 로그를 사용하지 않으며, 전달기 소멸 이후 접근은 허용하지 않는다. 현재는 단일 스레드에서 전달하고 `Write` 도중 목록을 변경하지 않는다. 최초 `static` 초기화 보호는 이후 로그 처리의 스레드 안전을 보장하지 않는다.

  현재 구현은 EngineMain의 창 / 렌더러 루프다. 최초 실행 역할은 FEngineLoop / UEngine / UGameEngine이며 게임 세션 역할로 작은 UGameInstance도 설계한다. UEditorEngine은 에디터 단계에서 추가한다. UGameInstance 준비 / 조회 / 정리와 UGameEngine의 세션 호출 연결은 구현했고 FEngineLoop의 준비 / 종료 호출도 구현했고 실제 진입점 / 프레임 연결은 후속이다. 창 / 렌더러 / 엔진 객체의 소유권과 함수 형태는 아직 미정이다. UEngine이 플랫폼과 GPU 기반의 전체 수명을 담당하는 이전 예시는 새 책임 분담에 적용하지 않는다. 구체 이전 범위와 비교 근거는 [EditorDecisions 1.8.1](../../Docs/EditorDecisions.md#181-책임-구분)에 있다.

- **서브시스템 접근** — 타입으로 조회한다. 서브시스템 등록은 이후 UEngine::Init의 공통 준비 단계에서 연결한다. 현재 최소 Init은 루프 포인터 저장만 수행하며 서브시스템 등록 / 조회는 미구현이다.

  ```cpp
  UEngine* gEngine = nullptr;

  IRenderer* renderer = gEngine->GetSubsystem<IRenderer>();

  // 금지
  IRenderer& renderer = IRenderer::GetInstance();
  ```

- **전역 변수** — 새 전역 변수는 만들지 않는다. 이 문서에 적힌 엔진 기반 전역(`gEngine`)과 로그 전달기의 공유 상태 예외만 둔다. 상수(`constexpr`)와 `.cpp` 안의 익명 namespace 변수는 허용한다. 필요한 객체는 가능하면 생성자나 함수 인자로 넘겨받는다.

## 3. 플랫폼 추상화

- **분기 코드 위치** — 플랫폼별 코드(OS API를 부르는 코드)는 `Engine/Private/Platform/` 폴더에만 둔다. 예외는 HAL의 플랫폼 선언 헤더(`Core/Public/<플랫폼>/`, 아래 "구현 방식")로, OS 헤더를 include하지 않고 선언과 `using`만 둔다. 그 외에는 Core의 플랫폼 매크로 정의 헤더만 `#if SE_PLATFORM_*` 분기를 쓸 수 있다. 일반 코드는 플랫폼에 따라 나뉘지 않는다.

  ```text
  Engine/Source/
  ├─ Core/Public/
  │  ├─ HAL/PlatformMisc.h                    입구 (쓰는 코드가 include)
  │  ├─ GenericPlatform/GenericPlatformMisc.h 모든 플랫폼이 제공할 함수 목록과 설명
  │  └─ Windows/WindowsPlatformMisc.h         Windows 선언, using FPlatformMisc = FWindowsPlatformMisc
  └─ Engine/Private/Platform/Windows/
     └─ WindowsPlatformMisc.cpp               Windows 본문 (OS API 호출)
  ```

- **플랫폼별 상수** — 줄바꿈(`LineTerminator`)처럼 플랫폼마다 값이 다른 C++ 상수는 플랫폼별 헤더(`Core/Public/Windows/WindowsPlatform.h` 등)에 같은 이름의 `constexpr` 전역 상수로 정의한다. 플랫폼마다 반드시 정의한다(기본값 없음). 쓰는 코드는 입구 `Core/Public/HAL/Platform.h`만 include한다. 여러 플랫폼의 값을 함께 다루거나 공통 기본값을 묶어 관리해야 하면 struct(`FPlatformProperties`) 도입을 검토한다.
- **플랫폼 헤더 고르기** — 입구 헤더는 `HAL/PreprocessorHelpers.h`의 `SE_COMPILED_PLATFORM_HEADER`로 빌드하는 플랫폼의 헤더를 include한다. 플랫폼별 `#if` / `#elif`를 입구마다 쓰지 않는다.

  ```cpp
  // HAL/PlatformMisc.h
  #include "GenericPlatform/GenericPlatformMisc.h"
  #include "HAL/PreprocessorHelpers.h"

  #include SE_COMPILED_PLATFORM_HEADER(PlatformMisc.h)    // Windows 빌드: "Windows/WindowsPlatformMisc.h"
  ```

  - 플랫폼 이름(`SE_PLATFORM_HEADER_NAME`)은 빌드 설정이 준다([Project Settings](ProjectSettings.md)). 없으면 `HAL/PreprocessorHelpers.h`가 `#error`로 멈춘다.
  - 플랫폼 헤더는 `<플랫폼>/<플랫폼><이름>.h`(예: `Windows/WindowsPlatformMisc.h`)로 둔다.
  - 플랫폼 헤더는 맨 위에서 자기 플랫폼 빌드인지 확인한다. 플랫폼 값과 이름이 어긋나면 여기서 멈춘다.

    ```cpp
    #if !SE_PLATFORM_WINDOWS
    #error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
    #endif
    ```
- **구현 방식** — 정적 함수로 쓰는 플랫폼 서비스(HAL)는 세 층으로 만든다. 지원 플랫폼이 하나여도 같다.
  - `GenericPlatform/Generic….h`: 모든 플랫폼이 제공할 함수를 선언하고 설명한다. 본문은 모든 플랫폼에서 같은 것만 둔다.
  - `<플랫폼>/<플랫폼>….h`: Generic을 상속하고 그 플랫폼이 구현하는 함수를 다시 선언한 뒤, `using`으로 공통 이름을 붙인다. 설명 주석은 Generic에만 둔다.
  - `HAL/….h`: 입구. 위 "플랫폼 헤더 고르기"로 플랫폼 헤더를 include한다. 쓰는 코드는 이것만 include한다.
  - 본문은 `Engine/Private/Platform/<플랫폼>/`의 `.cpp`에 `F<플랫폼>Platform…::함수`로 정의한다.

  ```cpp
  // GenericPlatform/GenericPlatformFile.h
  struct FGenericPlatformFile
  {
    /** 파일이 있는지 알려 준다. */
    static bool Exists(const FString& path);
  };

  // Windows/WindowsPlatformFile.h (OS 헤더 없이 선언만)
  struct FWindowsPlatformFile : public FGenericPlatformFile
  {
    static bool Exists(const FString& path);
  };

  using FPlatformFile = FWindowsPlatformFile;

  // Engine/Private/Platform/Windows/WindowsPlatformFile.cpp
  bool FWindowsPlatformFile::Exists(const FString& path)
  {
    ...
  }

  // 쓰는 코드
  #include "HAL/PlatformFile.h"
  FPlatformFile::Exists(path);
  ```

- **이름** — 객체를 만들지 않고 정적 함수로 쓰는 플랫폼 서비스는 `FPlatform` + 이름(`FPlatformFile`, 플랫폼 구현은 `FWindowsPlatformFile`)으로 짓는다. 인스턴스를 만들어 쓰는 플랫폼 객체는 `Platform`을 붙이지 않는다. 창은 공통 기반 `FGenericWindow`와 플랫폼 구현 `FWindowsWindow`로 나눈다. 공통 기반은 가상 함수와 가상 소멸자를 제공하며 공통 상태를 보관한다. 플랫폼 진입점은 `FWindowsWindow` 등 구체 창 객체의 수명을 소유하고 `EngineMain(FGenericWindow&, IRenderer&)`에 빌려준다. 현재 진입점은 기본 FRenderer도 생성해 공통 계약 참조로 전달한다. 공통 루프는 출력 가능 상태에서 최초 Init, Resize / RenderFrame을 호출하고 정상 종료 시 렌더러 자원 정리 뒤 창을 종료한다. 공통 진입점은 창 초기화 / 루프 / 명시적 종료를 수행하며, 객체 생성과 파괴는 플랫폼 진입점이 담당한다. `FWindow` 별칭과 `Platform/Window.h` 선택 입구는 사용하지 않는다. 두 클래스는 현재 Private에 두며, `PumpMessages()`를 애플리케이션 계층으로 옮기는 작업은 별도로 남긴다.

- **문자열 변환** — Windows API 경계의 UTF-8 ↔ UTF-16 변환은 `Platform/Windows/WindowsString.h`의 `FWindowsString`으로 한다. Windows 구현 파일끼리만 쓰는 도우미라 Private에 두고, HAL(`FPlatform…`)이 아니다.
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

  - 유휴 판단 / 대기 정책은 현재 적용하지 않는다. 향후 필요 시 엔진 루프 계층에서 판단과 실제 대기를 구분한다. 플랫폼 Sleep 서비스는 제공하지만 현재 루프에서는 호출하지 않는다. 렌더러 연결 시 최소화 / 크기 0의 그래픽스 호출 생략 조건은 유지한다.

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
  | 현재 | IRenderer / FRenderer와 메인 루프 연결, D3D11 별도 생성 / 종료 / Resize, 고정 삼각형 바인딩 / Draw·초기화·멤버 / 해제 제거 완료. 예제 HLSL 자동 빌드·배치는 유지하며 실행 .cso 의존 없음. 현재 FRenderer는 Device 장치 준비 / Clear와 Viewport Init / Resize / Present / Shutdown을 연결해 마젠타 배경을 표시한다. 기존 Device 출력 함수 / 멤버 삭제 완료, 장치 / 컨텍스트만 보유한다. Debug / Release 배경 픽셀 / 크기 변경 / 최소화·복원 / 종료 회귀 확인 |
  | 다음 | 장치 / 창 출력 분리 완료. 다음 기능 범위는 별도 결정. 실제 메시 렌더링 필요 시 셰이더 / 버퍼 자원 객체와 정점 형식·셰이더 조합의 입력 레이아웃 관리 확장. 카메라 / 깊이 / 텍스처와 초기화 실패 진단은 별도 후속 작업 |
  | 장기 | D3D12 백엔드를 시작할 때 RHI 추출, 이후 Vulkan ([Deferred Tasks](DeferredTasks.md) "RHI") |

- **그래픽스 장기 방향 (확정 / 자원 이동 전)** — Unreal식 공통 그래픽스 기반과 창별 출력 / 게임 실행 구성을 목표로 한다. 구현은 클래스를 하나씩 만들고 자원 / 동작을 작은 묶음으로 이동하며 설명한다. FD3D11Viewport 골격·생성자 필수 장치 연결과 Init / Shutdown / GetSwapChain 및 Resize / Present 선언을 추가했다. Resize는 void로 같은 크기 생략·GetDeviceContext / ClearState·RTV Reset·ResizeBuffers(수 / 형식 유지·flags 0)·RTV 재생성·성공 크기 갱신까지 구현했다. 버퍼 / 뷰 재구성 실패는 Fatal이며 새 Viewport의 실제 호출 연결 전이라 기존 Device.Resize 경로를 유지한다. 상세 진단 / 설정·복구·RHI 동기화 확장의 착수 조건은 DeferredTasks의 출력 재구성 확장을 따른다. Viewport의 private SizeX / SizeY는 0으로 시작하고 Init의 스왑체인·RTV 준비 성공 뒤 갱신하며 Shutdown에서 참조 해제 후 0으로 초기화한다. 두 값은 준비에 성공한 출력 크기다. Present는 [[nodiscard]] bool로 본문을 구현했으며 HRESULT 실패 시 Error / false, 그 외 true다. 실패 후 종료 / 복구 판단은 호출자가 하며 초기 연결 시 false는 호출부 Fatal로 처리할 방향이다. 실제 표시 완료 보증은 아니다. 기존 Device의 제출 / Fatal 호출은 유지하고 새 Viewport 호출은 아직 없다. Viewport에 빈 ComPtr<ID3D11RenderTargetView> BackBufferRenderTargetView 및 public GetBackBufferRenderTargetView / private nodiscard bool CreateBackBufferRenderTargetView 선언을 추가했다. CreateBackBufferRenderTargetView 본문은 GetBuffer / CreateRenderTargetView·실패 참조 정리 / Error·false 및 성공 true로 구현했다. 준비된 장치 / 스왑체인과 빈 RTV 참조를 전제로 하며 장치 / 스왑체인을 종료하지 않는다. GetBackBufferRenderTargetView 본문은 멤버 Get()을 반환하는 비소유 조회로 구현했다. Shutdown은 RTV Reset 후 SwapChain Reset한다. 출력 바인딩 / 외부 백버퍼 참조 정리는 호출자 책임이고 장치 종료 / GPU 완료 대기는 하지 않는다. Init의 스왑체인 성공 뒤 RTV 생성 helper를 호출하고 실패 시 Shutdown / false, 두 자원 준비 성공 시 true를 반환한다. FRenderer의 실제 Viewport 실행 연결은 아직 없다. 현재 RTV 직접 보관으로 시작하고 텍스처 자원 객체화는 후속으로 둔다. 빈 ComPtr<IDXGISwapChain> SwapChain 멤버와 cpp의 GetSwapChain(Get 비소유 조회) / Shutdown(Reset 참조 해제)을 구현했다. Init의 누락된 장치 / DXGI 조회·어댑터·Factory·CreateSwapChain 완료 흐름을 복원했다. 각 실패는 확보한 참조 정리 / false이며 생성 실패는 Viewport Shutdown / false, 성공은 true다. 실제 Init 호출 연결은 아직 없다. 현재 파일을 확인하며 후속 멤버 / 선언·본문 이동을 작은 단계로 진행한다. 스왑체인과 RTV 참조가 빈 상태에서 한 번 호출하는 계약이며 기존 실행 호출은 아직 연결하지 않았다. FD3D11Device::GetDevice() const는 내부 ID3D11Device의 비소유 포인터를 반환하도록 구현했다. public ID3D11DeviceContext* GetDeviceContext() const는 Direct3DDeviceIMContext.Get()을 반환하는 비소유 조회로 본문을 구현했다. 새 Viewport Resize 호출 연결은 아직 없다. 현재 소스를 기준으로 선언 단계부터 순서대로 구현한다. 새 Viewport의 실제 호출 연결은 없다. 장치는 private 비소유 고정 포인터로 저장하며 Viewport보다 오래 살아 있어야 한다. 출력 자원 / 동작 / FRenderer 연결은 없다. 기존 통합 GPU 코드와 실행은 유지하며 다음은 출력 자원 / 장치 연결의 역할 설명이다. 근거는 RendererDecisions 11.42 / EditorDecisions 1.21을 따른다.

- **초기 그래픽스 소유 배치 (값 멤버 연결 완료 / GPU 호출 이동 전)** — FRenderer가 FD3D11Device와 FD3D11Viewport를 나란히 소유한다. Device를 먼저 선언하고 Viewport는 장치를 비소유로 연결한다. 생성 / 종료 연결은 단계별로 이동하며 장치보다 출력 자원을 먼저 종료한다. 장치 준비 분리용 void InitD3DDevice()의 본문을 구현하고 기존 Init에서 호출하도록 연결했다. 기존 장치 재사용·정상 반환 시 준비 완료·최종 생성 실패 부분 정리 후 Fatal 계약이다. 전체 Init의 반환형은 bool로 유지하며 이후 자원 실패는 정리 / false다. 여러 출력 관리 객체의 장치 공유나 공통 장치와 창별 출력의 독립 수명이 필요해지면 공통 기반 장치 소유 / 창별 출력 Viewport 소유로 전환한다. 현재 FRenderer에 Device 다음 Viewport{Device} 멤버를 추가했다. 기본 생성 / 파괴 연결만 적용했고 실제 GPU Init / Shutdown / 프레임 호출은 기존 Device 경로다. 판단 근거는 EditorDecisions 1.21.1을 따른다.

- **경계 두 개** — 렌더러를 통째로 바꾸는 경계와, SeonEngine 렌더러 아래에서 그래픽스 API를 바꾸는 경계를 나눈다.

  - 현재 기본 구현은 `Private/Renderer/Renderer.h`의 `FRenderer`이며 `IRenderer`의 호출을 장치 준비 / Clear의 `FD3D11Device`와 창 출력의 `FD3D11Viewport`에 연결한다. 계약은 `Public/Renderer/RendererInterface.h`에 둔다. 기본 구현은 D3D11 고정 구성이며 RHI 분리는 아직 미구현이다. 실제 그래픽스 API 호출과 COM 자원 관리는 D3D11 전용 폴더에 둔다.
  - D3D11 장치 / 컨텍스트와 단일 창 스왑체인은 같은 클래스 안에서 별도 호출로 생성한다. `D3D11CreateDevice`로 만든 장치에 대응하는 DXGI adapter / factory를 확보하고 `CreateSwapChain`을 호출한다. 생성 호출 분리는 RHI / viewport 클래스 분리를 뜻하지 않는다.

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
- **창과 출력 대상** — 아래 연결 계약을 따른다. 계약 헤더와 단일 창 D3D11 구현, 메인 루프 호출을 연결했다.
  - 첫 범위는 창 하나, 출력 대상 하나, 메인 스레드다. 엔진은 창과 렌더러 구현을 소유한다. 플랫폼 창은 OS 창을 만들고 파괴하며, 렌더러는 출력용 그래픽스 자원을 만들고 해제한다.
  - 초기화 함수에 `void* WindowHandle`, `uint32 SizeX`, `uint32 SizeY`를 개별 인수로 전달한다. 크기는 백버퍼가 사용할 클라이언트 영역의 픽셀 수다. 창 바깥 크기와 DPI 배율 적용 전 UI 논리 크기를 넘기지 않는다.
  - 핸들은 소유권을 넘기지 않는 참조다. 렌더러는 값을 보관할 수 있지만 창을 파괴하거나 창 메시지를 구독하지 않는다. 핸들의 플랫폼 타입 변환은 백엔드 안에서만 한다.
  - `IRenderer::Init(void* WindowHandle, uint32 SizeX, uint32 SizeY)`는 `bool`을 반환한다. 호출은 한 번이며, 유효한 핸들과 양수 크기가 필요하다. 시작부터 최소화 / 크기 0인 창은 엔진이 양수 크기로 복원될 때까지 초기화를 미룬다. 초기화 전 닫기 요청이면 렌더러를 초기화하거나 `Shutdown()`하지 않고 창만 정리한다.
  - 성공한 초기화 뒤에만 `Resize(uint32 SizeX, uint32 SizeY)`와 프레임 출력을 호출한다. `Resize`에는 양수 크기만 전달하며 반환형은 `void`다. 출력 자원 재구성 실패는 복구 불가로 보고 구현이 치명 로그 후 종료한다. 장치 손실 복구는 첫 범위에 포함하지 않는다.
  - 창 프로시저는 최신 크기와 최소화 상태만 기록하고 렌더러를 호출하지 않는다. 엔진은 메시지 펌프가 끝난 뒤 닫기 요청을 먼저 확인하고, 그다음 최신 상태를 읽는다. 유효 크기가 적용된 출력 크기와 다르면 프레임 출력 전에 한 번 `Resize`한다. 생성 중 메시지도 같은 방식으로 기록한다.
  - 최소화 상태이거나 어느 한 변이 0이면 `Resize`, 그리기, `Present`를 건너뛴다. 마지막 양수 출력 크기는 유지한다. 복원하면 최신 양수 크기를 적용한 뒤 출력을 재개한다. 메시지 펌프는 계속 수행하고, 출력 정지 중 CPU 대기 정책은 적용 보류다. 필요해지면 엔진 루프에서 관리한다.
  - 첫 구현에서는 창 이동 / 크기 조절 모달 루프 중 프레임 출력을 멈춰도 된다. 모달 루프를 빠져나와 펌프가 반환하면 최신 상태를 반영한다. 창 프로시저에서 재진입 렌더링하지 않는다.
  - 정상 종료는 렌더 씬 파괴(생겼을 때) → 성공적으로 초기화된 렌더러의 `Shutdown()` → 창의 `Shutdown()` 순서다. 현재 관리 객체는 EngineMain 반환 후 플랫폼 진입점의 스택에서 소멸한다. 렌더러가 창 핸들을 사용하는 동안 창이 살아 있어야 한다. 초기화 실패 후 치명 종료는 위 오류 처리 규칙을 따른다.
  - DPI 인식 설정과 플랫폼 크기 조회가 실제 백버퍼 픽셀 크기를 제공하는지는 구현 때 함께 확인한다. 여러 창, 출력 대상 변경, 오프스크린 텍스처, 전체 화면과 실행 중 재초기화는 별도 확장으로 둔다.
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
  | World | 게임의 원본. 액터, 컴포넌트, 게임 로직 | `FWorld` (액터 생성 / 소유 / 소멸 구현) |
  | `<시스템>Scene` | 각 시스템이 월드를 자기 용도로 본 사본 | `IRenderScene`(렌더러 인터페이스, 4장), 나중에 `FPhysicsScene` |
  | Level | 저장하고 불러오는 콘텐츠 단위 (맵) | 필요해지면 만든다 |

  - 접두어 없는 `FScene`은 쓰지 않는다. 씬 이름에는 항상 시스템 이름을 붙인다.
  - 물리 쪽 씬은 충돌 검사와 시뮬레이션을 함께 담당하고 `Physics`로 이름을 통일한다.
- **오브젝트 모델** — 월드에 놓이는 오브젝트는 `FActor` 타입 하나로 두고, 기능은 컴포넌트 클래스(데이터 + 동작)를 붙여 만든다. 액터를 종류별로 상속해 기능을 넣지 않는다(Unreal 5의 `AActor` 서브클래스 방식과 다르다). ECS(ID 엔티티 + 데이터 컴포넌트 + 시스템)는 쓰지 않는다.
- **소유와 통신** — 월드가 시스템별 씬을 소유한다. 컴포넌트는 자기와 관련된 씬에만 등록한다(메시 컴포넌트는 렌더 씬, 충돌 컴포넌트는 물리 씬). 씬끼리는 직접 통신하지 않는다. 물리 결과는 월드의 컴포넌트에 반영되고, 컴포넌트가 렌더 씬을 갱신한다(1장 "같은 계층끼리 의존 금지").

- **계층 변환 합성 (계산 / 부모 변경 구현 완료)** — 로컬·월드는 FTransform이고 컴포넌트 GetComponentTransform은 로컬 * 부모 월드를 조회 시 재귀 계산한다. FActor::SetParent의 KeepWorld는 기존 월드.GetRelativeTransform(새 부모 월드)를 새 로컬로 적용하고 루트 분리는 기존 월드 자체를 적용한다. 일반 아핀 행렬의 전단 전체를 보존하지 않으며 재합성 차이만으로 false를 반환하지 않는다. 행렬이 필요하면 계산된 월드 FTransform을 ToMatrixWithScale로 바꾼다. 캐시 / 렌더 연결은 아직 없다. 근거와 한계는 WorldDecisions 32·33·38절, MathDecisions 33~40절을 따른다.

- **전체 트랜스폼 조회** — FTransformComponent::GetTransform() const는 내부 FTransform을 값으로 반환한다. 반환값은 컴포넌트와 독립적이며 기존 행렬 변환 함수에 사용할 수 있다.
- **필수 트랜스폼 컴포넌트** — 모든 `FActor`는 생성 완료 시 기본 `FTransformComponent` 하나를 가진다. 추가 시 트랜스폼 예외 없이 일반 실제 타입 중복 규칙을 적용한다. 기본 타입은 false 정책과 기존 인스턴스 때문에 추가가 거부되지만 서로 다른 파생 타입은 함께 추가할 수 있다. 계열 전체를 하나로 제한하지 않는다. 제거 API / 정책은 후속이다. 액터의 위치 / 회전 / 크기는 이 컴포넌트의 `FTransform`을 기준으로 하며 액터에 별도로 중복 보관하지 않는다. 로직 전용 액터에도 적용한다. 트랜스폼 보유만으로 렌더링이나 물리 처리를 수행하지 않는다. 현재는 FActor 생성자에서 자신의 참조를 전달해 트랜스폼 하나를 생성하고 소유 목록에 넣는다. 목록은 private이며 AddComponent는 일반 중복 규칙을 적용한다. 컴포넌트 제거 API는 없다. 데이터 기본값은 위치 0 / 회전 없음 / 크기 1배다. 위치·회전·크기 배율 조회와 변경, 부모·자식 연결과 월드 변환 조회가 가능하다.

- **액터 계층과 공간 변환 (기본 연결 구현 완료)** — 부모·자식 관계와 자식 소유는 FActor에 두고 FTransformComponent는 로컬 값과 월드 계산을 담당한다. 월드는 루트만 소유하며 부모는 자식 목록을 단독 소유한다. 필수 트랜스폼 하나를 유지하고 부모 관계를 컴포넌트에 중복 저장하지 않는다. 실제 부모 변경과 다단계 월드 조회를 구현했다. 개별 제거·제거 예정 상태·씬 갱신은 후속이다. WorldDecisions 27·38절을 따른다.

- **부모 변경과 분리의 변환 유지 (구현 완료)** — ETransformRule은 Engine/Public/Engine/EngineTypes.h에 선언하고 Actor.h에서 포함한다. bool FActor::SetParent(FActor* newParent, ETransformRule rule)은 KeepWorld / KeepLocal을 명시적으로 요구하고 nullptr는 월드 루트 분리다. 조건과 새 로컬을 계산하고 대상 저장 공간을 준비한 뒤 기존 소유 포인터를 옮겨 부모와 로컬을 변경한다. 객체 / 컴포넌트 주소와 월드 소속은 유지한다. true는 변경 완료 또는 정상 같은 부모 요청이며 false는 기존 상태 유지다. 할당 실패의 치명 종료는 false 복구 계약에 포함하지 않는다. WorldDecisions 30·33·38절을 따른다.

- **SetParent의 현재 검사와 후속 조건** — 살아 있는 포인터와 유한 값·단위 회전 전제를 사용한다. 지원하지 않는 규칙 / 자기 자신 / 자손 / 다른 월드 / 월드 소유 트리에 없는 액터는 false다. 정상 등록 검사 뒤 같은 부모 / 이미 루트의 nullptr 요청은 변경 없이 true다. 안전 역수 0 / 역행렬 항등 대체를 허용하고 보존 오차 검사는 없다. 제거 처리 상태와 Destroy / DestroyActor는 구현했다. 보관 목록의 별도 정리 시점은 아직 없다. 비유한 / 비정규 입력을 SetParent 전체에서 검증하거나 죽은 포인터를 판별하지 않는다. WorldDecisions 30.4·38절을 따른다.

- **액터와 컴포넌트의 수명** — 현재 월드는 루트 액터를 소유하고 액터는 자식 액터와 자신의 컴포넌트를 소유한다. 부모 제거 시 모든 자손도 함께 제거하며, 분리는 파괴 없이 월드 루트로 소유권을 옮기는 방향이다. 월드 정리 시 전체 하위 계층과 컴포넌트를 함께 정리한다. 컴포넌트의 소속 액터 참조는 비소유다. 액터 제거 시 관련 컴포넌트를 시스템 씬에서 등록 해제한 뒤 컴포넌트와 액터를 파괴한다. 시스템 씬은 이 정리가 끝날 때까지 유지하고, 이후 씬 → 렌더러 → 창 순서로 종료한다. 공간 관계와 소유는 별도 개념이지만 현재 채택한 모델에서는 동일한 액터 계층에 적용한다. 월드는 제거 실행의 안전한 시점, 액터는 소유한 하위 계층 정리를 맡는 방향이며 구체 절차는 미정이다. 생성 / 소멸과 루트 / 자식 소유권 이전은 구현했다. 개별 제거 / 제거 예정 상태 / 안전 시점 실행과 씬 등록 해제는 아직 없다. 근거는 WorldDecisions 28절을 따른다. 개별 제거 / 시스템 씬 등록·해제 / 외부 참조의 유효성 보장 / 갱신 중 제거 정책은 후속 설계에서 정한다.

- **컴포넌트 관리의 첫 범위** — 같은 실제 타입의 범용 컴포넌트 중복 검사를 구현했으며 트랜스폼 전용 캐시 참조 / 조회 API는 필요할 때 추가한다. 생성 시 기본 트랜스폼 하나를 준비하며 트랜스폼 추가에도 일반 중복 규칙을 적용한다. 계열 전체 추가 금지 예외는 두지 않는다. 제거 정책은 후속이다. 일반 컴포넌트는 같은 실제 타입의 중복을 기본 금지하고, 필요한 타입만 AllowsMultipleComponents() const를 재정의하여 true를 반환한다. 상속 그룹 제한은 필요 시 도입한다. FActorComponent의 public virtual 정책 조회와 false 반환 기본 구현을 제공하며 bAllowMultiple 정적 상수는 제거했다. 재정의 없는 자손은 상위 구현을 사용한다. AddComponent의 소속·제거 상태·실제 타입 중복 검사와 소유권 이전을 구현했다. 시스템 등록 / 이벤트는 아직 없다. 실제 클래스 메타데이터 조회가 가능해지면 정책 조회 구현을 리플렉션으로 이전한다. 이전 조건·범위는 DeferredTasks의 리플렉션 항목과 작업 공간 WorldDecisions 60절, 현재 구현은 61절, 트랜스폼에 일반 중복 규칙을 적용하는 결정은 63절을 따른다.

- **액터 컴포넌트 공통 기반** — 액터에 붙는 기능 객체는 `FActorComponent`를 공통 기반으로 사용한다. 상위 `FComponent`는 현재 만들지 않으며, 다른 용도의 컴포넌트 체계와 실제 공통 데이터 / 동작 / 계약을 공유할 필요가 생기면 추출한다. 생성 시 `FActor&`로 소속 액터를 받고 비소유 포인터로 보관한다. 소속 변경은 지원하지 않으며 액터는 컴포넌트의 전체 수명 동안 살아 있어야 한다. `GetOwner() const`는 소속 액터의 참조를 반환하며 가상 소멸자는 기반 타입을 통한 파괴를 지원한다. 기반과 파생 FTransformComponent를 구현했고 액터 생성 / 소멸에서 필수 트랜스폼의 소유 / 수명을 연결했다. 시스템 씬 등록 / 해제는 아직 없다.

- **스태틱 메시 컴포넌트의 현재 범위** — FStaticMeshComponent는 FActorComponent 파생이며 FActor& 생성자에서 기반 소속만 연결한다. private TArray<FVector3> vertices에 컴포넌트 로컬 정점 위치를 직접 소유하며 생성 시 빈 배열이다. 인덱스·설정 API·렌더 씬 등록 / 출력은 아직 없다. 중복 정책은 기반 false를 상속한다. GPU 자원은 렌더러에서 담당한다. 공유 메시 타입 도입 시 CPU 데이터를 이전하고 컴포넌트에는 참조·객체별 설정을 남긴다. 생성·추가 흐름은 기존 계약을 따른다. 작업 공간 WorldDecisions 71·74절과 DeferredTasks의 메시 데이터 분리 항목을 참조한다.

- **액터의 컴포넌트 저장** — `FActor`는 `TArray<TUniquePtr<FActorComponent>>` 목록에서 각 컴포넌트를 단독 소유한다. 생성자에서 필수 트랜스폼 하나를 만들어 목록에 넣으며 기본 소멸 때 함께 정리한다. 월드의 SpawnActor도 이 생성자를 사용한다. 전용 트랜스폼 캐시는 없다. AddComponent 검사·소유권 이전을 구현했으며 컴포넌트 제거 API / 시스템 씬 연결은 아직 없다.

- **컴포넌트 조회** — `T* GetComponent<T>() const`와 `void GetComponents(TArray<T*>& outComponents) const`를 제공한다. T는 FActorComponent 자신 또는 public 파생 타입이며 RTTI로 목록 순서상 첫 변환 가능한 객체를 반환하고 없으면 nullptr다. 소유권을 넘기지 않으며 포인터는 컴포넌트 파괴 시 무효다. 조회는 목록을 바꾸지 않지만 const 액터에서도 가변 컴포넌트 포인터를 반환한다. 트랜스폼 컴포넌트 조회와 위치·회전·크기 배율 조회·변경은 가능하다. 생성 시 먼저 넣은 기본 트랜스폼이 현재 액터 계층 변환의 조회 기준이다. 이후 파생 트랜스폼을 추가해도 이 기준을 자동 대체하지 않는다. 기준 선택 / 교체 API는 없다. GetComponents는 출력 배열을 clear하고 T와 파생 타입의 모든 일치를 저장 순서로 채운다. 확보 용량을 재사용하며 빈 결과도 정상이다. 자기 액터만 검색하고 결과 배열의 수정은 소유 목록에 영향이 없다. 파괴 후 포인터는 무효이며 후속 추가는 재조회해야 반영된다. Unreal 출력 흐름을 참고하되 std::vector와 RTTI를 사용하는 자체 구현이다. 결정과 검증은 작업 공간 WorldDecisions 69절을 따른다.

- **트랜스폼 위치 접근** — FTransformComponent의 GetPosition() const는 저장된 위치를 FVector3 값으로 반환하고 SetPosition(FVector3)는 위치 값만 바꾼다. 단위는 cm다. 부모 관계 / 좌표 변환 / 시스템 씬 갱신은 아직 연결하지 않았다. 내부 FTransform 전체를 가변 참조로 공개하지 않는다.
- **트랜스폼 회전 조회** — GetRotation() const는 저장된 FQuat의 ToRotator() 결과를 도 단위 FRotator 값으로 반환하고 GetQuaternion() const는 저장 쿼터니언의 복사본을 반환한다. 내부 회전은 하나만 저장하며 조회 결과의 수정은 내부 값에 영향을 주지 않는다. SetRotation(FRotator)은 ToQuat() 결과로 기존 저장 회전을 대체한다. SetRotation(FQuat)은 유한한 단위 쿼터니언 입력 전제로 그대로 대체한다. 두 setter에 별도 검사·정규화나 시스템 씬 갱신은 없다.
- **트랜스폼 크기 접근** — GetScale() const는 저장된 축별 배율을 FVector3 값으로 반환하고 SetScale(FVector3)는 새 배율로 대체한다. 기본값은 (1, 1, 1)이며 단위 없는 배율이다. 행렬 계산과 시스템 씬 갱신은 아직 연결하지 않았다.

- **액터 객체의 복사 / 이동** — `FActor`는 복사 생성 / 복사 대입 / 이동 생성 / 이동 대입을 금지한다. 액터 타입에 네 연산의 삭제 선언을 구현했다. 월드는 단일 소유 포인터로 액터 객체의 주소를 유지하며, 소유 포인터의 이동과 트랜스폼 변경은 허용 가능한 별개 동작이다. 월드의 액터 생성 / 소유, 액터의 필수 트랜스폼 생성, 위치·회전·크기 배율 조회·변경을 구현했다.

- **월드의 액터 저장** — `FWorld`는 `TArray<TUniquePtr<FActor>>` 목록에서 루트 액터를 단독 소유하고 각 부모가 자식을 단독 소유한다. SpawnActor()는 MakeUnique<FActor>(*this)로 소속 월드를 전달해 액터를 생성하고 목록에 넣어 같은 액터의 비소유 참조를 반환한다. FActor는 explicit FActor(FWorld&)로 생성 시 월드를 요구하며 private FWorld* const world에 비소유 저장한다. GetWorld() const는 그 월드의 수정 가능한 참조를 반환한다. 소속은 생성 후 바꾸지 않으며 월드는 액터가 살아 있는 동안 유효해야 한다. 생성자 호출만으로 월드 목록에 자동 등록되지는 않으므로 엔진의 소유 / 등록 경로는 SpawnActor다. 반환 참조는 액터 파괴 시 무효다. 월드 기본 생성자는 빈 목록을 준비하며 월드 소멸 때 소유한 액터와 컴포넌트를 함께 파괴한다. GameInstance 비소유 연결 / 조회는 구현했지만 개별 제거 API, 게임 프레임 실행과의 연결, 시스템 씬 소유 / 등록 해제는 아직 없다.

- **게임 객체 구성 정책** — 기본 FActor는 공통 월드·계층·컴포넌트·수명 관리만 담당한다. 게임별 조율은 Actor 파생 타입 또는 행동 컴포넌트에서 선택하고 혼합 구성도 허용한다. FActor 소멸자는 virtual로 기반 소유 포인터의 파생 파괴를 준비했다. 타입 지정 파생 생성·컴포넌트 추가 / 갱신·이벤트 연결은 후속이며 두 사용 방식 전체가 구현된 것은 아니다. 근거는 작업 공간 WorldDecisions 50~53절을 따른다.

- **컴포넌트 생성과 추가** — 공개 사용 흐름은 생성 → 설정 → 액터에 추가로 통일한다. 생성 시 소속 액터를 지정하고 호출자가 임시 소유하며, 추가 성공 시 액터 소유로 이전하는 방향이다. 생성만으로 목록 추가·시스템 등록되지는 않는다. AddComponent(TUniquePtr<T>&)의 bool 템플릿 본문을 헤더에 구현했다. 성공하면 인수를 비우고 액터가 소유하며 실패하면 호출자 소유를 유지한다. 빈 포인터 / 다른 소속 / 제거 중 액터 / 복수 불허인 같은 실제 타입 중복이면 false다. 실제 객체의 가상 정책과 RTTI 타입을 사용하여 기반 소유 포인터도 허용한다. 모든 검사 후 공간 준비·소유권 이전 순서이며 등록 생명주기는 미구현이다. 근거와 검증은 작업 공간 WorldDecisions 64절을 따른다.

- **액터 계층 저장의 현재 구현** — FActor의 parent는 비소유, children은 자식 단독 소유 목록이다. GetParent는 루트에서 nullptr다. GetChildCount / GetChild(int32)는 직접 자식 수와 비소유 주소를 조회한다. 음수 / 범위 밖이면 nullptr이며 자식 파괴 시 포인터는 무효다. 계층 변경 후 인덱스는 달라질 수 있다. World의 액터 목록은 루트만 소유하고 SetParent가 월드 ↔ 부모 / 부모 ↔ 부모 사이의 TUniquePtr를 이동한다. FWorld는 이를 위해 FActor에 friend 접근을 허용하며 외부에 소유 목록 변경 API는 열지 않는다. 자식 목록은 컴포넌트 뒤에 선언해 기본 소멸에서 먼저 정리된다. WorldDecisions 35·38절을 따른다.

- **액터 제거 처리와 보관** — Destroy는 World.DestroyActor에 위임한다. 자기 월드의 살아 있는 등록 액터가 전제이며 소속·소유 불변식 위반은 assert / abort로 중단한다. 이미 제거 중이면 true, 보관 공간 준비 후 자신·자손 상태를 켜고 활동 소유 목록에서 destroyedActors로 이전한다. 대상 parent는 null, 보관 자손 연결은 유지한다. SetParent는 자신 / 새 부모 제거 중이면 같은 부모 요청도 false다. 제거 뒤 월드 변환 보존은 보장하지 않는다. PurgeDestroyedActors는 보관 원소를 실제 파괴한다. 해당 객체 사용 / 순회가 끝난 안전한 시점에 명시적으로 호출하며 프레임 연결 / 씬·Tick·통지는 후속이다. 미호출 시 월드 소멸 때 회수한다. GC 등 회수 책임 대체 후 임시 목록·정리 함수를 제거하는 HACK 조건을 기록했다. 비트 필드는 보류한다. 작업 공간 WorldDecisions 40~47절 참조.

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
- **수학 타입** — 엔진 타입(`FVector2`, `FVector3`, `FVector4`, `FRotator`, `FQuat`, `FMatrix4`, `FTransform`, 함수 모음 `FMath`)을 `Core/Public/Math/`에 둔다. 엔진 코드와 공개 API는 이 타입만 쓴다.
  - 계산은 DirectXMath로 구현한다. DirectXMath는 Windows SDK판이 아니라 vcpkg로 받은 것을 쓴다(Project Settings "vcpkg"). 함수를 하나씩 직접 구현으로 바꿀 수 있다. 사용처는 엔진 타입만 쓰므로 바꿀 때 고치지 않는다.
  - 타입은 `float` 멤버만 가진다. SIMD 타입(`XMVECTOR`, `XMMATRIX`)은 정렬 제약이 있어 멤버로 두지 않고, 계산할 때만 쓴다.
  - DirectXMath는 수학 타입의 구현 파일에서만 include한다. 공개 헤더에 `DirectX` 타입이 드러나지 않게 한다. 인라인이 필요할 만큼 호출 비용이 문제가 되면 그때 다시 검토한다.
- **행렬 데이터** — FMatrix4는 Math/Matrix4.h의 public float m[4][4]로 16개 성분을 행 우선 저장한다. 접근은 m[행][열]이며 인덱스는 0~3이다. 멤버 초기값은 {}이며 기본 생성 시 모든 성분이 0인 영행렬이다. SetIdentity()는 현재 객체를 항등행렬로 바꾼다. Identity 상수와 나머지 계산 / 변환 함수는 아직 없다.
- **트랜스폼 저장** — 컴포넌트와 에셋은 위치, 회전, 크기를 `FTransform`으로 저장한다. FTransform::ToMatrixWithScale() const로 위치·회전·크기를 포함한 FMatrix4를 값으로 만든다. 입력은 유한한 위치·크기와 단위 회전이며 행 벡터 적용 순서는 크기 → 회전 → 이동이다. DirectXMath는 cpp 내부에서만 사용하며 입력 보정이나 부모 좌표 계산은 없다. ToMatrixNoScale() const는 크기 제외 행렬 API로 선언되어 있으나 현재 작업 폴더에는 정의가 없으며 별도 진행 중 변경 상태다. 렌더러 연결은 아직 없다.
- **각도 회전 표현** — FRotator는 Math/Rotator.h에서 도 단위 float pitch / yaw / roll을 보관하며 기본값은 모두 0이다. LUF의 X / Y / Z 양의 축에 오른손 법칙을 적용한다. 회전은 고정 기준 축 X(Left) → Y(Up) → Z(Forward)의 외재적 순서로 해석하며 각 단계에서 기준 축은 움직이지 않는다. 고정 기준 축은 회전을 표현하는 공간의 처음 축이며 항상 월드 축이라는 뜻은 아니다. 내부 회전 저장은 FQuat을 유지한다. FRotator::ToQuat() const는 유한한 도 단위 각도를 이 순서의 정규화된 FQuat으로 변환한다. DirectXMath는 Core/Private/Math의 변환 구현 파일에서만 사용하며 공개 헤더에는 노출하지 않는다. FQuat::ToRotator() const는 유한한 단위 쿼터니언을 같은 순서의 도 단위 대표 각도로 반환한다. pitch / roll은 [-180, 180], yaw는 [-90, 90]이다. Quat.cpp는 float 기본 추출 수식과 cmath를 사용하고 도 변환은 DirectXMath다. 길이 보정과 특이점 분기는 없으며 yaw ±90도와 그 근처의 안정적인 재구성은 보장하지 않는다. 컴포넌트 회전 조회와 FRotator / FQuat 설정은 가능하다. 입력 오류 검사는 아직 없다.
- **수학 타입의 현재 구현 범위** — `FVector3`는 `Math/Vector3.h`에서 float x / y / z와 기본값 0을, `FQuat`은 `Math/Quat.h`에서 float x / y / z / w와 기본값 (0, 0, 0, 1)을 구현했다. FQuat 기본값은 회전 없음이다. 성분은 축별 각도가 아니며 임의 수정 시 단위 길이를 자동 보장하지 않는다. `FTransform`은 `Math/Transform.h`에서 position / rotation / scale을 값으로 묶으며 기본값은 위치 0 / 회전 없음 / 크기 1배다. 좌표 공간이나 부모 관계를 저장하지 않으며 사용처에서 기준을 정한다. 액터가 생성하는 FTransformComponent에 데이터를 보관하며 컴포넌트의 위치와 크기 배율 조회·변경은 가능하다. FRotator::ToQuat과 FQuat::ToRotator 변환 계산은 구현했다. 컴포넌트 회전 조회와 FRotator / FQuat 설정도 가능하다. FTransform::operator*(const FTransform& other) const는 왼쪽 후 오른쪽의 성분 합성을 값으로 반환하고 두 원본을 유지한다. 유한한 값·단위 회전을 전제로 한다. 어느 입력이든 크기 < 0이면 행렬 합성 후 축 길이를 제거하고 결과 크기 부호를 반영해 회전을 복원 / 정규화한다. 결과 크기는 두 입력의 축별 곱이고 위치는 행렬 원점이다. 축이 사라지면 회전은 항등값이며 복원 기준은 길이 제곱 1e-8 / 성분 절댓값 1e-4다. 입력 검사·입력 회전 정규화는 없다. 크기 0 기본 합성은 가능하며 일반 행렬 곱의 전단을 보존하지 않는다. 부모 관계나 좌표 공간을 알지 않는다. 나머지 계산과 수학 타입은 아직 없다.
- **상대 변환의 현재 구현 범위** — FTransform::GetRelativeTransform(const FTransform& other) const는 기준 값에 대한 상대 변환을 값으로 반환한다. 유한 값·양쪽 단위 회전을 전제로 기본 경로와 음수 크기의 행렬 경로를 구현했다. 기준 크기의 절댓값이 1e-8f 이하인 축은 안전 역수 0으로 대체하며 원본 두 값은 유지한다. 반환값이 정확한 월드 보존 성공을 의미하지 않는다. 어느 입력이든 음수 크기가 있으면 현재 행렬 × 기준 역행렬을 사용하고 결과 크기는 안전 역수로 따로 계산한다. 기준 행렬의 세 축 모두 절댓값 1e-8 이하이면 항등 역행렬을 대체하며, 그 외에는 행렬식이 0 또는 비유한 값이면 항등으로 대체한다. FQuat::IsNormalized와 FTransform::IsRotationNormalized는 유한 성분 전제로 abs(길이 제곱 - 1) <= 0.01을 검사한다. 기준 회전 검사 실패 시 음수 분기 전에 전체 항등값을 반환한다. 자동 정규화·포괄적 입력 검증·계층·SetParent 연결은 아직 없다. 설명과 확인 범위는 MathDecisions 36·38·40절을 따른다.
- **위치와 방향 변환** — 점은 `TransformPosition`(이동 적용), 방향은 `TransformVector`(이동 무시)로 나눠 변환한다. 행렬 곱셈 연산자는 수학 타입과 렌더러 안에서 쓸 수 있다.

### D3D11 실행 연결 현재 기준 (2026-10-09)

FRenderer.Init은 Device.InitD3DDevice → Viewport.Init, Resize는 Viewport.Resize, RenderFrame은 Viewport RTV로 Device.ClearRenderTargetView 후 Viewport.Present를 호출한다. Present 실패는 FRenderer Fatal이다. Shutdown은 컨텍스트 바인딩 해제 → Viewport → Device 순서이며 출력 생성 실패는 부분 출력 / 장치 정리 후 false다. 기존 Device.Init / Resize / RenderFrame와 출력 멤버는 삭제 완료했다. 위 상세 분리 절의 실제 연결 전 설명은 이전 단계 이력으로 읽고 현재 호출은 이 절을 기준으로 한다. Debug·Release 실제 배경 / 크기 변경 / 최소화·복원 / 종료 회귀 6개를 확인했다. GPU 완료 / 누수·실패 주입 검증은 없다.
