# Project Settings

프로젝트에 적용해 둔 설정이다.

## 언어 표준

모든 구성에서 C++20(`/std:c++20`)을 쓴다. (`.vcxproj` → C/C++ → 언어 → C++ 언어 표준)

## 소스 인코딩

소스 파일은 UTF-8 (BOM 없음)으로 저장한다.

| 설정 | 위치 | 이유 |
|---|---|---|
| `/utf-8` | `.vcxproj` → C/C++ → 명령줄 → 추가 옵션 (모든 구성) | MSVC는 BOM 없는 파일을 시스템 코드페이지(CP949)로 읽기 때문에 한글 주석과 문자열이 깨진다. |
| `charset = utf-8` | `.editorconfig` | Visual Studio가 파일을 BOM 없는 UTF-8로 저장하게 한다. |

## `.editorconfig`

저장소 루트의 `.editorconfig`로 에디터 설정을 통일한다.

- 인코딩: UTF-8 (BOM 없음)
- 줄바꿈: CRLF
- 파일 끝 newline
- C++ / HLSL 파일 들여쓰기: Tab, 폭 4

## `.clang-format`

저장소 루트의 `.clang-format`으로 코드 포매팅을 통일한다(Code Convention 1.1 Formatting). Visual Studio는 내장 clang-format으로 이 파일을 읽는다(도구 → 옵션 → 텍스트 편집기 → C/C++ → 코드 스타일 → 서식에서 clang-format 지원이 켜져 있어야 한다).

`ThirdParty/.clang-format`은 서드파티 폴더의 서식 정리와 include 정렬을 끈다(Code Convention 6.3 예외 조항).

## `.gitattributes`

줄바꿈을 각자의 `core.autocrlf` 설정과 관계없이 고정한다.

- 텍스트 파일: 저장소에는 LF, 작업 폴더에는 CRLF
- 이미지, 오디오, 폰트, 바이너리 파일: 변환하지 않음 (`binary`)
- 예외: `.bat` / `.cmd`는 CRLF, `.sh`는 LF로 고정

## 플랫폼 / 빌드 구성

- 플랫폼: x64만 둔다 (`Engine.slnx`, `Engine.vcxproj`).
- 구성: Debug(개발용), Release(배포용).

## 컴파일러 설정

모든 구성에 적용한다 (`.vcxproj` → C/C++).

| 설정 | 값 | 위치 |
|---|---|---|
| 경고 수준 | `/W4` | 일반 → 경고 수준 |
| 경고를 오류로 처리 | 예 (`/WX`) | 일반 → 경고를 오류로 처리 |
| 추가 경고 | `/w14668 /w14265` | 명령줄 → 추가 옵션 |
| 외부 헤더 | `<>` include를 외부 헤더로 취급, 경고 끔 | 외부 포함 → 괄호로 묶인 포함을 외부로 처리 / 외부 헤더 경고 수준 |
| C++ 예외 | 끔 (`_HAS_EXCEPTIONS=0` 정의) | 코드 생성 → C++ 예외 처리 가능 |
| RTTI | 끔 (`/GR-`) | 언어 → 런타임 형식 정보 사용 |
| 전처리기 정의 | `SE_PLATFORM_WINDOWS`, `SE_BUILD_DEBUG`, `SE_BUILD_RELEASE` (구성별 0 / 1) | 전처리기 → 전처리기 정의 |
