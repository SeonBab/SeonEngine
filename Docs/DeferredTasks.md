# Deferred Tasks

지금은 하지 않고, 필요해지면 진행할 작업을 모아둔다. 작업을 진행하면 결과를 해당 문서에 반영하고 여기서 삭제한다.

## 목록

| 분류 | 작업 | 진행 시점 |
|---|---|---|
| 빌드 / 도구 | PCH (미리 컴파일된 헤더) | 빌드 시간이 문제가 될 때 |
| 빌드 / 도구 | Development 빌드 구성 | Debug 빌드가 너무 느려서 평소 개발이 불편해질 때 |
| 빌드 / 도구 | 모듈별 프로젝트 분리 | `Engine` 모듈을 `Renderer`, `RHI`, `Platform` 등으로 나눌 때, 또는 역방향 include가 반복해서 생길 때 |
| 빌드 / 도구 | clang-tidy | 검사할 코드가 어느 정도 쌓였을 때 |
| 빌드 / 도구 | CI | 빌드할 코드가 저장소에 들어갔을 때 |
| 엔진 기반 시스템 | 서브시스템 등록 / 조회 | `NEngine`과 첫 서브시스템을 구현할 때 |
| 엔진 기반 시스템 | 핸들 시스템 | 리소스(텍스처, 메시 등)나 게임 오브젝트 관리 시스템을 만들 때 |
| 엔진 기반 시스템 | 메모리 할당자 | 메모리 사용량 추적, 누수 검사, 프레임 단위 임시 할당 등이 필요해질 때 |
| 엔진 기반 시스템 | 델리게이트 / 이벤트 | 객체 간 이벤트 통지(입력, UI, 게임 이벤트 등)가 필요할 때 |
| 엔진 기반 시스템 | Threading | 렌더 스레드나 워커 스레드(에셋 로딩, 작업 시스템 등)를 도입할 때 |
| 엔진 기반 시스템 | 프로젝트 경로와 `Saved` 폴더 | 에셋 로딩이나 로그 파일처럼 파일 경로가 필요한 기능을 구현할 때 |
| 렌더링 | RHI (그래픽스 API 추상화 계층) | D3D12나 Vulkan 등 두 번째 그래픽스 API를 추가할 때 |
| 에디터 | 리플렉션 | 에디터 개발을 시작할 때 (인스펙터, 씬 직렬화, Undo/Redo에 필요) |
| 에디터 | GC 관리 객체 | 에셋이나 게임 오브젝트처럼 수명을 엔진이 관리해야 하는 객체 체계가 필요할 때 |
| 에디터 | 에디터용 에러 메시지 전달 | 에디터나 도구에서 사용자에게 실패 원인을 보여줘야 할 때 |

---

## 빌드 / 도구

### PCH (미리 컴파일된 헤더)

- **진행 시점**: 빌드 시간이 문제가 될 때
- **정할 것**
  - 도입 여부, 파일 이름과 위치 (모듈별 / 엔진 공통)
  - 포함 대상: 무겁고 거의 바뀌지 않는 헤더(`Windows.h`, `d3d11.h`, STL 등). 자주 바뀌는 엔진 헤더는 제외
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 2.2 Header 규칙
  - MSVC에서는 PCH를 `.cpp`의 첫 include로 두어야 하므로, include 순서에서 자기 헤더보다 앞에 둔다.

### Development 빌드 구성

- **진행 시점**: Debug 빌드가 너무 느려서 평소 개발이 불편해질 때
- **정할 것**
  - Release를 기반으로 최적화를 켜고, assert(`SE_ASSERT` 등)와 디버그 정보를 켠 구성 추가 (Unreal의 Development에 해당)
  - `SE_BUILD_DEVELOPMENT` 매크로 정의
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 5.2 Assert, 6.1 빌드 / 컴파일러 설정, [Project Settings](ProjectSettings.md)

### 모듈별 프로젝트 분리

- **진행 시점**: `Engine` 모듈을 `Renderer`, `RHI`, `Platform` 등으로 나눌 때, 또는 역방향 include가 반복해서 생길 때
- **범위**: 엔진과 게임은 이미 별도 프로젝트로 나뉘어 있다. 여기서는 `Engine` 프로젝트 안의 엔진 모듈(`Core`, `Engine` 등) 분리를 다룬다.
- **정할 것**
  - 엔진 모듈마다 정적 라이브러리 프로젝트(`.vcxproj`)를 만들지 여부
  - 각 프로젝트의 include 경로에 의존 모듈의 `Public` 폴더만 넣어 빌드로 의존성 규칙을 강제하는 방식
