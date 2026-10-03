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
| 빌드 / 도구 | 서드파티 관리 방식 (방향 결정, 적용 대기) | 저장소 안에서 컴파일하는 외부 코드를 들일 때 (CI 도입이나 다른 PC 빌드가 먼저 오면 그때) |
| 빌드 / 도구 | 빌드 시스템 / 프로젝트 생성기 | 엔진 모듈별 프로젝트 분리, 리플렉션 코드 생성, 두 번째 플랫폼이나 IDE 지원 중 하나가 필요할 때 |
| 엔진 기반 시스템 | 플랫폼별 기본 타입 정의 | Windows가 아닌 두 번째 플랫폼을 지원할 때 |
| 엔진 기반 시스템 | 서브시스템 등록 / 조회 | `FEngine`과 첫 서브시스템을 구현할 때 |
| 엔진 기반 시스템 | 핸들 시스템 | 리소스(텍스처, 메시 등)나 게임 오브젝트 관리 시스템을 만들 때 |
| 엔진 기반 시스템 | 메모리 할당자 | 메모리 사용량 추적, 누수 검사, 프레임 단위 임시 할당 등이 필요해질 때 |
| 엔진 기반 시스템 | null이 될 수 없는 공유 참조 (`TSharedRef`) | 공유 소유 객체를 null 없이 주고받는 API가 반복될 때 (UI 위젯 트리 등) |
| 엔진 기반 시스템 | 델리게이트 / 이벤트 | 객체 간 이벤트 통지(입력, UI, 게임 이벤트 등)가 필요할 때 |
| 엔진 기반 시스템 | Threading | 렌더 스레드나 워커 스레드(에셋 로딩, 작업 시스템 등)를 도입할 때 |
| 엔진 기반 시스템 | 프로젝트 경로와 `Saved` 폴더 | 에셋 로딩이나 로그 파일처럼 파일 경로가 필요한 기능을 구현할 때 |
| 렌더링 | RHI (그래픽스 API 추상화 계층) | D3D12나 Vulkan 등 두 번째 그래픽스 API를 추가할 때 |
| 렌더링 | 여러 렌더 씬 / 여러 뷰 | 에디터, 에셋 미리보기, 분할 화면처럼 씬이나 뷰가 둘 이상 필요할 때 |
| 렌더링 | 렌더 리소스 소유자 | 에셋 시스템을 설계할 때 |
| 렌더링 | Scene Proxy 방식 | 물체 종류별 등록 / 갱신과 캐시 관리가 복잡해질 때. 렌더 스레드 도입 시에도 검토 |
| 에디터 | 실행 파일 구성 (엔진 실행 파일 + 게임 DLL) | 에디터 개발을 시작할 때, 또는 게임 코드 없이 콘텐츠만으로 실행해야 할 때 |
| 에디터 | 리플렉션 | 범용 인스펙터와 씬 직렬화를 구현할 때 |
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
  - 도입 전에 vcpkg 전역 통합을 끈다(아래 "서드파티 관리 방식" 참고). 켜 둔 채로는 개발 PC에서 숨은 의존성을 알아챌 수 없다.
  - PR 도입: 작업 브랜치 → PR → Rebase and merge로 바꾸고, CI 통과를 병합 조건으로 건다. PR 템플릿(`.github/pull_request_template.md`)은 준비되어 있다.
  - GitHub 저장소 설정: Rebase and merge만 허용, `main` 보호 규칙(PR 필수, 한 줄 기록)
  - 브랜치 이름 규칙: PR 목록과 원격 브랜치에 이름이 드러나므로 정한다. 후보안은 루트 설계 기록의 Git 컨벤션 결정 기록 5장에 있다.
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 6.2 자동화 도구, [Git Convention](Conventions/GitConvention.md) 1장 브랜치, 3장 병합

### 서드파티 관리 방식

