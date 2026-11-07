# 빌드 및 개발 환경

## 필요한 도구

| 항목 | 설치 요건 |
| --- | --- |
| 대상 플랫폼 | Windows x64 |
| IDE | Visual Studio 2022 17.14 이상 또는 Visual Studio 2026 |
| 워크로드 | C++를 사용한 데스크톱 개발 |
| C++ 도구 집합 | VS 2022는 MSVC v143, VS 2026은 MSVC v145 |
| Windows SDK | Windows 10/11 SDK |

Visual Studio Installer에서 워크로드와 해당 구성 요소를 설치합니다. VS 2022의 최소 버전은 현재 솔루션 형식인 `.slnx`의 [정식 지원 기준](https://devblogs.microsoft.com/dotnet/introducing-slnx-support-dotnet-cli/)에 따른 것입니다.

## 빌드 방법

1. Visual Studio에서 저장소 루트의 `SeonEngine.slnx`를 엽니다.
2. 구성을 `Debug` 또는 `Release`, 플랫폼을 `x64`로 선택합니다.
3. **빌드 -> 솔루션 빌드**를 실행합니다.

## 소스와 헤더 경로

프로세스 진입점은 `Game/Source/GameMain.cpp`에 둡니다. `Engine/Source`에는 게임에서 사용하는 창 처리와 시간 측정 코드를 둡니다. 현재는 하나의 실행 프로젝트에서 두 경로의 소스를 함께 빌드합니다.

Debug, Release의 추가 포함 디렉터리에 `$(ProjectDir)Engine\Source`를 지정합니다. 엔진 공용 수학 코드는 `Engine/Source/Core/Math`에 두고, 다른 폴더에서 사용할 때는 `#include "Core/Math/Vector3.h"`처럼 소스 루트 기준 경로로 포함합니다.

## 외부 라이브러리

외부 라이브러리는 `Engine/ThirdParty/<라이브러리>/`에 원본 그대로 두고, 각 폴더의 README에 출처, 버전, 받은 파일의 해시와 포함한 범위를 적습니다. 별도 설치는 필요하지 않습니다.

| 라이브러리 | 용도 | 버전 | 추가 포함 디렉터리 |
| --- | --- | --- | --- |
| RapidJSON | JSON 읽기 | 커밋 `24b5e7a8` ([README](../Engine/ThirdParty/RapidJSON/README.md)) | `$(ProjectDir)Engine\ThirdParty\RapidJSON\include` |

헤더 전용 라이브러리이므로 링크 설정은 없습니다. 소스에서는 `#include "rapidjson/document.h"`처럼 포함합니다.

## 도구 집합 선택과 출력 경로

MSBuild가 자동으로 읽는 [Directory.Build.props](../Directory.Build.props)에서 `VisualStudioVersion`에 따라 `SEPlatformToolset`을 선택합니다. [SeonEngine.vcxproj](../SeonEngine.vcxproj)의 Debug, Release는 `PlatformToolset`에 `$(SEPlatformToolset)`을 사용합니다. [MSBuild 공통 설정 안내](https://learn.microsoft.com/en-us/visualstudio/msbuild/customize-by-directory)

| 빌드 환경 | VisualStudioVersion | 도구 집합 |
| --- | --- | --- |
| VS 2022 | `17.0` | `v143` |
| VS 2026 | `18.0` | `v145` |

저장소 루트를 기준으로 출력, 중간 파일을 도구 집합별로 분리합니다.

| 속성 | 경로 |
| --- | --- |
| `OutDir` | `Binaries/<도구 집합>/<플랫폼>/<구성>/` |
| `IntDir` | `Intermediate/<도구 집합>/<프로젝트 이름>/<플랫폼>/<구성>/` |

두 폴더는 빌드 결과물이므로 `.gitignore`로 버전 관리에서 제외합니다.

## 자산 폴더

게임 자산은 `Game/Content/`, 엔진 기본 자산은 `Engine/Content/`에 둡니다. 코드에서는 `Game/Content/Meshes/Box.gltf`처럼 저장소 루트 기준 상대 경로로 가리키며, `Engine/Source/Paths.h`의 `Paths::GetAbsolutePath`가 이를 절대 경로로 바꿉니다.

자산은 출력 폴더로 복사하지 않습니다. `Paths::GetRootDirectory`는 실행 파일이 위 `OutDir` 배치(`Binaries` 아래 세 단계)에 있다는 전제로 저장소 루트를 구하므로, 실행 파일만 다른 위치로 옮기면 자산을 찾지 못합니다. `OutDir`의 깊이를 바꾸면 이 함수도 함께 고칩니다.

## 소스 인코딩과 편집기 설정

저장소 루트의 [.editorconfig](../.editorconfig)는 UTF-8(BOM 없음), CRLF와 파일 끝 줄바꿈을 지정합니다. C++ 헤더와 소스는 탭으로 들여쓰고 탭 표시 폭은 4칸입니다. 이 파일을 지원하는 편집기를 사용합니다. 기존 파일의 인코딩은 자동으로 일괄 변환되지 않으므로 CP949 파일을 가져올 때는 원래 인코딩으로 읽은 뒤 UTF-8로 변환합니다.

[Directory.Build.props](../Directory.Build.props)의 공통 C++ 컴파일 옵션 `/utf-8`은 Debug와 Release의 소스 및 실행 문자 집합을 UTF-8로 지정합니다. 프로젝트의 `CharacterSet=Unicode`는 Windows API의 문자 형식 선택에 사용되며 소스 파일 인코딩 설정과 별개입니다. Win32의 `W` 계열 API에 전달하는 wide 문자열은 계속 UTF-16을 사용합니다.

## 설정 확인

확인할 VS 버전의 **Developer PowerShell** 또는 **Developer Command Prompt**를 열고, 저장소 루트에서 실행합니다.

```powershell
msbuild SeonEngine.vcxproj /nologo /p:Configuration=Debug /p:Platform=x64 /getProperty:VisualStudioVersion,SEPlatformToolset,PlatformToolset,OutDir,IntDir
```

`PlatformToolset`이 VS 2022에서는 `v143`, VS 2026에서는 `v145`이고 출력 경로에도 같은 도구 집합이 포함되는지 확인합니다. `Configuration=Release`로 바꾸면 Release 설정을 조회합니다. 이 명령은 설정만 조회하며 컴파일, 실행하지 않습니다.
