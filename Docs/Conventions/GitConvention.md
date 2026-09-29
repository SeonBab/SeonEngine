# SeonEngine Git Convention

## 관련 문서

- [Code Convention](CodeConvention.md)
- [Project Settings](../ProjectSettings.md)

---

## 1. 브랜치

- `main`은 항상 Debug / Release 빌드가 되는 상태로 유지한다.
- 모든 작업은 작업 브랜치에서 하고, 로컬에서 rebase한 뒤 `main`에 합친다(3장 병합). `main`에서 직접 작업하지 않는다.
- 작업 브랜치는 합친 뒤 삭제한다.

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

- 바뀐 모듈이나 폴더 이름을 소문자로 쓴다(`core`, `engine`, `game`, `platform`, `renderer` 등). 모듈 구성은 [Architecture](../Architecture.md) 1장을 따른다.
- 여러 모듈에 걸친 변경이나 `docs`, `build`, `chore`처럼 모듈과 관계없는 변경은 범위를 생략한다.

### 2.4 커밋 단위

- 커밋 하나에는 논리적인 변경 하나만 담는다.
- 모든 커밋은 그 커밋만으로 빌드가 되어야 한다. `main`에는 작업 브랜치의 커밋이 그대로 들어가므로(3장 병합), 중간 커밋이 깨져 있으면 `git bisect`로 문제 지점을 찾을 수 없다.
- "wip", "오타 수정" 같은 중간 커밋은 `main`에 합치기 전에 앞 커밋과 합친다.
- 서식 일괄 수정(`style`)은 기능 변경과 섞지 않는다.

---

## 3. 병합

### 3.1 순서

작업 브랜치를 `main` 최신 상태 위로 rebase한 뒤 fast-forward로 합친다. `main`의 기록은 병합 커밋 없이 한 줄로 이어진다.

```bash
git switch log-category
git rebase main
git switch main
git merge --ff-only log-category
git push
git branch -d log-category
```

- 작업 브랜치를 원격에 올려둔 뒤 rebase했다면 `--force-with-lease`로 push한다(`--force`는 쓰지 않는다). 합친 뒤 원격 브랜치도 삭제한다.
- `--ff-only`가 실패하면 `main`이 앞서 나간 것이다. 다시 rebase한 뒤 합친다.

### 3.2 합치기 전 확인

- 작업 브랜치의 변경 전체(`git diff main...`)를 한 번 훑어본다. 디버그 코드나 관계없는 파일이 섞이지 않았는지 확인한다.
- Debug / Release 빌드가 경고 없이 성공한다(Code Convention 6.1).
- 바뀐 파일에 clang-format을 적용했다(Code Convention 6.2).
- 컨벤션이나 설정을 바꿨다면 관련 문서도 함께 고쳤다.