- **반영할 곳**: [Architecture](Architecture.md) 1. 모듈과 의존성 방향, [Project Settings](ProjectSettings.md)

### clang-tidy

- **진행 시점**: 검사할 코드가 어느 정도 쌓였을 때
- **정할 것**
  - 켤 검사 목록 (`readability-identifier-naming` 등)
  - 우리 네이밍 규칙 중 표현할 수 있는 범위 (`T` 템플릿 클래스, `b` bool 접두사 등은 표현이 어려울 수 있음)
  - Visual Studio 코드 분석과 연결하는 방법
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 6.2 자동화 도구

### CI

- **진행 시점**: 빌드할 코드가 저장소에 들어갔을 때
- **정할 것**
  - GitHub Actions Windows 환경에서 Debug / Release 빌드
  - clang-format 검사 (서식이 맞지 않으면 실패)
  - 도입 후 clang-tidy 검사 추가 여부
  - PR 도입: 작업 브랜치 → PR → Rebase and merge로 바꾸고, CI 통과를 병합 조건으로 건다. PR 템플릿(`.github/pull_request_template.md`)은 준비되어 있다.
  - GitHub 저장소 설정: Rebase and merge만 허용, `main` 보호 규칙(PR 필수, 한 줄 기록)
  - 브랜치 이름 규칙: PR 목록과 원격 브랜치에 이름이 드러나므로 정한다. 후보안은 루트 설계 기록의 Git 컨벤션 결정 기록 5장에 있다.
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 6.2 자동화 도구, [Git Convention](Conventions/GitConvention.md) 1장 브랜치, 3장 병합

---

## 엔진 기반 시스템

### 서브시스템 등록 / 조회

- **진행 시점**: `NEngine`과 첫 서브시스템을 구현할 때
- **정할 것**
  - `GetSubsystem<T>()`가 타입을 구분하는 방법. RTTI(`typeid`)를 쓰지 않기로 했으므로 템플릿 정적 타입 ID나 리플렉션을 쓴다.
    - 참고: Unreal의 `GetEngineSubsystem<T>()`는 리플렉션의 `T::StaticClass()`(UClass 포인터)를 키로 쓴다.
    - 리플렉션 전에는 템플릿 정적 타입 ID를 쓸 수 있다. 타입마다 함수가 따로 만들어지므로 처음 호출될 때 받은 번호가 타입별로 고유하다.

      ```cpp
      template<typename T>
      uint32 GetTypeID()
      {
      	static const uint32 id = GenerateNextTypeID();
      	return id;
      }
      ```

    - 주의: 모듈을 DLL로 나누면 DLL마다 `static` 변수가 따로 생겨 같은 타입이 다른 번호를 받을 수 있다. DLL로 분리할 때는 타입 이름을 컴파일 시간에 해시하는 방식 등 DLL과 상관없이 같은 값이 나오는 방법을 검토한다.
    - 리플렉션이 생기면 리플렉션의 타입 정보로 바꾼다.
  - 등록 시점(`NEngine::Initialize()`)과 초기화 / 종료 순서(Architecture 2. 서브시스템과 전역 상태)의 관계
  - 없는 서브시스템을 조회했을 때의 처리 (nullptr / assert)
- **반영할 곳**: [Architecture](Architecture.md) 2. 서브시스템과 전역 상태

### 핸들 시스템

- **진행 시점**: 리소스(텍스처, 메시 등)나 게임 오브젝트 관리 시스템을 만들 때
- **정할 것**
  - 핸들 구조 (인덱스 + 세대 번호, 비트 배분)
  - 타입별 핸들 (`NTextureHandle` 등) 정의 방식
  - 핸들 → 객체 변환(`Resolve`)과 무효 핸들 처리
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 4.1 메모리 소유권

### 메모리 할당자

- **진행 시점**: 메모리 사용량 추적, 누수 검사, 프레임 단위 임시 할당 등이 필요해질 때
- **정할 것**
  - 엔진 할당자 인터페이스와 종류 (일반 / 프레임 / 풀 등)
  - STL 별칭(`TArray` 등)에 할당자를 넣는 방식
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 3.12 STL 사용 정책, 4.1 메모리 소유권

### 델리게이트 / 이벤트

