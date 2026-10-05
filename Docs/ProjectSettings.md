# Project Settings

프로젝트에 적용해 둔 설정이다.

## 언어 표준

모든 프로젝트, 모든 구성에서 C++20(`/std:c++20`)을 쓴다. (`Directory.Build.props`, 속성 페이지: C/C++ → 언어 → C++ 언어 표준)

전처리기는 표준 준수 전처리기(`/Zc:preprocessor`)를 쓴다. (`Directory.Build.props`의 `UseStandardPreprocessor`, 속성 페이지: C/C++ → 전처리기 → 표준 준수 전처리기 사용) 가변 인자 매크로에서 인자가 없을 때의 쉼표는 C++20 `__VA_OPT__`로 처리한다(`Format __VA_OPT__(,) __VA_ARGS__`). 비표준 확장 `, ##__VA_ARGS__`는 쓰지 않는다.

## 소스 인코딩

소스 파일은 UTF-8 (BOM 없음)으로 저장한다.

| 설정 | 위치 | 이유 |
|---|---|---|
| `/utf-8` | `Directory.Build.props` (속성 페이지: C/C++ → 명령줄 → 추가 옵션) | MSVC는 BOM 없는 파일을 시스템 코드페이지(CP949)로 읽기 때문에 한글 주석과 문자열이 깨진다. |
| `charset = utf-8` | `.editorconfig` | Visual Studio가 파일을 BOM 없는 UTF-8로 저장하게 한다. |

## `.editorconfig`

저장소 루트의 `.editorconfig`로 에디터 설정을 통일한다.

- 인코딩: UTF-8 (BOM 없음)
- 줄바꿈: CRLF
- 파일 끝 newline
- C++ / HLSL / C# 파일 들여쓰기: Tab, 폭 4

## `.clang-format`

`.clang-format`으로 코드 포매팅을 통일한다(Code Convention 1.1 Formatting, 적용 범위). clang-format은 파일에서 가장 가까운 폴더의 `.clang-format`을 읽는다. Visual Studio는 내장 clang-format으로 이 파일을 읽는다(도구 → 옵션 → 텍스트 편집기 → C/C++ → 코드 스타일 → 서식에서 clang-format 지원이 켜져 있어야 한다).

| 파일 | 적용 대상 | 내용 |
|---|---|---|
| `.clang-format` (저장소 루트) | 엔진 코드, 게임 코드 (게임 코드는 권장 기본값) | 공통 서식. 공백으로 줄을 맞추지 않고(`AlignConsecutiveAssignments` / `Declarations` / `Macros`: `None`), 줄 끝 주석만 맞춘다(`AlignTrailingComments`: `Always`) |

`Engine/ThirdParty/.clang-format`은 서드파티 폴더의 서식 정리와 include 정렬을 끈다(Code Convention 6.3 예외 조항).

- **`WhitespaceSensitiveMacros`** — 인자 안의 공백이 결과에 영향을 주는 매크로를 등록한다. 엔진의 `SE_STRINGIZE`(플랫폼 헤더 경로)와 clang-format 기본 다섯 이름(`BOOST_PP_STRINGIZE`, `CF_SWIFT_NAME`, `NS_SWIFT_NAME`, `PP_STRINGIZE`, `STRINGIZE`)을 함께 적는다. 이 목록은 기본 목록을 앞에서부터 덮어쓰므로, 기본 이름을 빼지 않으려면 함께 적어야 한다. clang-format 버전을 바꾸면 `clang-format --dump-config`로 실제로 읽힌 목록과 새 기본 이름을 확인한다.
- 옵션은 파일 마지막 줄 `...`(YAML 문서 끝 표시) 앞에 넣는다. 뒤에 넣으면 clang-format이 설정을 읽지 못한다.

## `.gitattributes`

줄바꿈을 각자의 `core.autocrlf` 설정과 관계없이 고정한다.

- 텍스트 파일: 저장소에는 LF, 작업 폴더에는 CRLF
- 이미지, 오디오, 폰트, 바이너리 파일: 변환하지 않음 (`binary`)
- 예외: `.bat` / `.cmd`는 CRLF, `.sh`는 LF로 고정

## 폴더 구조

저장소 루트에 솔루션을 두고, 엔진과 게임 프로젝트를 최상위 폴더로 나눈다. 빌드 결과는 루트에 모은다.

