# Project Settings

프로젝트에 적용해 둔 설정이다.

## 언어 표준

모든 프로젝트, 모든 구성에서 C++20(`/std:c++20`)을 쓴다. (`Directory.Build.props`, 속성 페이지: C/C++ → 언어 → C++ 언어 표준)

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
- C++ / HLSL 파일 들여쓰기: Tab, 폭 4

## `.clang-format`

저장소 루트의 `.clang-format`으로 코드 포매팅을 통일한다(Code Convention 1.1 Formatting). Visual Studio는 내장 clang-format으로 이 파일을 읽는다(도구 → 옵션 → 텍스트 편집기 → C/C++ → 코드 스타일 → 서식에서 clang-format 지원이 켜져 있어야 한다).

`Engine/ThirdParty/.clang-format`은 서드파티 폴더의 서식 정리와 include 정렬을 끈다(Code Convention 6.3 예외 조항).

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
├─ Directory.Build.props   모든 프로젝트 공통 설정
├─ Engine/
│  ├─ Source/
│  │  ├─ Engine.vcxproj
│  │  ├─ Core/            모듈 (Public/, Private/)
│  │  └─ Engine/          모듈
│  └─ ThirdParty/
├─ SampleGame/            게임 프로젝트
│  └─ Source/
│     ├─ SampleGame.vcxproj
│     └─ SampleGame/      모듈
├─ Docs/
├─ Binaries/              빌드 결과 (git 무시)
└─ Intermediate/          중간 파일 (git 무시)
```

- 엔진과 게임 프로젝트는 같은 구조를 갖는다. 셰이더(`Shaders/`), 에셋(`Content/`), 설정(`Config/`) 폴더는 처음 필요할 때 각 폴더 아래에 추가한다.
- 게임 프로젝트 이름은 `<이름>Game`으로 짓고, 게임 모듈 이름은 프로젝트 이름과 같게 한다.
- `Engine/`과 게임 폴더에는 빌드 결과를 두지 않는다.

## 프로젝트

| 프로젝트 | 종류 | 담는 모듈 | include 경로 |
|---|---|---|---|
| `Engine` | 정적 라이브러리 (`.lib`) | `Core`, `Engine` | 두 모듈의 `Public`, `Private` |
| `SampleGame` | 실행 파일 (`.exe`, Windows 서브시스템) | `SampleGame` | 엔진 모듈의 `Public`, 자기 모듈의 `Public`, `Private` |

- `SampleGame`은 `Engine`을 프로젝트 참조로 연결한다. 빌드 순서와 `Engine.lib` 링크가 자동으로 처리된다.
- 진입점(`WinMain`)은 `Engine.lib`에 있다(Architecture 3. 플랫폼 추상화). 게임 프로젝트는 엔진과 게임 코드를 실행 파일로 링크하는 단위다.
- 게임 프로젝트에는 `.cpp`가 하나 이상 있어야 한다. 컴파일된 `.obj`가 없으면 CRT가 링크되지 않아 시작 코드(`WinMainCRTStartup`)를 찾지 못한다.
- 게임 프로젝트의 include 경로에는 엔진 모듈의 `Public`만 넣는다. 엔진 내부 헤더를 include하면 빌드가 실패한다(Code Convention 2.1 파일 구성 참고).
- 프로젝트에서 다른 폴더를 가리킬 때는 `$(SERootDir)`(저장소 루트)을 쓴다. `$(SolutionDir)`은 솔루션 없이 프로젝트만 빌드하면 값이 달라진다.

## 공통 설정 (`Directory.Build.props`)

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
- `.vcxproj`에서 목록형 설정(전처리기 정의, include 경로, 추가 옵션)을 넣을 때는 `%(PreprocessorDefinitions)`처럼 기존 값을 이어 붙인다. 빠뜨리면 공통 값이 사라진다.

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
| 전처리기 정의 | `SE_PLATFORM_WINDOWS`, `SE_BUILD_DEBUG`, `SE_BUILD_RELEASE` (구성별 0 / 1) | 전처리기 → 전처리기 정의 |
