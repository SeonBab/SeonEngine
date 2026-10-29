# 빌드·개발 설정

이 문서는 저장소를 받아 프로젝트를 생성·빌드하고 소스를 편집하는 데 필요한 현재 설정을 다룬다. 설정값의 기준은 아래에 명시한 실제 파일이다.

## 개발 환경과 빌드

- Windows x64, Visual Studio 2022 17.14 이상 또는 Visual Studio 2026의 C++를 사용한 데스크톱 개발 워크로드와 Windows SDK 10.0 계열을 사용한다. VS 2022에는 MSVC v143, VS 2026에는 MSVC v145가 필요하다.
- 프로젝트 파일 생성에는 .NET 10 이상 SDK가 필요하다. 런타임만 설치한 환경은 사용할 수 없다.
- Visual Studio Installer의 vcpkg 패키지 관리자 구성 요소를 설치하거나 `VCPKG_ROOT`에 별도 vcpkg 경로를 지정한다.

프로젝트 파일 목록을 생성한 뒤, 사용하려는 Visual Studio 버전으로 `SeonEngine.slnx`를 연다. 두 버전이 함께 설치되어 있다면 탐색기 더블클릭의 기본 연결 대신 해당 IDE의 솔루션 열기를 사용한다. VisualStudioVersion 17.0은 v143, 18.0은 v145를 선택하며, 아래 명령도 해당 버전의 개발자 명령 프롬프트에서 실행한다.

구성은 Debug와 Release, 플랫폼은 x64다. 솔루션에는 루트의 SeonEngine 실행 프로젝트 하나가 있으며 Engine/Source와 Game/Source를 함께 컴파일한다.

Visual Studio 개발자 명령 프롬프트에서는 다음과 같이 빌드한다.

```bat
GenerateProjectFiles.bat
MSBuild SeonEngine.slnx /p:Configuration=Debug /p:Platform=x64 /p:PreferredToolArchitecture=x64
MSBuild SeonEngine.slnx /p:Configuration=Release /p:Platform=x64 /p:PreferredToolArchitecture=x64
```

첫 빌드는 vcpkg 패키지 다운로드·컴파일을 수행할 수 있으므로 네트워크와 설치·캐시 경로의 쓰기 권한이 필요하다.

## 폴더 구조

```text
SeonEngine/
├─ SeonEngine.slnx
├─ SeonEngine.vcxproj          엔진·게임을 함께 빌드하는 실행 프로젝트
├─ Directory.Build.props       공통 컴파일·출력·vcpkg 설정
├─ Directory.Build.targets     vcpkg 연결
├─ GenerateProjectFiles.bat
├─ Engine/
│  ├─ Build/BatchFiles/         프로젝트 파일 생성 도구
│  ├─ Source/                  엔진 소스와 공통 include 기준
│  │  ├─ Core/                 공통 타입·컨테이너·수학·도구
│  │  ├─ Components/           Actor 컴포넌트
│  │  ├─ Launch/               엔진 진입점·실행부
│  │  └─ …                     D3D11·Renderer·Image·Input·UI·Windows 등
│  ├─ Shaders/Private/         HLSL 소스
│  ├─ Content/                 엔진 콘텐츠
│  └─ ThirdParty/              vcpkg manifest·외부 라이브러리 고지
├─ Game/                       게임 전용 파일
│  ├─ Source/                  게임 소스
│  └─ Content/                 게임 콘텐츠
├─ Docs/                      개발 규칙·빌드 안내
├─ Binaries/                  빌드 결과（Git 제외）
└─ Intermediate/              중간 결과·vcpkg 설치 파일（Git 제외）
```

현재 별도의 Engine.vcxproj나 Engine.lib는 만들지 않는다. 엔진과 게임의 책임은 폴더로 구분하며 include 검색 기준은 `Engine/Source`다. 엔진 모듈마다 Public/Private 폴더를 두는 구성은 사용하지 않는다.

## 렌더러 구현 연결

어댑터의 구현 순서, 입력 데이터와 수명, 에디터 지원 및 검증 기준은 [다른 렌더러 연결 안내](RendererIntegration.md)를 따른다.

실행 구성을 제공하는 쪽에서 `Renderer/RendererFactory.h`의 `CreateRenderer()`를 정확히 하나 정의한다. 기본 샘플 정의는 `Game/Source/RendererFactory.cpp`에 있으며 미초기화 `FD3D11Renderer`의 단독 소유 포인터를 반환한다.

