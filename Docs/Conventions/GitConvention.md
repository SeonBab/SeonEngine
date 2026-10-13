# SeonEngine Git Convention

## 관련 문서

- [Code Convention](CodeConvention.md)
- [Project Settings](../ProjectSettings.md)

---

## 1. 브랜치

- 기본은 `main`에서 직접 작업하고, 검증한 변경을 커밋하는 방식이다.
- `main`에 기록된 모든 커밋은 Debug / Release 빌드가 되는 상태로 유지한다. 작업 중인 미커밋 변경까지 항상 완성되어 있어야 한다는 뜻은 아니다.
- 기존 작업과 별도로 실험하거나 여러 작업을 동시에 진행할 때만 작업 브랜치를 사용한다. 작업 브랜치를 사용했다면 3.3의 순서로 합친 뒤 삭제한다.

---

## 2. 커밋

### 2.1 메시지 형식

[Conventional Commits](https://www.conventionalcommits.org/) 형식을 따른다. 타입과 범위는 영어로, 제목과 본문은 한국어로 쓴다.

```text
타입(범위): 제목

본문 (선택)
```

- **제목** — 무엇을 했는지 한 줄로 쓴다. 명사형으로 끝내고(`추가`, `수정`, `분리`) 마침표를 찍지 않는다.
- **본문** — 제목만으로 부족할 때 쓴다. 무엇을 했는지보다 **왜** 했는지를 쓴다(Code Convention 1.3 Comment와 같은 원칙). 제목과 본문 사이에 빈 줄을 둔다.
- **푸터** — 쓰지 않는다(`Co-Authored-By`, `Signed-off-by` 등).

```text
feat(core): 로그 카테고리 추가
fix(renderer): 창 크기가 0일 때 스왑체인 재생성 실패 수정
refactor(platform): Windows 헤더 래퍼 분리
docs: Git 컨벤션 작성
build: 경고 레벨을 /W4로 변경
```

### 2.2 타입

| 타입 | 용도 |
|---|---|
| `feat` | 기능 추가 |
| `fix` | 버그 수정 |
| `refactor` | 동작 변경 없는 구조 개선 |
| `perf` | 성능 개선 |
| `test` | 테스트 추가 / 수정 |
| `docs` | 문서 |
| `build` | 빌드 설정, 프로젝트 파일(`.vcxproj`, `.clang-format` 등) |
| `ci` | CI 설정 |
| `style` | 서식만 변경 (컨벤션 변경 후 일괄 수정 등, Code Convention 6.3 예외 조항) |
| `chore` | 그 외 (`.gitignore` 등) |

### 2.3 범위

- 바뀐 모듈이나 폴더 이름을 소문자로 쓴다(`core`, `engine`, `game`, `platform`, `renderer` 등). 모듈 구성과 경로는 [Project Settings의 폴더 구조](../ProjectSettings.md#폴더-구조)를 참고한다.
- 여러 모듈에 걸친 변경이나 `docs`, `build`, `chore`처럼 모듈과 관계없는 변경은 범위를 생략한다.

### 2.4 커밋 단위

- 커밋 하나에는 논리적인 변경 하나만 담는다.
- 모든 커밋은 그 커밋만으로 빌드가 되어야 한다. 후속 미커밋 변경에 의존하는 커밋은 만들지 않는다. 각 커밋이 빌드 가능해야 `git bisect`로 문제 지점을 검사할 수 있다.
- `main`에는 미완성 작업을 "wip" 커밋으로 남기지 않는다. 작업 브랜치에서 만든 임시 커밋은 `main`에 합치기 전에 논리적인 변경 단위로 정리한다. 이미 공유한 커밋의 수정은 후속 커밋으로 남긴다.
- 서식 일괄 수정(`style`)은 기능 변경과 섞지 않는다.

---

## 3. 작업 순서

### 3.1 main에서 직접 커밋

`main`에서 변경을 구현하고, 3.2의 확인을 마친 뒤 해당 변경만 스테이징해 커밋한다. 커밋은 논리적인 변경 단위로 나눈다.

```bash
git status
git diff
git add <이번 커밋에 포함할 파일>
git diff --cached
git diff --cached --check
git commit
```

- 작업을 시작할 때 현재 브랜치와 미커밋 변경을 확인한다. 기존 작업 브랜치에 변경이 남아 있다면 먼저 상태를 확인해 통합 방법을 정한다.
- 커밋 요청만으로 push하지 않는다. push는 별도로 요청받았을 때 수행한다.
- 이미 push한 `main`의 커밋은 rebase / reset으로 고치지 않고 revert 커밋으로 되돌린다. 기록을 고치기 전에 원격에 올라갔는지 확인한다.

### 3.2 커밋 전 확인

- 변경 내용과 스테이징한 내용(`git diff`, `git diff --cached`)을 훑어본다. 디버그 코드나 관계없는 파일이 섞이지 않았는지 확인한다.
- 코드나 빌드 설정을 변경했다면 Debug / Release 빌드가 경고 없이 성공한다(Code Convention 6.1). 문서만 바꾼 경우에는 내용과 링크를 확인한다.
- 바뀐 C++ 파일에 clang-format을 적용했다(Code Convention 6.2).
- 컨벤션이나 설정을 바꿨다면 관련 문서도 함께 고쳤다.

### 3.3 작업 브랜치를 사용한 경우

작업 브랜치를 `main` 최신 상태 위로 rebase한 뒤 fast-forward로 합친다. `main`의 기록은 병합 커밋 없이 한 줄로 이어진다.

```bash
git switch log-category
git rebase main
git switch main
git merge --ff-only log-category
git branch -d log-category
```

- 합치기 전에 작업 브랜치의 변경 전체(`git diff main...`)와 각 커밋을 확인하고 3.2의 검증을 수행한다.
- 원격에 공유한 작업 브랜치의 기록을 rebase로 바꾸거나 원격 브랜치를 삭제할 때는 별도로 요청받은 범위를 확인한다. 기록을 바꾼 브랜치의 push에는 `--force-with-lease`를 사용한다(`--force`는 쓰지 않는다).
- `--ff-only`가 실패하면 `main`이 앞서 나간 것이다. 다시 rebase한 뒤 합친다.
