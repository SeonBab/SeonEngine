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
├─ Game/Source/                게임 소스
├─ Docs/                      개발 규칙·빌드 안내
├─ Binaries/                  빌드 결과（Git 제외）
└─ Intermediate/              중간 결과·vcpkg 설치 파일（Git 제외）
```

현재 별도의 Engine.vcxproj나 Engine.lib는 만들지 않는다. 엔진과 게임의 책임은 폴더로 구분하며 include 검색 기준은 `Engine/Source`다. 엔진 모듈마다 Public/Private 폴더를 두는 구성은 사용하지 않는다.

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

빌드 후 `Engine/Content/Textures/Checker.png`를 `$(OutDir)Content/Textures/`에 복사한다. 런타임 셰이더를 준비할 때는 실행 파일과 Shaders 출력을 함께 둔다.

## 소스 인코딩과 서식

- `.editorconfig`: UTF-8 BOM 없음, CRLF, 파일 끝 newline. C++·HLSL·C#의 들여쓰기는 Tab, 폭 4다.
- `.clang-format`: 공통 코드 서식. 엔진 코드는 [코드 컨벤션](Conventions/CodeConvention.md)을 따른다. `Engine/ThirdParty/.clang-format`은 서드파티 서식·include 정렬을 끈다.
- 루트 `.clang-format`은 `Language`를 지정하지 않은 공통 기본 설정이며 `Standard: c++20`을 사용한다.
- `WhitespaceSensitiveMacros`에는 기본 다섯 이름과 `SE_STRINGIZE`를 함께 유지한다. 옵션은 YAML 끝 표시 `...` 앞에 두며 도구 버전을 바꾸면 `clang-format --dump-config`로 적용값을 확인한다.
- `.gitattributes`: 일반 텍스트는 저장소 LF·작업 폴더 CRLF, `.bat`·`.cmd`는 CRLF, `.sh`는 LF다. 바이너리는 변환하지 않는다. `Engine/ThirdParty/Assimp/Licenses/*.txt`는 `-text`로 원문 바이트를 유지한다.
