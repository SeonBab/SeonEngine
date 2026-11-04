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
3. **빌드 → 솔루션 빌드**를 실행합니다.

## 도구 집합 선택과 출력 경로

MSBuild가 자동으로 읽는 [Directory.Build.props](../Directory.Build.props)에서 `VisualStudioVersion`에 따라 `SEPlatformToolset`을 선택합니다. [SeonEngine.vcxproj](../SeonEngine.vcxproj)의 Debug·Release는 `PlatformToolset`에 `$(SEPlatformToolset)`을 사용합니다. [MSBuild 공통 설정 안내](https://learn.microsoft.com/en-us/visualstudio/msbuild/customize-by-directory)

| 빌드 환경 | VisualStudioVersion | 도구 집합 |
| --- | --- | --- |
| VS 2022 | `17.0` | `v143` |
| VS 2026 | `18.0` | `v145` |

저장소 루트를 기준으로 출력·중간 파일을 도구 집합별로 분리합니다.

| 속성 | 경로 |
| --- | --- |
| `OutDir` | `Binaries/<도구 집합>/<플랫폼>/<구성>/` |
| `IntDir` | `Intermediate/<도구 집합>/<프로젝트 이름>/<플랫폼>/<구성>/` |

## 설정 확인

확인할 VS 버전의 **Developer PowerShell** 또는 **Developer Command Prompt**를 열고, 저장소 루트에서 실행합니다.

```powershell
msbuild SeonEngine.vcxproj /nologo /p:Configuration=Debug /p:Platform=x64 /getProperty:VisualStudioVersion,SEPlatformToolset,PlatformToolset,OutDir,IntDir
```

`PlatformToolset`이 VS 2022에서는 `v143`, VS 2026에서는 `v145`이고 출력 경로에도 같은 도구 집합이 포함되는지 확인합니다. `Configuration=Release`로 바꾸면 Release 설정을 조회합니다. 이 명령은 설정만 조회하며 컴파일·실행하지 않습니다.
