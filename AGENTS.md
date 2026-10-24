# AGENTS.md

## 프로젝트

SeonEngine: C++ 자체 게임 엔진 (Windows, D3D11, C++20, Visual Studio)

## 규칙 문서

코드를 작성하기 전에 작업과 관련된 절을 확인한다. 이어지는 작업에서는 이미 읽은 규칙을 재사용하되 변경·충돌 여부를 확인한다.

- 코드 컨벤션: [Docs/Conventions/CodeConvention.md](Docs/Conventions/CodeConvention.md)
- Git 컨벤션: [Docs/Conventions/GitConvention.md](Docs/Conventions/GitConvention.md)
- 빌드·개발 설정: [Docs/BuildSetup.md](Docs/BuildSetup.md)

## 작업 규칙

- 서식은 저장소 루트의 `.clang-format`을 따른다.
- 컨벤션이나 설정을 바꾸면 위 문서도 함께 고친다.
- 컨벤션의 현재 규칙과 공개 API의 실제 계약을 구분한다. 규칙을 정리했다는 이유로 승인받지 않은 기존 코드까지 일괄 변경하지 않는다.
- 이름·상속·호출 순서를 바꾸는 제안은 선언·정의·호출부·계약 주석을 함께 확인해 필요한 변경 범위를 빠뜨리지 않는다.
