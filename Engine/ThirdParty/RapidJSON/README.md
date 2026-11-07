# RapidJSON

JSON 파일을 읽는 데 사용하는 헤더 전용 라이브러리입니다.

| 항목 | 내용 |
| --- | --- |
| 저장소 | https://github.com/Tencent/rapidjson |
| 커밋 | `24b5e7a8b27f42fa16b96fc70aade9106cf7102f` (2025-02-05) |
| 받은 날짜 | 2026-11-07 |
| 받은 파일 | `https://codeload.github.com/Tencent/rapidjson/tar.gz/24b5e7a8b27f42fa16b96fc70aade9106cf7102f` |
| 받은 파일 SHA512 | `50f8723414a6e63eadd45f97be5c44e9fff2d06216c8cc4df802f5bfc2a9416a039f2c69e9bb1882f7e756cd38a7097eea05cab76c739f45805dc41617140799` |
| 라이선스 | MIT. `include/rapidjson/msinttypes`는 BSD 계열이며 자세한 내용은 [license.txt](license.txt)에 있습니다 |

마지막 정식 릴리스인 v1.1.0은 C++17부터 사용 중단된 `std::iterator`를 상속하므로, vcpkg의 `rapidjson` 포트가 고정한 커밋과 SHA512를 사용합니다.

## 포함한 범위

압축 파일에서 다음만 원본 그대로 복사했고 수정하지 않았습니다.

- `include/rapidjson/`: 헤더 38개
- `license.txt`

JSON 라이선스가 적용되는 `bin/jsonchecker/`와 예제, 테스트, 문서는 포함하지 않습니다.

## 사용

프로젝트의 추가 포함 디렉터리에 `Engine\ThirdParty\RapidJSON\include`가 있으므로 `#include "rapidjson/document.h"`처럼 포함합니다.

## 갱신

다른 커밋으로 바꿀 때는 위 표의 커밋, 받은 날짜, SHA512를 함께 갱신하고 `include/rapidjson/` 폴더 전체를 교체합니다.