```text
SeonEngine/
├─ SeonEngine.slnx
├─ Directory.Build.props   모든 프로젝트 공통 설정 (먼저 읽힘)
├─ Directory.Build.targets 모든 프로젝트 공통 설정 (마지막에 읽힘, vcpkg 연결)
├─ GenerateProjectFiles.bat  프로젝트 파일 목록 / 필터 생성
├─ Engine/
│  ├─ Build/BatchFiles/   빌드 도구 스크립트
│  ├─ Source/
│  │  ├─ Engine.vcxproj
│  │  ├─ Core/            모듈 (Public/, Private/)
│  │  └─ Engine/          모듈
│  └─ ThirdParty/
│     └─ vcpkg.json       vcpkg로 받는 외부 라이브러리 목록 (manifest)
├─ SampleGame/            게임 프로젝트
│  └─ Source/
│     ├─ SampleGame.vcxproj
│     └─ SampleGame/      모듈 (Public / Private 없음)
├─ Docs/
├─ Binaries/              빌드 결과 (git 무시)
└─ Intermediate/          중간 파일 (git 무시)
   └─ vcpkg_installed/    vcpkg가 빌드할 때 설치하는 라이브러리
```

- 엔진과 게임 프로젝트는 같은 구조를 갖는다. 셰이더(`Shaders/`), 에셋(`Content/`), 설정(`Config/`) 폴더는 처음 필요할 때 각 폴더 아래에 추가한다.
- 게임 프로젝트 이름은 `<이름>Game`으로 짓고, 게임 모듈 이름은 프로젝트 이름과 같게 한다.
- `Engine/`과 게임 폴더에는 빌드 결과를 두지 않는다.

## 프로젝트

| 프로젝트 | 종류 | 담는 모듈 | include 경로 |
|---|---|---|---|
| `Engine` | 정적 라이브러리 (`.lib`) | `Core`, `Engine` | 두 모듈의 `Public`, `Private` |
| `SampleGame` | 실행 파일 (`.exe`, Windows 서브시스템) | `SampleGame` | 엔진 모듈의 `Public`, 자기 모듈 폴더 |

- `SampleGame`은 `Engine`을 프로젝트 참조로 연결한다. 빌드 순서와 `Engine.lib` 링크가 자동으로 처리된다.
- 진입점(`WinMain`)은 `Engine.lib`에 있다(Architecture 3. 플랫폼 추상화). 게임 프로젝트는 엔진과 게임 코드를 실행 파일로 링크하는 단위다.
- 게임 프로젝트에는 `.cpp`가 하나 이상 있어야 한다. 컴파일된 `.obj`가 없으면 CRT가 링크되지 않아 시작 코드(`WinMainCRTStartup`)를 찾지 못한다.
- 게임 프로젝트의 include 경로에는 엔진 모듈의 `Public`만 넣는다. 엔진 내부 헤더를 include하면 빌드가 실패한다(Code Convention 2.1 파일 구성 참고).
- 프로젝트에서 다른 폴더를 가리킬 때는 `$(SERootDir)`(저장소 루트)을 쓴다. `$(SolutionDir)`은 솔루션 없이 프로젝트만 빌드하면 값이 달라진다.

## 프로젝트 파일 생성 (`GenerateProjectFiles.bat`)

소스 파일을 추가, 삭제, 이동한 뒤 저장소 루트의 `GenerateProjectFiles.bat`을 실행한다. 각 `.vcxproj`의 파일 목록(`ClInclude` / `ClCompile`)과 `.vcxproj.filters`가 폴더 구조대로 다시 만들어진다.

