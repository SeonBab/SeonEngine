# 빌드·개발 설정

이 문서는 저장소를 받아 프로젝트를 생성·빌드하고 소스를 편집하는 데 필요한 현재 설정을 다룬다. 설정값의 기준은 아래에 명시한 실제 파일이다.

## 개발 환경과 빌드

- Windows x64, Visual Studio 2026의 C++를 사용한 데스크톱 개발 워크로드와 MSVC v145, Windows SDK 10.0 계열을 사용한다.
- 프로젝트 파일 생성에는 .NET 10 이상 SDK가 필요하다. 런타임만 설치한 환경은 사용할 수 없다.
- Visual Studio Installer의 vcpkg 패키지 관리자 구성 요소를 설치하거나 `VCPKG_ROOT`에 별도 vcpkg 경로를 지정한다.

저장소 루트에서 프로젝트 파일 목록을 생성한 뒤 `SeonEngine.slnx`를 연다. 구성은 Debug와 Release, 플랫폼은 x64다. 솔루션에는 SampleGame 실행 프로젝트 하나가 있으며 엔진 소스도 함께 컴파일한다.

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
├─ SampleGame/Source/          게임 소스와 SampleGame.vcxproj
├─ Docs/                      개발 규칙·빌드 안내
├─ Binaries/                  빌드 결과（Git 제외）
└─ Intermediate/              중간 결과·vcpkg 설치 파일（Git 제외）
```

현재 별도의 Engine.vcxproj나 Engine.lib는 만들지 않는다. 엔진과 게임의 책임은 폴더로 구분하며 include 검색 기준은 `Engine/Source`다. 엔진 모듈마다 Public/Private 폴더를 두는 구성은 사용하지 않는다.

## 설정 파일과 출력 경로

| 파일 | 관리하는 설정 |
|---|---|
| `Directory.Build.props` | `SERootDir`, 출력·중간 디렉터리, 공통 컴파일러 옵션, vcpkg manifest 설정 |
| `Directory.Build.targets` | 프로젝트 본문 이후의 vcpkg targets import와 경로 검사 |
| `SampleGame/Source/SampleGame.vcxproj` | 실행 프로젝트, v145, SDK, include·링크, 구성별 CRT·최적화, HLSL 컴파일·콘텐츠 복사 |
| `SeonEngine.slnx` | 솔루션 프로젝트와 플랫폼 |

- 출력: `$(SERootDir)Binaries\$(Platform)\$(Configuration)\`
- 중간 결과: `$(SERootDir)Intermediate\$(MSBuildProjectName)\$(Platform)\$(Configuration)\`
- vcpkg 설치: `$(SERootDir)Intermediate\vcpkg_installed\`

다른 폴더를 참조할 때는 저장소 루트인 `$(SERootDir)`을 쓴다. 공통 옵션은 props/targets에서 수정하며 VS 속성 페이지에서 바꾸면 해당 프로젝트에 저장될 수 있다. 목록형 설정은 `%(PreprocessorDefinitions)`, `%(AdditionalIncludeDirectories)`, `%(AdditionalOptions)`처럼 기존 값을 이어 붙인다.

## 프로젝트 파일 생성

소스를 추가·삭제·이동한 뒤 `GenerateProjectFiles.bat`을 실행한다. 본체는 `Engine/Build/BatchFiles/GenerateProjectFiles.cs`이며 도구 빌드 결과는 `Intermediate/DotNET/GenerateProjectFiles`에 둔다.

- 저장소의 `.vcxproj`를 찾되 Binaries·Intermediate·ThirdParty 경로는 제외한다.
- 기본 검색 범위는 프로젝트 폴더이며 Source 프로젝트의 형제 Shaders 폴더도 포함한다. 현재 SampleGame에는 `Engine/Source`와 `Engine/Shaders`를 추가로 포함한다.
- `.h`는 ClInclude, `.cpp`는 ClCompile, 명시적으로 컴파일하지 않는 `.hlsl`·`.hlsli`는 편집용 None 목록에 넣는다. `.vcxproj.filters`는 폴더 구조에 맞춰 생성한다.
- 생성 대상 소스 목록과 필터는 생성기를 통해 관리한다. 자동 목록의 파일별 메타데이터·Condition·다른 종류 항목은 지원하지 않으며 발견하면 쓰기 전에 오류로 중단한다.
- 셰이더 FxCompile 전용 목록과 컴파일 메타데이터는 프로젝트에서 직접 관리한다. 검색 범위 안의 단일 `.hlsl` 상대 경로만 허용하며 중복·변수·와일드카드는 거부한다. 생성기는 FxCompile을 보존하고 None 중복을 제거한다.
- 내용이 바뀐 파일만 BOM 없는 UTF-8·CRLF로 쓴다. 필터 이름은 프로젝트 상대 경로의 앞쪽 `..`를 제거해 표시한다.

## 컴파일러 설정

공통 설정의 기준은 `Directory.Build.props`다.

| 항목 | 값 |
|---|---|
| 언어·전처리기 | C++20, 표준 준수 모드, `/Zc:preprocessor` |
| 소스·실행 문자 집합 | `/utf-8` |
| 경고 | `/W4`, `/WX`, `/w14668`, `/w14265` |
| 외부 헤더 | 꺾쇠 include를 외부 헤더로 취급하고 외부 경고 끔 |
| C++ 예외 | 끔, `_HAS_EXCEPTIONS=0` |
| RTTI·SDL 검사 | 켬 |
| 플랫폼 정의 | `SE_PLATFORM_WINDOWS=1`, `SE_PLATFORM_HEADER_NAME=Windows` |
| Debug 정의 | `SE_BUILD_DEBUG=1`, `SE_BUILD_RELEASE=0`, `_DEBUG` |
| Release 정의 | `SE_BUILD_DEBUG=0`, `SE_BUILD_RELEASE=1`, `NDEBUG` |

SampleGame 프로젝트는 PCH를 사용하지 않으며 `_CONSOLE`, Console 서브시스템, Unicode 문자 집합을 지정한다. Debug는 최적화 끔·`/MDd`, Release는 속도 최적화·`/MD`를 사용한다. 두 구성 모두 PDB 정보를 생성하며 Release는 함수 단위 링크·내장 함수·참조 제거·COMDAT 접기를 사용한다.

현재 직접 링크하는 Windows 라이브러리는 `dxgi.lib`, `d3d11.lib`, `user32.lib`, `windowscodecs.lib`, `ole32.lib`다.

## 외부 라이브러리

`Engine/ThirdParty/vcpkg.json`의 manifest 모드를 사용한다. 현재 manifest 목록은 DirectXMath, Assimp, ImGui이며 ImGui에는 docking-experimental·win32-binding·dx11-binding 기능을 지정한다. manifest 등록은 제품 코드에서 해당 기능을 사용한다는 뜻은 아니다. 버전 기준은 manifest의 `builtin-baseline`이다.

- 사용자 전역 classic 통합은 `VCPkgLocalAppDataDisabled=true`로 끈다.
- `VcpkgEnableManifest=true`, manifest 루트는 `Engine/ThirdParty/`다.
- `VcpkgUseStatic=true`와 `VcpkgUseMD=true`로 `x64-windows-static-md` 구성을 사용한다.
- vcpkg 연결은 `VCPKG_ROOT`를 우선하고, 없으면 Visual Studio 내장 `VC/vcpkg`를 사용한다. targets를 찾지 못하면 빌드 오류로 안내한다.
- vcpkg 명령을 직접 실행할 때는 manifest 폴더에서 실행하거나 `--x-manifest-root`로 위치를 지정한다.

배포 시의 고지 파일은 [Assimp와 의존 라이브러리 고지](../Engine/ThirdParty/Assimp/README.md)를 따른다.

## 셰이더와 콘텐츠 출력

`SampleGame.vcxproj`의 FxCompile은 `TriangleVertexShader.hlsl`과 `TrianglePixelShader.hlsl`을 각각 Vertex·Pixel, Shader Model 5.0, 진입점 Main으로 컴파일한다. 경고는 오류로 처리하며 출력은 `$(OutDir)Shaders/`의 같은 이름 `.cso`다. 프로젝트가 컴파일 전 해당 디렉터리를 만든다.

빌드 후 `Engine/Content/Textures/Checker.png`를 `$(OutDir)Content/Textures/`에 복사한다. 런타임 셰이더를 준비할 때는 실행 파일과 Shaders 출력을 함께 둔다.

## 소스 인코딩과 서식

- `.editorconfig`: UTF-8 BOM 없음, CRLF, 파일 끝 newline. C++·HLSL·C#의 들여쓰기는 Tab, 폭 4다.
- `.clang-format`: 공통 코드 서식. 엔진 코드는 [코드 컨벤션](Conventions/CodeConvention.md)을 따른다. `Engine/ThirdParty/.clang-format`은 서드파티 서식·include 정렬을 끈다.
- 루트 `.clang-format`은 `Language`를 지정하지 않은 공통 기본 설정이며 `Standard: c++20`을 사용한다.
- `WhitespaceSensitiveMacros`에는 기본 다섯 이름과 `SE_STRINGIZE`를 함께 유지한다. 옵션은 YAML 끝 표시 `...` 앞에 두며 도구 버전을 바꾸면 `clang-format --dump-config`로 적용값을 확인한다.
- `.gitattributes`: 일반 텍스트는 저장소 LF·작업 폴더 CRLF, `.bat`·`.cmd`는 CRLF, `.sh`는 LF다. 바이너리는 변환하지 않는다. `Engine/ThirdParty/Assimp/Licenses/*.txt`는 `-text`로 원문 바이트를 유지한다.