- **진행 시점**: 객체 간 이벤트 통지(입력, UI, 게임 이벤트 등)가 필요할 때
- **정할 것**
  - 단일 / 멀티캐스트 델리게이트 구조
  - 구독(`Add` / `Remove`)은 외부에 열고, 발생(`Broadcast`)은 소유 클래스만 할 수 있는 구조 (Unreal `DECLARE_EVENT`, C# `event` 참고)
  - 구독한 객체가 먼저 파괴될 때의 처리
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 2.3 Class 구성의 멤버 변수 접근 예외

### Threading

- **진행 시점**: 렌더 스레드나 워커 스레드(에셋 로딩, 작업 시스템 등)를 도입할 때
- **정할 것**
  - 스레드 역할 구분 (Main / Render / Worker)과 각 스레드에서 호출할 수 있는 API
  - 스레드 안전한 API 표기 방법
  - Mutex / Lock 사용 규칙과 Lock 순서
  - Atomic 사용 기준
  - 저장해 두었다가 다른 스레드에서 실행하는 람다의 캡처 수명 (3.11 Lambda 참고)
- **반영할 곳**: [Architecture](Architecture.md)에 Threading 장 추가

### 프로젝트 경로와 `Saved` 폴더

- **진행 시점**: 에셋 로딩이나 로그 파일처럼 파일 경로가 필요한 기능을 구현할 때
- **정할 것**
  - 게임 폴더(`Content/`, `Config/`)를 찾는 기준. 실행 파일은 루트 `Binaries/`에 있어 게임 폴더와 떨어져 있다.
    - 개발 중에는 VS 디버거 작업 디렉터리(`LocalDebuggerWorkingDirectory`)를 게임 폴더로 지정하는 방식을 우선 검토한다. `.vcxproj.user`가 아니라 `.vcxproj`에 넣어야 git으로 공유된다.
  - 로그, 크래시 덤프 등 실행 중 생성되는 파일의 위치 (루트 `Saved/` / 게임 폴더의 `Saved/`). Unreal은 프로젝트 폴더의 `Saved/`를 쓴다.
  - 배포(패키징)할 때 실행 파일과 `Content/`를 모으는 폴더 구성
- **반영할 곳**: [Project Settings](ProjectSettings.md) 폴더 구조, `.gitignore`

---

## 렌더링

### RHI (그래픽스 API 추상화 계층)

- **진행 시점**: D3D12나 Vulkan 등 두 번째 그래픽스 API를 추가할 때
- **정할 것**
  - 구조: Renderer → RHI(공통 인터페이스) → 백엔드(D3D11RHI, D3D12RHI 등). Renderer 이상은 RHI 타입만 쓰고, API 타입은 백엔드 안에서만 쓴다.
  - 연결 방식: 가상 함수 인터페이스(Unreal `FDynamicRHI`와 같은 방식)를 우선 검토한다.
  - 설계 기준: RHI는 D3D12 방식(커맨드 리스트, 리소스 상태 전환, 펜스)으로 설계하고, D3D11 백엔드에서는 이 개념들을 간단하게 처리한다. D3D11 모양으로 설계하면 D3D12 / Vulkan을 붙일 때 RHI를 크게 고쳐야 한다.
- **반영할 곳**: [Architecture](Architecture.md) 4. 렌더링 백엔드

---

## 에디터

### 리플렉션

- **진행 시점**: 에디터 개발을 시작할 때 (인스펙터, 씬 직렬화, Undo/Redo에 필요)
- **정할 것**
  - 구현 방식: 매크로 수동 등록 / 코드 생성기(libclang 등) / C++26 정적 리플렉션(MSVC 지원 확인 필요)
  - 초기에는 매크로 등록으로 시작하고, 타입이 많아지면 다른 방식으로 옮기는 것을 우선 검토한다.
- **반영할 곳**: [Architecture](Architecture.md)

### GC 관리 객체

- **진행 시점**: 에셋이나 게임 오브젝트처럼 수명을 엔진이 관리해야 하는 객체 체계가 필요할 때
- **정할 것**
  - GC 방식과 기반 클래스 (예: `OObject`)
  - 생성 / 참조 규칙 (`new` / `delete` 금지, 전용 생성 함수, 추적되는 참조 등)
  - 리플렉션과의 관계
- **반영할 곳**
  - [Code Convention](Conventions/CodeConvention.md) 1.2 Naming: `O` 접두사 예약 해제
  - [Code Convention](Conventions/CodeConvention.md) 4장 Memory & Lifetime

### 에디터용 에러 메시지 전달

- **진행 시점**: 에디터나 도구에서 사용자에게 실패 원인을 보여줘야 할 때
- **정할 것**
  - 사용자에게 보여줄 메시지를 넘기는 방식. Unreal처럼 `NString& outErrorMessage` out 매개변수를 쓰는 방식을 우선 검토한다.
  - 코드 분기용 에러 코드 enum(5.1 에러 처리)과 함께 쓰는 기준
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 5.1 에러 처리