- 다른 구현을 연결하려면 샘플 생성 함수 정의를 제외하고 자기 생성 함수와 `IRenderer` 구현 소스를 빌드에 포함한다. 엔진의 Loop 코드를 수정할 필요는 없다. 두 생성 함수 정의를 함께 링크하지 않는다.
- 생성 함수는 호출할 때마다 새 미초기화 객체를 반환한다. `nullptr`는 초기화 실패로 전달하며 기본 구현으로 자동 대체하지 않는다. 생성 함수 안에서 창을 만들거나 GPU 초기화를 수행하지 않는다.
- Loop가 창과 크기를 준비한 뒤 생성 함수를 호출하고 `Initialize`를 수행한다. Loop는 구현을 단독 소유하며 엔진과 UI를 먼저 정리한 뒤 `Shutdown`, 객체 파괴, 창 종료 순서로 정리한다. 같은 Loop의 다음 `Run`은 생성 함수를 다시 호출한다.
- 생성 함수만 바꿔도 모든 기능이 지원되는 것은 아니다. 구현은 사용할 실행 모드와 콘텐츠가 요구하는 자원, 장면, UI와 출력 기능 및 공통 수명 계약을 지원해야 한다.

현재는 엔진과 게임을 하나의 실행 프로젝트에서 컴파일한다. 이 연결은 별도 엔진 라이브러리나 DLL 로딩을 제공하지 않는다.

## 설정 파일과 출력 경로

| 파일 | 관리하는 설정 |
|---|---|
| `Directory.Build.props` | `SERootDir`, VS 버전별 `SEPlatformToolset`, 출력·중간 디렉터리, 공통 컴파일러 옵션, vcpkg manifest 설정 |
| `Directory.Build.targets` | 프로젝트 본문 이후의 도구별 overlay triplet 연결, vcpkg targets import와 경로 검사 |
| `SeonEngine.vcxproj` | 실행 프로젝트, 선택한 `SEPlatformToolset`, SDK, include·링크, 구성별 CRT·최적화, HLSL 컴파일·콘텐츠 복사 |
| `SeonEngine.slnx` | 솔루션 프로젝트와 플랫폼 |