- **상태**: 방향은 정했다(판단 근거는 루트 설계 기록의 프로젝트 설정 결정 기록 "서드파티 관리 — vcpkg manifest"). vcpkg 연결과 라이브러리 설치(아래 순서 1~3)는 적용하고 확인했다([Project Settings](ProjectSettings.md) "vcpkg"). 저장소 안에서 컴파일하는 외부 코드의 연결(순서 4)이 남았다. 모두 적용을 마치면 결과를 [Project Settings](ProjectSettings.md)와 [Code Convention](Conventions/CodeConvention.md) 6.3 예외 조항에 옮기고 이 항목을 지운다.
- **진행 시점**: 저장소 안에서 컴파일하는 외부 코드를 들일 때. CI 도입이나 다른 PC에서 빌드하는 일이 먼저 오면 그때.
- **풀려는 문제**
  - **엔진 설정의 적용 범위**: `Directory.Build.props`는 저장소 아래 모든 `.vcxproj`에 자동으로 적용된다. 엔진용 설정(`/W4`, 경고를 오류로 처리, `SDLCheck`, 예외 / RTTI 끔, `_HAS_EXCEPTIONS=0`, `/permissive-`, `/w14668` `/w14265`)이 저장소 안에서 컴파일하는 외부 `.cpp`에도 걸린다. `<...>`로 include한 외부 헤더만 `TreatAngleIncludeAsExternal` + `ExternalWarningLevel`로 경고가 꺼진다.
  - **classic 창고에 기대는 상태**: 개발 PC는 `C:\vcpkg`(classic 모드)에 boost가 있고 사용자 전역 통합(`%LOCALAPPDATA%\vcpkg\vcpkg.user.props` / `.targets`)이 켜져 있다. 이 상태에서는 그 창고의 include 경로와 `lib\*.lib`가 SeonEngine에도 붙는다. 여기에 라이브러리를 설치해 쓰면 다른 PC에서 빌드가 안 되고, 버전이 PC와 시점마다 달라진다.