- **준비물**: .NET 10 이상의 SDK. Visual Studio의 C++ 워크로드만으로는 설치되지 않는다. 설치 관리자에서 ".NET 데스크톱 개발" 워크로드를 추가하거나 SDK를 따로 설치한다.
- **스크립트**: 본체는 `Engine/Build/BatchFiles/GenerateProjectFiles.cs`이고, bat이 `dotnet run`으로 실행한다. 빌드 결과는 `Intermediate/DotNET/`에 생긴다.
- **대상**: 저장소 안의 모든 `.vcxproj`(`Binaries`, `Intermediate`, `ThirdParty` 제외). `.vcxproj`가 있는 폴더 아래의 `.h` / `.cpp`를 모은다. 필터 이름은 `.vcxproj` 기준 상대 폴더 경로다.
- **`.vcxproj`의 소스 목록은 직접 고치지 않는다**. 다음 실행 때 폴더 내용으로 덮어쓴다. VS의 "새 항목 추가"로 만든 파일도 실행하면 필터가 폴더에 맞춰진다. 목록 밖의 설정은 그대로 둔다.
- **파일별 설정은 쓸 수 없다**: 소스 목록 `ItemGroup`에 파일별 설정(메타데이터), `Condition`, 다른 항목이 있으면 오류를 내고 아무 파일도 바꾸지 않는다.
- 내용이 바뀐 파일만 쓴다. 바뀌지 않았으면 열려 있는 VS가 다시 로드를 묻지 않는다.
- 출력은 BOM 없는 UTF-8, CRLF다. `.bat`은 첫 줄들에서 코드페이지를 UTF-8(65001)로 바꾼 뒤 한국어를 쓴다(cmd는 배치 파일을 현재 코드페이지로 한 줄씩 읽는다).

## 공통 설정 (`Directory.Build.props` / `.targets`)

저장소 루트의 `Directory.Build.props`는 MSBuild가 모든 `.vcxproj`에 자동으로 import한다. 새 프로젝트도 따로 설정하지 않아도 공통 설정이 적용된다.

| 파일 | 담는 설정 |
|---|---|
| `Directory.Build.props` | 루트 경로(`SERootDir`), 출력 / 중간 디렉터리, 아래 "컴파일러 설정" 전부 |
| 각 `.vcxproj` | 프로젝트 종류, include 경로, 프로젝트 전용 전처리기 정의(`_LIB`, `_WINDOWS`), 링커 / 매니페스트, 프로젝트 참조 |