- 실행 파일: `SeonEngine.exe`
- 로컬 디버거 인수는 Git 제외 `SeonEngine.vcxproj.user`에서 설정한다. `-game`은 게임 모드이며 인수 없으면 Editor다.
- 출력: `$(SERootDir)Binaries\$(SEPlatformToolset)\$(Platform)\$(Configuration)\`
- 중간 결과: `$(SERootDir)Intermediate\$(SEPlatformToolset)\$(MSBuildProjectName)\$(Platform)\$(Configuration)\`
- vcpkg 설치: `$(SERootDir)Intermediate\vcpkg_installed\$(SEPlatformToolset)\`

다른 폴더를 참조할 때는 저장소 루트인 `$(SERootDir)`을 쓴다. 공통 옵션은 props/targets에서 수정하며 VS 속성 페이지에서 바꾸면 해당 프로젝트에 저장될 수 있다. 목록형 설정은 `%(PreprocessorDefinitions)`, `%(AdditionalIncludeDirectories)`, `%(AdditionalOptions)`처럼 기존 값을 이어 붙인다.

## 프로젝트 파일 생성

소스를 추가·삭제·이동한 뒤 `GenerateProjectFiles.bat`을 실행한다. 본체는 `Engine/Build/BatchFiles/GenerateProjectFiles.cs`이며 도구 빌드 결과는 `Intermediate/DotNET/GenerateProjectFiles`에 둔다.

- 저장소의 `.vcxproj`를 찾되 Binaries·Intermediate·ThirdParty 경로는 제외한다.
- 루트 `SeonEngine.vcxproj`는 `Engine/Source`, `Engine/Shaders`, `Game/Source`만 수집한다. 저장소 전체를 소스로 검색하지 않는다. 다른 프로젝트의 기본 검색 범위는 프로젝트 폴더이며 Source 프로젝트의 형제 Shaders 폴더도 포함한다.
- `.h`는 ClInclude, `.cpp`는 ClCompile, 명시적으로 컴파일하지 않는 `.hlsl`·`.hlsli`는 편집용 None 목록에 넣는다. `.vcxproj.filters`는 폴더 구조에 맞춰 생성한다.
- 루트 프로젝트의 `Engine/Content`와 `Game/Content` 파일은 확장자와 관계없이 재귀 수집해 `SEGeneratedContent` 그룹과 필터에 표시한다. 추가·삭제 후 생성기를 실행하면 IDE 목록이 갱신된다. 이 표시 목록을 빌드 복사 목록으로 사용하지는 않는다. 외부 폰트 원본과 고지 등 수동 등록은 보존하며, 자동 관리 루트의 조건이나 파일별 메타데이터는 발견 시 쓰기 전에 거부한다.
- 생성 대상 소스 목록과 필터는 생성기를 통해 관리한다. 자동 목록의 파일별 메타데이터·Condition·다른 종류 항목은 지원하지 않으며 발견하면 쓰기 전에 오류로 중단한다.
- 셰이더 FxCompile 전용 목록과 컴파일 메타데이터는 프로젝트에서 직접 관리한다. 검색 범위 안의 단일 `.hlsl` 상대 경로만 허용하며 중복·변수·와일드카드는 거부한다. 생성기는 FxCompile을 보존하고 None 중복을 제거한다.
- 내용이 바뀐 파일만 BOM 없는 UTF-8·CRLF로 쓴다. 필터 이름은 프로젝트 상대 경로의 앞쪽 `..`를 제거해 표시한다.

## 리플렉션 코드 생성

### 일반 빌드 연결

루트 `SeonEngine.vcxproj`는 Directory.Build.targets에서 `Engine/Build/Reflection.targets`를 import한다. 일반 MSBuild/VS 빌드의 PrepareForBuild 전에 SEGenerateReflection이 프로젝트의 평가된 ClInclude 항목과 공통 ClCompile 설정을 JSON으로 작성하고 GenerateReflection.cs를 실행한다. 생성 성공 후 GeneratedSources.txt의 현재 cpp만 빌드 항목에 추가한다. 프로젝트 파일에 생성 cpp 목록을 영구 기록하지 않으며 GenerateProjectFiles가 생성기를 실행하는 방식도 아니다. 새 원본 헤더를 추가하면 기존 규칙대로 프로젝트 파일 목록을 갱신해야 한다.

출력은 `$(IntDir)Reflection/`, 생성 도구의 .NET 빌드 결과는 `$(IntDir)ReflectionTool/`이다. 기존 IntDir 규칙을 따라 프로젝트·도구 집합·플랫폼·구성별로 구분한다. Headers.json은 현재 헤더 목록, CompilerArgs.json은 평가된 매크로·추가 include·시스템/외부 include·CRT 구성, GeneratedSources.txt는 성공한 현재 생성 cpp 목록이다. 실제 MSBuild가 실행될 때마다 생성기를 호출하며, 동일한 입력 JSON과 생성 결과는 다시 쓰지 않는다. VS가 빌드 자체를 생략하면 이 대상도 실행되지 않는다.

.NET 10 SDK와 clang-cl이 필요하다. 기본 실행 명령은 `dotnet`, Clang 경로는 64비트 Program Files의 `LLVM/bin/clang-cl.exe`다. 설치 위치가 다르면 MSBuild 속성 SEReflectionDotNet과 SEReflectionClang으로 실행 파일을 지정한다. 예를 들어 `/p:SEReflectionClang=C:/Tools/LLVM/bin/clang-cl.exe`를 추가한다. 현재 연결은 Visual Studio의 MSBuild를 사용하는 루트 C++ 실행 프로젝트를 대상으로 한다. 게임 실행 파일이 .NET이나 Clang을 필요로 하는 것은 아니다.

입력 작성이나 생성 명령이 실패하면 C++ 컴파일로 진행하지 않는다. 성공한 목록의 경로가 출력 폴더 밖이거나 대응 원본 헤더를 하나로 찾을 수 없으면 거부한다. 생성 cpp에는 원본 헤더 폴더와 기존 공통 include 설정을 함께 전달한다. 생성 헤더 검색 경로는 평가 시점의 ClCompile 기본값에도 포함하므로 IntelliSense에서 같은 경로를 사용하지만 DesignTimeBuild에서는 생성 명령을 실행하지 않는다. 생성 전에는 파일이 아직 없어 IDE에서 찾지 못할 수 있다.

공통 분석 구성은 C++20·RTTI·/EHsc·PCH 미사용·/MD 또는 /MDd다. 수집하는 PreprocessorDefinitions, AdditionalIncludeDirectories, RuntimeLibrary, LanguageStandard, RuntimeTypeInfo, ExceptionHandling, PrecompiledHeader가 활성 cpp마다 다르면 조용히 합치지 않고 오류로 중단한다. 임의 MSVC 옵션 전체를 Clang 설정으로 변환하는 도구는 아니다. 현재 생성기가 지원하는 선언·타입 제한은 아래와 같다. 생성 파일은 직접 수정하지 않는다. 이전 생성 파일을 물리 삭제하지 않으므로 폴더 glob 대신 성공한 GeneratedSources.txt만 컴파일 대상으로 사용한다.

### 생성기 단독 실행

`Engine/Build/BatchFiles/GenerateReflection.cs`는 명시한 `.h` 하나 또는 헤더 목록을 분석해 기존 UObject/FPropertyInfo/FTypeInfo용 코드를 생성하는 독립 도구다. .NET 10 SDK, clang-cl과 MSVC/Windows SDK 헤더 환경이 필요하다. VS 개발자 명령 환경에서 실행하며 Clang 22.1.0을 기준으로 한다. 루트 제품 빌드는 위 연결로 실행하며, 아래 명령으로 생성기만 단독 실행할 수도 있다. GenerateProjectFiles는 원본 파일 목록을 관리하며 생성기를 실행하지 않는다.

참여 헤더마다 전역 UCLASS 클래스 하나, GENERATED_BODY 하나, 일반 float, 엔진 FVector3/FRotator 또는 TSharedPtr<UStaticMesh> 인스턴스 필드를 지원한다. FVector3/FRotator는 값 전체를 복사하며 내부 성분의 프로퍼티 정보를 생성하지 않는다. FRotator의 pitch/yaw/roll은 도 단위 저장값을 그대로 유지하며 정규화나 쿼터니언 변환을 수행하지 않는다. 실제 엔진 전역 정의와 필드 타입을 확인하고 동명 사용자 구조체·사용자 타입 별칭·본인/조상의 해당 타입 이름 가림은 거부한다. TSharedPtr<UStaticMesh>는 엔진 별칭 템플릿 한 종류를 명시적으로 허용하는 예외다. Core/Templates/SharedPtr.h와 Renderer/StaticMesh.h의 실제 전역 정의가 분석에 포함돼야 하며 단순 전방 선언은 허용하지 않는다. 원본 헤더에서 두 정의를 자기 generated.h보다 먼저 포함한다. 공유 참조의 읽기·대입 코드를 생성하며 메시 복제·파일 로딩·변경 알림을 수행하지 않는다. 직접 std::shared_ptr 표기, 다른 자산 타입, 별칭을 다시 씌운 타입, const 자산/참조·포인터 변형은 지원하지 않는다. 원본 헤더에서 사용할 타입의 Core/Math/Vector3.h 또는 Core/Math/Rotator.h를 포함해 타입을 먼저 선언한 뒤 자기 generated.h를 마지막으로 포함한다. 부모는 UObject 또는 UCLASS/GENERATED_BODY를 선언한 전역 참여 클래스이며 public 비가상 단일 상속이어야 한다. UObject까지 조상의 선언 계약을 확인하고 실제 직접 부모의 StaticTypeInfo를 연결한다. 참여하지 않은 부모·조상은 기반 정보를 대신 연결하지 않고 오류로 거부한다. 프로퍼티가 없는 참여 클래스도 허용한다. 클래스·필드·헤더 파일 이름은 ASCII 식별자이며 namespace/템플릿/중첩 참여 클래스·부모 타입 별칭·다중 상속·가상 상속·const/volatile/mutable/static/배열/비트필드 프로퍼티·매크로 옵션·#line은 지원하지 않는다. 지원하지 않는 입력은 오류로 거부한다. 일반 C++ 문법 자체는 Clang이 분석하며 조건부 컴파일은 전달한 구성에서 활성인 선언을 기준으로 한다. Clang AST JSON 읽기의 최대 깊이는128이며 초과 시 생성 오류로 중단하고 기존 결과를 유지한다. 경로/인수 JSON이나 맵 JSON의 제한과는 별개다.

사용자 헤더는 Object.h 뒤에 자기 `Example.generated.h`를 마지막 include로 두고, 클래스 안의 GENERATED_BODY 뒤에 public/private 등 접근 지정자를 명시한다. 생성 헤더가 공통 ReflectionMacros.h를 포함하므로 별도 include는 필수가 아니다. 표기와 SE_REFLECTION 계열 매크로를 사용자 코드에서 재정의하지 않는다.

컴파일 환경은 JSON 문자열 배열 파일로 전달한다. 예를 들어 `Intermediate/Reflection/DebugArgs.json`의 최소 구성은 다음과 같다. 포함하는 헤더가 요구하는 include·플랫폼 정의는 실제 C++ 구성과 일치시킨다.

```json
["/MDd", "/D_DEBUG", "/DSE_BUILD_DEBUG=1", "/DSE_BUILD_RELEASE=0", "/DSE_PLATFORM_WINDOWS=1", "/DSE_PLATFORM_HEADER_NAME=Windows"]
```

도구가 C++20·UTF-8·RTTI·분석 모드를 지정하고 Engine/Source include를 추가한다. JSON에서 추가할 수 있는 인수는 `/I경로`, `/external:I경로`, `/D이름=값`, `/MD`·`/MDd`다. 각 경로는 `/I`와 한 문자열로 전달한다. 도구 전용 표기·SE_REFLECTION 계열 정의와 임의 Clang 동작/출력 옵션은 전달하지 않는다. Release는 `/MD`, `/DNDEBUG`, Debug/Release 정의의 반대 값을 사용한다.

다음은 저장소 루트에서 실행하는 예다. Example.h는 사용자가 준비한 입력 예시이며 저장소의 샘플에 자동 추가되지 않는다. clang 경로는 설치 위치에 맞춘다.

```powershell
dotnet run --file Engine/Build/BatchFiles/GenerateReflection.cs --artifacts-path Intermediate/DotNET/GenerateReflection --property:UseSharedCompilation=false -- --header Game/Source/Example.h --output Intermediate/Reflection/Example --clang 'C:/Program Files/LLVM/bin/clang-cl.exe' --compiler-args Intermediate/Reflection/DebugArgs.json
```

여러 헤더는 `--header` 대신 `--headers 목록.json`으로 전달한다. 두 입력 모드를 함께 쓰지 않는다. 예를 들어 `Intermediate/Reflection/Headers.json`은 다음과 같은 JSON 문자열 배열이다. 상대 헤더 경로는 목록 파일의 폴더 기준이며 절대 경로도 허용한다.

```json
["../../Game/Source/Base.h", "../../Game/Source/Child.h"]
```

```powershell
dotnet run --file Engine/Build/BatchFiles/GenerateReflection.cs --artifacts-path Intermediate/DotNET/GenerateReflection --property:UseSharedCompilation=false -- --headers Intermediate/Reflection/Headers.json --output Intermediate/Reflection/Batch --clang 'C:/Program Files/LLVM/bin/clang-cl.exe' --compiler-args Intermediate/Reflection/DebugArgs.json
```

목록의 모든 항목은 존재하는 UTF-8 `.h`여야 한다. UCLASS 문자열이 없는 헤더는 분석 후보에서 제외하고, 후보의 실제 참여 여부는 현재 구성의 AST로 판정한다. 주석이나 비활성 전처리 구간에만 표시가 있으면 출력하지 않는다. 실제 참여 클래스의 모든 참여 조상 헤더도 목록에 포함한다. 경로 중복, 참여 클래스 이름 중복, 참여 헤더의 파일 이름 충돌은 거부한다. 목록 순서와 부모 우선 생성 순서는 요구하지 않는다. 모든 후보의 임시 generated.h를 먼저 준비하므로 부모 출력이 없는 첫 일괄 생성도 처리한다. 부모/자식은 동일한 구성으로 분석·컴파일한다.

출력은 저장소 Intermediate 아래의 별도 폴더로 제한한다. 참여 헤더별 `Example.generated.h`와 `Example.generated.cpp`를 생성한다. 목록 전체의 분석·검사·출력 문자열 준비가 성공한 뒤 파일을 갱신하고, 마지막에 현재 생성 cpp의 절대 경로 목록 `GeneratedSources.txt`를 쓴다. 목록은 경로순이며 각 경로를 한 줄에 기록한다. 참여 클래스가 없거나 입력이 빈 배열이면 빈 목록을 만든다. 호출자는 이 목록을 컴파일 대상으로 사용하며, 출력 폴더 전체를 glob으로 컴파일하지 않는다. 이전 파일은 삭제하지 않으므로 제거된 대상의 파일이 남아 있을 수 있다. 분석/검사 실패는 기존 생성 파일과 목록을 유지하고 종료 코드1을 반환한다. 쓰기 I/O 실패의 여러 파일 원자적 롤백은 제공하지 않으므로 실패한 실행의 결과로 빌드를 계속하지 않는다. 내용이 같으면 파일과 수정 시각을 유지한다. 단일 헤더 모드는 GeneratedSources.txt를 관리하지 않으며 두 모드의 출력 폴더를 구분한다.

단일 헤더 모드를 사용할 때 부모가 있는 클래스는 부모 헤더부터 생성하고, 자식 분석 시 부모 generated.h를 찾을 수 있게 JSON 인수에 생성 폴더의 `/I경로`를 추가한다. 예를 들어 Base.h → Child.h → Grand.h 순으로 같은 명령을 반복한다. 도구는 조상의 원본 선언도 검사하지만 한 번에 출력하는 것은 명시한 대상 헤더의 두 파일뿐이다. 부모/자식은 같은 구성의 매크로·include 조건으로 분석/컴파일하고 부모 선언을 바꾸면 부모 결과도 다시 생성한다.

단독 실행 결과를 직접 컴파일하는 호출자는 생성 폴더를 include 경로에 추가하고 생성 cpp도 컴파일해야 한다. 생성 cpp가 원본 헤더의 파일 이름으로 include하므로 원본 헤더 폴더도 include 경로에 필요하다. Object.cpp와 Reflection/TypeInfo.cpp를 링크한다. GENERATED_BODY 줄 번호나 헤더 위치를 바꾸면 다시 생성한다. generated.h/cpp를 직접 수정하지 않는다. 단독 도구 자체는 프로젝트 수정이나 이전 출력 삭제를 수행하지 않는다. 일반 빌드의 생성 cpp 연결은 위 Reflection.targets가 담당하며, 단독 실행용 출력과 일반 빌드의 출력 폴더를 구분한다.

## 컴파일러 설정

공통 설정의 기준은 `Directory.Build.props`다.

| 항목 | 값 |
|---|---|
| 언어·전처리기 | C++20, 표준 준수 모드, `/Zc:preprocessor` |
| 소스·실행 문자 집합 | `/utf-8` |
| 병렬 컴파일 | `/MP` (`MultiProcessorCompilation=true`) |
| 경고 | `/W4`, `/WX`, `/w14668`, `/w14265` |
| 외부 헤더 | 꺾쇠 include를 외부 헤더로 취급하고 외부 경고 끔 |
| C++ 예외 | 표준 C++ 예외 지원 `/EHsc` (`ExceptionHandling=Sync`), `_HAS_EXCEPTIONS` 별도 재정의 없음 |
| RTTI·SDL 검사 | 켬 |
| 플랫폼 정의 | `SE_PLATFORM_WINDOWS=1`, `SE_PLATFORM_HEADER_NAME=Windows` |
| Debug 정의 | `SE_BUILD_DEBUG=1`, `SE_BUILD_RELEASE=0`, `_DEBUG` |
| Release 정의 | `SE_BUILD_DEBUG=0`, `SE_BUILD_RELEASE=1`, `NDEBUG` |

SeonEngine 프로젝트는 PCH를 사용하지 않으며 `_CONSOLE`, Console 서브시스템, Unicode 문자 집합을 지정한다. Debug는 최적화 끔·`/MDd`, Release는 속도 최적화·`/MD`를 사용한다. 두 구성 모두 PDB 정보를 생성하며 Release는 함수 단위 링크·내장 함수·참조 제거·COMDAT 접기를 사용한다.

현재 직접 링크하는 Windows 라이브러리는 `dxgi.lib`, `d3d11.lib`, `user32.lib`, `windowscodecs.lib`, `ole32.lib`다.

## 외부 라이브러리

`Engine/ThirdParty/vcpkg.json`의 manifest 모드를 사용한다. 현재 manifest 목록은 DirectXMath, Assimp, RapidJSON, ImGui, DirectXTK이며 ImGui에는 docking-experimental·win32-binding·dx11-binding, DirectXTK에는 tools 기능을 지정한다. RapidJSON은 헤더 전용 JSON 라이브러리이며 별도 링크 라이브러리는 없다. manifest 등록은 제품 코드에서 해당 기능을 사용한다는 뜻은 아니다. 버전 기준은 manifest의 `builtin-baseline`이다.

- 사용자 전역 classic 통합은 `VCPkgLocalAppDataDisabled=true`로 끈다.
- `VcpkgEnableManifest=true`, manifest 루트는 `Engine/ThirdParty/`다.
- `VcpkgUseStatic=true`와 `VcpkgUseMD=true`로 `x64-windows-static-md` 구성을 사용한다.
- `Engine/ThirdParty/Triplets/v143/`와 `v145/`의 overlay triplet은 `VCPKG_PLATFORM_TOOLSET`을 각각 v143/v145로 고정한다. `Directory.Build.targets`가 프로젝트의 도구에 맞는 폴더를 `--overlay-triplets`로 전달한다. triplet 이름은 `x64-windows-static-md`를 유지하며 `/MD`·정적 라이브러리 연결 방식은 바꾸지 않는다.
- `VcpkgManifestInstalledBaseDir`와 `VcpkgInstalledDir`를 같은 도구별 설치 루트로 지정한다. VS 2022의 구형 vcpkg와 신형 vcpkg 모두 이 경로를 사용하며, 그 아래에 triplet별 include/lib가 설치된다.
- vcpkg 연결은 `VCPKG_ROOT`를 우선하고, 없으면 Visual Studio 내장 `VC/vcpkg`를 사용한다. targets를 찾지 못하면 빌드 오류로 안내한다.
- 설치 직전 `SESelectVcpkgVisualStudio`가 `VCPKG_VISUAL_STUDIO_PATH=$(VsInstallRoot)`를 MSBuild 프로세스 환경에 설정한다. vcpkg 대상 라이브러리는 현재 빌드를 수행하는 VS 설치 안에서 지정한 v143/v145 도구를 선택한다.
- 선택한 overlay triplet과 `Directory.Build.targets`도 vcpkg 설치 단계의 입력이다. 이 파일이 설치 완료 기록보다 새로우면 의존성을 다시 검사하며, 변경이 없으면 설치 단계를 생략한다. vcpkg 자체의 ABI 검사에 따라 필요한 패키지만 재빌드하거나 캐시에서 복원한다.
- vcpkg 명령을 직접 실행할 때는 manifest 루트뿐 아니라 `--x-install-root`에 도구별 설치 루트, `--overlay-triplets`에 해당 v143/v145 폴더를 지정한다. 명령을 실행할 VS의 설치 경로를 `VCPKG_VISUAL_STUDIO_PATH`에도 지정한다. IDE 빌드는 경로 인자와 설치 선택을 자동으로 전달한다.

배포 시의 고지 파일은 [Assimp와 의존 라이브러리 고지](../Engine/ThirdParty/Assimp/README.md)와 [DirectXTK 고지](../Engine/ThirdParty/DirectXTK/README.md)를 따른다.

### DirectXTK와 폰트 생성 도구

DirectXTK는 D3D11용 라이브러리이며 현재 baseline에서 2026-05-07#1(may2026)을 사용한다. 헤더는 `<directxtk/SpriteFont.h>`, `<directxtk/SpriteBatch.h>`로 포함한다. 도구별 설치 루트의 `x64-windows-static-md/lib/DirectXTK.lib`와 `debug/lib/DirectXTK.lib`가 각각 Release와 Debug용이다. 기존 vcpkg MSBuild 통합의 헤더·라이브러리 경로와 자동 링크를 사용하며 프로젝트에 별도 경로를 하드코딩하지 않는다.

tools 기능은 `x64-windows-static-md/tools/directxtk/makespritefont.exe`와 `xwbtool.exe`를 설치한다. 폰트 생성에는 MakeSpriteFont를 사용하고 XWBTool은 오디오용 별도 도구다. VS2026 기본 폰트 도구 경로는 `Intermediate/vcpkg_installed/v145/x64-windows-static-md/tools/directxtk/makespritefont.exe`이며 VS2022는 v143 설치 루트를 사용한다. 도구는 필요할 때 명시적으로 실행하며 일반 엔진 빌드에서 폰트를 자동 생성하지 않는다.

```powershell
& './Intermediate/vcpkg_installed/v145/x64-windows-static-md/tools/directxtk/makespritefont.exe' '<설치된 폰트 이름>' '<출력 경로>.spritefont' /FontSize:16 /CharacterRegion:0x20-0x7e /DefaultCharacter:0x3f
```

위 예시는 영문·숫자·기호와 미지원 문자 대체용 `?`를 포함한다. 제품에 사용할 폰트와 생성 파일의 배포 조건은 폰트별로 확인한다. 이 버전은 `/?`를 도움말 옵션으로 지원하지 않으며, 인수가 잘못되면 사용법을 출력하고 실패 코드를 반환한다.

## 셰이더와 콘텐츠 출력

`SeonEngine.vcxproj`의 FxCompile은 `StaticMeshVertexShader.hlsl`과 `ImageVertexShader.hlsl`을 Vertex로, `StaticMeshPixelShader.hlsl`과 `ImagePixelShader.hlsl`을 Pixel로 등록한다. 모두 Shader Model 5.0, 진입점 Main으로 컴파일한다. 경고는 오류로 처리하며 출력은 `$(OutDir)Shaders/`의 같은 이름 `.cso`다. 프로젝트가 컴파일 전 해당 디렉터리를 만든다.

빌드 후 `SESyncContent`가 현재 `Engine/Content`와 `Game/Content`를 다시 수집해 하위 경로를 유지하며 각각 `$(OutDir)Content/Engine/`과 `$(OutDir)Content/Game/`에 복사한다. 파일마다 프로젝트 설정을 수정할 필요는 없다. 복사 목록은 빌드 실행 때 수집하므로 생성기를 다시 실행하지 않아도 추가·수정·삭제를 반영한다. 변경이 없는 파일은 크기와 수정 시각으로 복사를 생략한다. IDE가 빌드 자체를 생략한 경우에는 동기화도 실행되지 않으므로 실제 빌드를 실행한다. 런타임 셰이더는 기존 `$(OutDir)Shaders/`를 유지한다.

`Content/.se-managed-files.txt`는 해당 출력 폴더에서 이전에 관리한 상대 경로 목록이다. 현재 원본에서 사라진 관리 파일만 삭제하며, 목록의 정규화된 경로가 Content/Engine 또는 Content/Game 밖으로 나가면 복사와 삭제 전에 실패한다. 관리하지 않는 출력 파일과 디렉터리는 유지한다. 기존 개별 복사 설정의 네 파일(이전 Content/Textures/Checker.png와 Content/Fonts의 폰트 및 고지 세 파일)은 새 배치로 복사한 뒤 정리한다. 콘텐츠 출력 경로가 충돌하거나 복사·삭제에 실패하면 빌드도 실패한다. 생성기는 이 동기화 Target을 보존한다.

폰트는 `SESyncContent`가 `Engine/Content/Fonts/DebugMono.spritefont`와 `Engine/ThirdParty/Fonts/NotoSansMono/`의 `OFL.txt`, `NOTICE.txt`를 `$(OutDir)Content/Engine/Fonts/`로 함께 복사한다. 생성 자산과 저작권·라이선스 고지는 함께 배포한다. 원본 TTF와 생성 조건은 [폰트 자산 안내](../Engine/ThirdParty/Fonts/NotoSansMono/README.md)에 보관하며, 일반 빌드에서는 폰트를 생성하거나 원본 TTF를 출력으로 복사하지 않는다. 생성 스크립트는 제공하지 않는다.

## 소스 인코딩과 서식

- `.editorconfig`: UTF-8 BOM 없음, CRLF, 파일 끝 newline. C++·HLSL·C#의 들여쓰기는 Tab, 폭 4다.
- `.clang-format`: 공통 코드 서식. 엔진 코드는 [코드 컨벤션](Conventions/CodeConvention.md)을 따른다. `Engine/ThirdParty/.clang-format`은 서드파티 서식·include 정렬을 끈다.
- 루트 `.clang-format`은 `Language`를 지정하지 않은 공통 기본 설정이며 `Standard: c++20`을 사용한다.
- `WhitespaceSensitiveMacros`에는 기본 다섯 이름과 `SE_STRINGIZE`를 함께 유지한다. 옵션은 YAML 끝 표시 `...` 앞에 두며 도구 버전을 바꾸면 `clang-format --dump-config`로 적용값을 확인한다.
- `.gitattributes`: 일반 텍스트는 저장소 LF·작업 폴더 CRLF, `.bat`·`.cmd`는 CRLF, `.sh`는 LF다. 바이너리는 변환하지 않는다. `Engine/ThirdParty/Assimp/Licenses/*.txt`는 `-text`로 원문 바이트를 유지한다.
