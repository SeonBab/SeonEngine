# SeonEngine

## 개발 환경

- Windows x64
- Visual Studio 2022 17.14 이상 또는 Visual Studio 2026 — C++를 사용한 데스크톱 개발 워크로드
- C++20 / MSVC 플랫폼 도구 집합: VS 2022는 v143, VS 2026은 v145
- .NET 10 이상 SDK — 프로젝트 파일 생성 스크립트 실행에 필요

`SeonEngine.slnx`를 열면 루트 `SeonEngine.vcxproj` 실행 프로젝트를 사용한다. 엔진 소스는 `Engine/Source`, 게임 소스는 `Game/Source`에 둔다.

상세 설정은 [빌드·개발 설정](Docs/BuildSetup.md)을 참고한다.

외부 라이브러리 사용·배포 정보: [Assimp와 의존 라이브러리 고지](Engine/ThirdParty/Assimp/README.md).