| 설정 | 값 |
|---|---|
| 출력 디렉터리 (`OutDir`) | `$(SERootDir)Binaries\$(Platform)\$(Configuration)\` |
| 중간 디렉터리 (`IntDir`) | `$(SERootDir)Intermediate\$(MSBuildProjectName)\$(Platform)\$(Configuration)\` |

- 모든 프로젝트의 `.exe`, `.lib`, DLL은 `Binaries`의 같은 폴더에 모은다. 중간 파일은 프로젝트별로 나눈다.
- 공통 설정은 이 파일을 직접 고친다. VS 속성 페이지에서 바꾼 값은 해당 `.vcxproj`에만 저장된다. 속성 페이지에서는 이 파일에서 온 값이 굵지 않은 글씨로 보인다.
- 이 파일은 프로젝트 앞부분에서 import되므로 `$(ProjectName)`처럼 프로젝트가 정의하는 속성을 쓸 수 없다. `$(MSBuildProjectName)`처럼 MSBuild가 미리 정의하는 속성을 쓴다.
- MSBuild는 C# 프로젝트(`dotnet run`으로 실행하는 `.cs` 도구 포함)에도 이 파일을 import한다. 속성(`PropertyGroup`)은 `.vcxproj` 조건을 붙여 C++ 프로젝트에만 적용한다. C#도 쓰는 이름(`OutDir` 등)이 새어 들어가지 않게 하기 위해서다.
- `.vcxproj`에서 목록형 설정(전처리기 정의, include 경로, 추가 옵션)을 넣을 때는 `%(PreprocessorDefinitions)`처럼 기존 값을 이어 붙인다. 빠뜨리면 공통 값이 사라진다.
- 저장소 루트의 `Directory.Build.targets`는 MSBuild가 모든 `.vcxproj`의 **마지막**에 import한다. 프로젝트 본문에서 정해지는 값(`Platform`, `UseDebugLibraries` 등)이 필요한 설정(vcpkg 연결)을 둔다. 이 파일도 `.vcxproj` 조건을 붙인다.

## vcpkg

외부 라이브러리는 vcpkg manifest 모드로 받는다. 목록은 `Engine/ThirdParty/vcpkg.json`이고, 빌드할 때 자동으로 설치된다.

| 설정 | 위치 | 값 |
|---|---|---|
| 사용자 전역 통합 끊기 | `Directory.Build.props` | `VCPkgLocalAppDataDisabled` = `true`. 개발 PC에 `vcpkg integrate install`이 되어 있어도 그 설정(classic 창고)을 쓰지 않는다 |
| manifest 모드 | `Directory.Build.props` | `VcpkgEnableManifest` = `true`, `VcpkgManifestRoot` = `Engine\ThirdParty\` |
| 설치 폴더 | `Directory.Build.props` | `VcpkgManifestInstalledBaseDir` = `Intermediate\vcpkg_installed\` (git 무시) |
| triplet | `Directory.Build.props` | `VcpkgUseStatic` + `VcpkgUseMD` = `true` → `x64-windows-static-md` (라이브러리는 정적, CRT는 엔진과 같은 `/MD`) |
| vcpkg 위치와 연결 | `Directory.Build.targets` | `VCPKG_ROOT` 환경 변수가 있으면 그 vcpkg, 없으면 Visual Studio 내장 vcpkg(`$(VsInstallRoot)\VC\vcpkg\`)의 `vcpkg.targets`를 import한다. 찾지 못하면 빌드 오류로 안내한다 |

**설치 목록** (`builtin-baseline` `eb2d3a32...`, vcpkg 2026-07-27로 확인)

| 패키지 | 버전 | 이유 | 설치되는 것 |
|---|---|---|---|
| `directxmath` | 3.21 (2026-06-12) | 엔진 수학 타입의 내부 계산(Architecture 6장) | 헤더 |

- **DirectXMath 출처**: vcpkg판을 쓴다. vcpkg include 경로가 Windows SDK 경로보다 먼저 검색되어 `#include <DirectXMath.h>`는 SDK판(3.19)이 아니라 vcpkg판을 가리킨다. 목록에 적어 버전을 baseline으로 고정한다.
- **새 PC 준비**: Visual Studio Installer에서 "vcpkg 패키지 관리자" 구성 요소를 설치한다. 다른 vcpkg를 쓰려면 `VCPKG_ROOT` 환경 변수를 지정한다.
- **첫 빌드**: 목록의 라이브러리를 내려받아 컴파일하므로 인터넷이 필요하고 오래 걸릴 수 있다. 이후에는 캐시를 쓴다.
- **설치 위치 확인**: 빌드 출력(상세도 "보통" 이상)에 `Using triplet "x64-windows-static-md" from "...\Intermediate\vcpkg_installed\..."`가 나온다.
- vcpkg는 설치 폴더의 `lib\*.lib`를 전부 자동으로 링크한다(전이 의존성 포함).
- vcpkg 명령을 직접 실행할 때는 `Engine/ThirdParty/`에서 실행하거나 `--x-manifest-root`로 위치를 넘긴다.

## 플랫폼 / 빌드 구성

- 플랫폼: x64만 둔다 (`SeonEngine.slnx`, 모든 `.vcxproj`).
- 구성: Debug(개발용), Release(배포용).

## 컴파일러 설정

모든 프로젝트, 모든 구성에 적용한다 (`Directory.Build.props`). 위치 열은 VS 속성 페이지(C/C++)에서 보이는 위치다.

| 설정 | 값 | 위치 |
|---|---|---|
| 경고 수준 | `/W4` | 일반 → 경고 수준 |
| 경고를 오류로 처리 | 예 (`/WX`) | 일반 → 경고를 오류로 처리 |
| 추가 경고 | `/w14668 /w14265` | 명령줄 → 추가 옵션 |
| 외부 헤더 | `<>` include를 외부 헤더로 취급, 경고 끔 | 외부 포함 → 괄호로 묶인 포함을 외부로 처리 / 외부 헤더 경고 수준 |
| C++ 예외 | 끔 (`_HAS_EXCEPTIONS=0` 정의) | 코드 생성 → C++ 예외 처리 가능 |
| RTTI | 끔 (`/GR-`) | 언어 → 런타임 형식 정보 사용 |
| 전처리기 정의 | `SE_PLATFORM_WINDOWS`, `SE_BUILD_DEBUG`, `SE_BUILD_RELEASE` (구성별 0 / 1), `SE_PLATFORM_HEADER_NAME=Windows`(플랫폼 헤더 경로를 만드는 이름, Architecture 3장 "플랫폼 헤더 고르기") | 전처리기 → 전처리기 정의 |