- **결정**

  | 항목 | 결정 |
  |---|---|
  | 외부 라이브러리 (DirectXMath 등) | vcpkg manifest. `Engine/ThirdParty/vcpkg.json`에 적고 `builtin-baseline`으로 버전 선택을 기록한다. 게임 쪽에만 필요한 라이브러리가 생기면 `vcpkg.json`을 저장소 루트로 옮긴다 |
  | triplet | `x64-windows-static-md` 우선 (정적 라이브러리 + CRT `/MD`). 엔진은 `RuntimeLibrary`를 지정하지 않아 Debug `/MDd`, Release `/MD`다. `x64-windows-static`은 CRT가 `/MT`라 맞지 않는다 |
  | vcpkg 연결 | 프로젝트에서 vcpkg의 `vcpkg.props` / `vcpkg.targets`를 명시적으로 import한다. 사용자 전역 통합은 `VCPkgLocalAppDataDisabled`로 끊는다. vcpkg 위치는 `VCPKG_ROOT` 환경 변수, 없으면 VS 내장 vcpkg(`$(VsInstallRoot)\VC\vcpkg\`) |
  | 외부 코드 컴파일 | `Engine.vcxproj` 안에서 컴파일하고, 외부 소스에만 경고를 완화한다 |
  | 예외 / RTTI | 현재 정책(끔) 유지. 외부 코드에 필요하다고 확인되면 따로 검토한다 |

- **서드파티 공통 규칙**
  1. 외부 코드는 `Engine/ThirdParty/` 아래에만 둔다.
  2. 외부 헤더는 `<...>`로 include하고 외부 include 경로로 등록한다.
  3. 저장소 안에서 컴파일하는 외부 `.cpp`에는 필요한 설정만 완화한다(우선 경고). 생성기가 관리하는 소스 목록이 아니라 라이브러리별 import 파일에서 소스 목록과 파일별 설정을 함께 관리한다. 연결이 복잡해지면 외부 코드 전용 정적 라이브러리 프로젝트로 나눈다(그 프로젝트도 `Directory.Build.props`를 물려받으므로 안에서 덮어쓰고, `GenerateProjectFiles`가 `ThirdParty`를 건너뛰므로 파일 목록을 따로 관리한다).
  4. vcpkg 패키지가 있고 요구 기능과 빌드 구성을 만족하면 vcpkg manifest를 우선한다. 패키지가 없거나 고쳐 써야 하는 라이브러리는 다른 방식을 쓸 수 있다.
  5. 외부 코드의 예외 / RTTI 정책은 경고 완화와 따로 정한다. 켤 때는 전역 `_HAS_EXCEPTIONS=0`과 섞이는 영향, 외부 예외가 엔진 호출 경계를 넘는 문제를 함께 검토한다.
  6. 엔진이나 게임 코드에서 직접 include하는 라이브러리는, 다른 라이브러리의 전이 의존성으로 들어오더라도 `vcpkg.json`에 직접 적는다.
- **새 PC 준비 조건**: VS의 vcpkg 구성 요소를 설치하거나 `VCPKG_ROOT`를 지정한다. `vcpkg.json`과 설정값만으로는 vcpkg와 MSBuild가 연결되지 않는다.
- **적용과 검증 순서**
  1. ~~vcpkg 연결 구성: 명시적 import, 전역 통합 끊기, `VcpkgEnableManifest` 켜기, triplet 지정~~ (완료. 빈 `vcpkg.json`으로 확인)
  2. ~~manifest 복원: `vcpkg.json`에 `directx-headers`, `directxtex`, `builtin-baseline`을 넣고 빌드해 `Intermediate\vcpkg_installed\`에 설치되는지. 고른 baseline과 툴셋 v145에서 설치가 성공하는지~~ (완료. 이후 사용처가 없어 `directx-headers`, `directxtex`는 목록에서 뺐다. 현재 목록은 [Project Settings](ProjectSettings.md) "vcpkg")
  3. ~~classic 경로 제외 확인: 빌드 로그와 `*.command.1.tlog`에 `C:\vcpkg\installed`가 없는지~~ (완료. Debug / Release 모두)
  4. 외부 코드 컴파일과 최종 링크. 정적 라이브러리는 호출되지 않은 코드를 링크에 넣지 않으므로 실제로 호출한 뒤 `SampleGame.exe` 링크를 확인한다. 가능하면 다른 PC에서 clone → 빌드까지 확인한다
- **적용하면서 확인할 것**: 외부 include 경로 등록 방법(`ExternalIncludePath` 등)
- **반영할 곳**: [Project Settings](ProjectSettings.md) 공통 설정, [Code Convention](Conventions/CodeConvention.md) 6.3 예외 조항

### 빌드 시스템 / 프로젝트 생성기

- **현재 상태**: 우선 엔진 작업을 위해 전환은 보류하고 `.vcxproj` / 현재 생성기를 유지한다. CMake는 유력한 검토 후보이며 채택은 미확정이다. 보류 근거와 전환 비용은 [프로젝트 설정 결정 기록](../../Docs/ProjectSetupDecisions.md#cmake-전환--우선-작업-이후-재검토-2026-10-01) 참고.
- **진행 시점**: 아래 중 하나가 필요할 때
  - 우선 작업을 마친 뒤, 자체 생성기나 빌드 단계에 새 책임을 추가하기 전
  - 엔진 모듈별 프로젝트 분리: 모듈마다 의존성과 include 경로를 손으로 맞추기 번거로워질 때
  - 리플렉션 코드 생성: 헤더를 분석해 코드를 만드는 단계를 빌드에 넣어야 할 때
  - 두 번째 플랫폼이나 IDE 지원: `.vcxproj`는 Windows / Visual Studio 전용
- **정할 것**
  - 도구 선택: Sharpmake(C#, Unreal `*.Build.cs`와 비슷한 사용감) / premake(Lua) / CMake / 자체 도구
  - 모듈 의존성 선언 방식 (Unreal `PublicDependencyModuleNames` / `PrivateDependencyModuleNames` 참고)
  - 생성된 `.sln` / `.vcxproj`를 git에 올릴지 여부
  - 서드파티와의 설정 분리: `.vcxproj` + `Directory.Build.props`는 설정이 저장소 전체에 적용된다. CMake는 컴파일 설정을 타깃마다 두기 쉽지만, 전역 설정을 주면 외부 타깃에도 영향을 준다. 서드파티 문제를 풀기 위해 꼭 바꿔야 하는 것은 아니고, 도구를 고를 때 비교 기준 중 하나로 쓴다.
- **전환을 선택하면 확인할 것**
  - Engine / SampleGame의 Debug / Release 빌드, 최종 링크, 실행을 먼저 재현한다.
  - CRT, 매크로 / 경고 설정, 출력 경로, 실행 작업 폴더, 리소스 복사와 IDE 표시를 비교한다.
  - 이후 외부 코드 타깃 / 컴파일 정책과 vcpkg manifest 복원을 연결한다. 가능한 경우 다른 PC에서도 확인한다.
  - 기존 `GenerateProjectFiles`와 생성 파일의 역할을 정하고, 같은 빌드 규칙을 두 방식에서 계속 수동 관리하지 않도록 전환 완료 기준을 정한다.
- **반영할 곳**: [Project Settings](ProjectSettings.md), [Architecture](Architecture.md) 1. 모듈과 의존성 방향

---

## 엔진 기반 시스템

### 플랫폼별 기본 타입 정의

- **진행 시점**: Windows가 아닌 두 번째 플랫폼을 지원할 때
- **현재**: 정수 별칭(`int32` 등)을 `Core/Public/CoreTypes.h` 한 곳에서 기본 타입에 직접 정의한다. Windows(MSVC)만 지원하므로 플랫폼별 계층이 없다.
- **정할 것**
  - 플랫폼별로 덮어쓰는 구조. 참고: Unreal은 `FGenericPlatformTypes`(기본값)를 `FWindowsPlatformTypes` 등이 상속해 바꾸고, `FPlatformTypes`로 선택한 뒤 전역 별칭을 만든다.
  - 같은 계층에 함께 둘 플랫폼 타입(`SIZE_T`, 문자 타입 등)
  - 외부 라이브러리의 `int64_t`와 엔진 `int64`가 다른 타입이 되는 플랫폼(Linux의 `long`)에서의 변환 처리
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 3.12 STL 사용 정책, [Architecture](Architecture.md) 3. 플랫폼 추상화

### 서브시스템 등록 / 조회

- **진행 시점**: `FEngine`과 첫 서브시스템을 구현할 때
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
  - 등록 시점(`FEngine::Initialize()`)과 초기화 / 종료 순서(Architecture 2. 서브시스템과 전역 상태)의 관계
  - 없는 서브시스템을 조회했을 때의 처리 (nullptr / assert)
- **반영할 곳**: [Architecture](Architecture.md) 2. 서브시스템과 전역 상태

### 핸들 시스템

- **진행 시점**: 리소스(텍스처, 메시 등)나 게임 오브젝트 관리 시스템을 만들 때
- **정할 것**
  - 핸들 구조 (인덱스 + 세대 번호, 비트 배분)
  - 타입별 핸들 (`FTextureHandle` 등) 정의 방식
  - 핸들 → 객체 변환(`Resolve`)과 무효 핸들 처리
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 4.1 메모리 소유권

### 메모리 할당자

- **진행 시점**: 메모리 사용량 추적, 누수 검사, 프레임 단위 임시 할당 등이 필요해질 때
- **정할 것**
  - 엔진 할당자 인터페이스와 종류 (일반 / 프레임 / 풀 등)
  - STL 별칭(`TArray` 등)에 할당자를 넣는 방식
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 3.12 STL 사용 정책, 4.1 메모리 소유권

### null이 될 수 없는 공유 참조 (`TSharedRef`)

- **진행 시점**: 공유 소유 객체를 null 없이 주고받는 API가 반복될 때 (UI 위젯 트리 등). 빌려 쓰기만 하면 레퍼런스(`T&`)로 충분하다.
- **현재**: `TSharedPtr`(`std::shared_ptr` 별칭)만 있다. `std`에 대응하는 타입이 없어서 별칭으로는 만들 수 없고 클래스를 직접 만들어야 한다.
- **정할 것**
  - 이동 처리. Unreal `TSharedRef`는 null이 되지 않도록 이동 생성자에서 복사한다(참조 카운트 증가). 같은 방식을 따를지 정한다.
  - `TSharedPtr`와의 변환(`TSharedRef` → `TSharedPtr`는 자유롭게, 반대는 null 검사 후)
  - `MakeShared`의 반환 타입을 `TSharedRef`로 바꿀지 (Unreal은 `TSharedRef`를 돌려준다)
  - 직접 만들지, `gsl::not_null<TSharedPtr<T>>` 같은 래퍼를 쓸지
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 4.1 메모리 소유권

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
  - 렌더 스레드를 둘 때: 렌더 씬(`IRenderScene`) 호출을 렌더 스레드로 보내는 명령으로 바꾸는 방식, 렌더러가 따로 들고 있을 데이터(Proxy / 스냅샷). Unreal은 컴포넌트가 `CreateRenderState_Concurrent` / `SendRenderTransform_Concurrent`로 변경을 넘긴다.
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

- **진행 시점**: D3D12나 Vulkan 등 두 번째 그래픽스 API를 추가할 때. 계획한 순서는 D3D11 → D3D12 → Vulkan이다.
- **범위**: SeonEngine 렌더러 안의 경계다. 렌더러 인터페이스(Architecture 4장)는 바꾸지 않는다. 다른 프로젝트의 렌더러 구현은 영향을 받지 않는다.
- **진행 방식**: D3D11 전용 폴더의 코드와 새 D3D12 코드를 비교하며 공통 인터페이스를 추출한다. 백엔드가 하나일 때 미리 만들면 설계를 검증할 수 없다.
- **정할 것**
  - 구조: Renderer → RHI(공통 인터페이스) → 백엔드(D3D11RHI, D3D12RHI 등). Renderer 이상은 RHI 타입만 쓰고, API 타입은 백엔드 안에서만 쓴다.
  - 연결 방식: 가상 함수 인터페이스(Unreal `FDynamicRHI`와 같은 방식)를 우선 검토한다.
  - 리소스 상태 전환을 누가 처리할지: 낮은 쪽 API에 맞춤(sokol_gfx, bgfx) / RHI가 상태를 자동 추적(NVRHI) / 명시적 전환(D3D12 방식, Unreal `RHITransition`). D3D11 모양으로 굳히면 D3D12 / Vulkan을 붙일 때 RHI를 크게 고쳐야 한다.
  - 백엔드 선택 시점: 실행할 때(Unreal처럼 실행 옵션) / 빌드할 때
  - 셰이더 소스 공유: D3D11(셰이더 모델 5.x)과 D3D12(6.x, DXC)를 함께 지원하는 방법, Vulkan용 SPIR-V 변환
  - Vulkan / OpenGL의 차이(깊이 범위, 클립 공간 Y 방향, 텍스처 원점)는 백엔드에서 처리한다. OpenGL은 RHI 백엔드가 아니라 렌더러 인터페이스 아래의 별도 렌더러가 될 수 있다.
- **반영할 곳**: [Architecture](Architecture.md) 4. 렌더링

### Scene Proxy 방식

- **진행 시점**: 스키닝, 지형, 파티클 등 물체 종류가 늘면서 등록 함수와 분기가 반복되거나, 물체별 렌더링 준비 코드 / 데이터 / 캐시 관리가 복잡해질 때. 렌더 스레드를 넣을 때도 검토한다.
- **현재**: 핸들과 값을 등록하고 변경 사항만 갱신한다. Proxy 도입은 미정이며, 렌더 스레드 도입의 필수 조건은 아니다.
- **정할 것**
  - 종류별 등록 함수와 값 구조체 / 명령 큐와 스냅샷 / 공통 Proxy 계약 중 필요한 범위
  - Proxy의 데이터와 동작, 생성 / 갱신 / 제거를 담당할 곳, 스레드 간 전달과 파괴 시 수명
  - 월드 / 컴포넌트 직접 접근 없이 공통 렌더링 계약으로 표현할 수 있는지
  - 외부 렌더러와 어댑터의 변환 가능성, 기능 지원 조회, 특정 셰이더 / 패스 / API 의존 여부
- **검증할 것**: 등록 / 갱신 / 제거, 오래된 참조 거부, 렌더 작업 중 제거, 외부 구현의 기능 지원과 변환
- **반영할 곳**: [Architecture](Architecture.md) 4. 렌더링. 기존 수명과 데이터 전달 규칙을 유지할지, 개정할지도 함께 확인한다.

### 여러 렌더 씬 / 여러 뷰

- **진행 시점**: 에디터, 에셋 미리보기, 분할 화면처럼 씬이나 뷰가 둘 이상 필요할 때
- **정할 것**
  - 선택 기능의 지원 여부를 알려 주는 방식 (함수별 조회 / 기능 목록 구조체)
  - 여러 뷰를 묶는 단위가 필요한지 (Unreal `FSceneViewFamily`)
- **반영할 곳**: [Architecture](Architecture.md) 4. 렌더링의 "필수 기능과 선택 기능"

### 렌더 리소스 소유자

- **진행 시점**: 에셋 시스템을 설계할 때
- **정할 것**
  - 메시를 파일 경로로 넘길지(지금 설계, 렌더러가 파일을 읽음), 엔진이 읽은 정점 데이터로 넘길지. 렌더러 구현마다 파일을 읽으면 같은 에셋을 다르게 해석할 수 있다.
  - 원본 머티리얼 정보를 담는 엔진 쪽 리소스와, 렌더러 머티리얼 핸들과의 대응을 관리하는 곳
  - GPU 리소스를 렌더러가 만들어 핸들로 줄지(지금 방식), 에셋이 소유할지(Unreal `FStaticMeshRenderData`, `FMaterialRenderProxy`). "핸들 시스템"과 함께 정한다.
- **반영할 곳**: [Architecture](Architecture.md) 4. 렌더링

---

## 에디터

### 실행 파일 구성 (엔진 실행 파일 + 게임 DLL)

- **진행 시점**: 에디터 개발을 시작할 때, 또는 게임 코드 없이 콘텐츠만으로 실행해야 할 때
- **현재**: 진입점은 `Engine.lib`에 있고, 게임 프로젝트(`SampleGame.exe`)가 엔진과 게임 코드를 한 실행 파일로 링크한다.
- **목표 구조**: 엔진 소유 실행 파일(`Launch.exe`, 에디터는 별도 exe)이 게임 모듈을 DLL로 실행 중에 불러온다. 게임 코드가 없으면 엔진만으로 실행한다. 게임 코드를 고쳐도 엔진을 다시 빌드하지 않는다(핫 리로드 가능). 배포용은 엔진과 게임을 한 실행 파일로 정적 링크하는 구성을 유지할 수 있다(Unreal의 에디터 / 패키징 빌드 참고).
- **정할 것**
  - 엔진 모듈의 DLL화: 게임 DLL과 엔진 실행 파일이 엔진 정적 라이브러리를 각각 링크하면 `gEngine`과 static 변수가 양쪽에 따로 생긴다. 엔진도 DLL로 만든다.
  - 공개 API export 매크로 (예: `SE_ENGINE_API`, `SE_CORE_API`)와 정적 링크 빌드에서 비우는 방법
  - DLL마다 static 타입 ID가 달라지는 문제("서브시스템 등록 / 조회" 참고)
  - DLL 경계를 넘는 메모리 할당 / 해제 규칙
  - 게임 DLL을 찾는 방법 (프로젝트 경로 인자 등, "프로젝트 경로와 `Saved` 폴더" 참고)
  - 게임을 엔진에 등록하는 방식. 정적 라이브러리에서는 참조되지 않는 `.obj`가 링크에서 빠지므로 전역 객체 생성자로 자동 등록하는 방식은 동작하지 않을 수 있다.
- **반영할 곳**: [Architecture](Architecture.md) 1. 모듈과 의존성 방향, 3. 플랫폼 추상화의 진입점, [Project Settings](ProjectSettings.md) 프로젝트

### 리플렉션

- **진행 시점**: 범용 인스펙터와 씬 직렬화를 구현할 때. 첫 ImGui 연결 단계에서는 생략할 수 있다. 필요성과 Undo/Redo와의 관계는 [에디터 결정 기록 1.1](../../Docs/EditorDecisions.md#11-리플렉션의-필요성과-도입-범위) 참고.
- **정할 것**
  - 구현 방식: 매크로 수동 등록 / 코드 생성기(libclang 등) / C++26 정적 리플렉션(MSVC 지원 확인 필요)
  - 초기에는 매크로를 통한 수동 등록으로 시작하고, 타입이 많아지면 다른 방식으로 옮기는 것을 검토한다.
- **반영할 곳**: [Architecture](Architecture.md)

### GC 관리 객체

- **진행 시점**: 에셋이나 게임 오브젝트처럼 수명을 엔진이 관리해야 하는 객체 체계가 필요할 때
- **정할 것**
  - GC 방식과 기반 클래스 (예: `UObject`)
  - `U` 접두사 유지 여부: Unreal 관례라 리플렉션, `NewObject`, 마크 앤 스윕 GC를 떠올리게 한다. 수명 정책이 Unreal UObject와 크게 다르면(참조 카운팅 등) 글자를 다시 검토한다.
  - 생성 / 참조 규칙 (`new` / `delete` 금지, 전용 생성 함수, 추적되는 참조 등)
  - 리플렉션과의 관계
- **반영할 곳**
  - [Code Convention](Conventions/CodeConvention.md) 1.2 Naming: `U` 접두사 예약 해제
  - [Code Convention](Conventions/CodeConvention.md) 4장 Memory & Lifetime

### 에디터용 에러 메시지 전달

- **진행 시점**: 에디터나 도구에서 사용자에게 실패 원인을 보여줘야 할 때
- **정할 것**
  - 사용자에게 보여줄 메시지를 넘기는 방식. Unreal처럼 `FString& outErrorMessage` out 매개변수를 쓰는 방식을 우선 검토한다.
  - 코드 분기용 에러 코드 enum(5.1 에러 처리)과 함께 쓰는 기준
- **반영할 곳**: [Code Convention](Conventions/CodeConvention.md) 5.1 에러 처리
