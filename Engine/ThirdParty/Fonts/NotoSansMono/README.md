# DebugMono 폰트 자산

`DebugMono.spritefont`는 Noto Sans Mono Regular의 글리프를 래스터화한 엔진용 비트맵 폰트다. FPS 등 영문·숫자 표시의 첫 자산이며 한글 글리프는 포함하지 않는다.

| 파일 | 역할 |
|---|---|
| ../../../Content/Fonts/DebugMono.spritefont | 실행 때 읽는 생성 자산 |
| NotoSansMono-Regular.ttf | 수정하지 않은 원본 폰트 |
| OFL.txt | 원본 저장소에서 가져온 SIL Open Font License 1.1 원문 |
| NOTICE.txt | TTF의 저작권, 원본 저장소 고지와 변환 정보 |

## 이름과 배포

DebugMono는 엔진에서 사용하는 자산 이름이다. 다운로드한 라이선스와 TTF의 name 테이블에는 Reserved Font Name 선언이 없음을 확인했다. OFL은 조건에 맞는 수정·형식 변환·재배포를 허용하고, 예약 이름은 저작권 문구 뒤에 명시되어야 한다. 이 근거로 생성 자산 이름을 DebugMono.spritefont로 정했다. 원본 TTF의 내부 이름과 바이트는 유지한다.

원본 TTF와 생성 자산은 OFL 1.1로 배포하며 저작권·허가·면책 고지를 유지한다. 생성 자산을 실행 파일과 배포할 때 OFL.txt와 NOTICE.txt를 함께 둔다. 빌드는 세 파일을 출력의 Content/Engine/Fonts로 복사하며, 원본 TTF와 이 안내는 런타임 출력으로 복사하지 않는다.

## 원본 식별

- 폰트 family: Noto Sans Mono, style: Regular, version: 2.007
- 원본 commit: `ffebf8c1ee449e544955a7e813c54f9b73848eac`
- 원본 경로: `hinted/ttf/NotoSansMono/NotoSansMono-Regular.ttf`
- TTF SHA256: `D9E2B23D19F8230BE7146F409A52B1D23117E635E28F2E2892CF91B7382F325B`
- OFL 원문 SHA256: `0DAB92D0544F7B233403F14B84A663BDBFA746982EDA629E7F4F9FFE1B036FEB`

## 생성 조건과 수동 절차

| 항목 | 값 |
|---|---|
| 크기 | 16pt, 96 DPI 기준 21.333334px |
| 문자 범위 | U+0020~U+007E, ASCII 95글자 |
| 대체 문자 | U+003F (`?`) |
| 스타일 | Regular |
| 알파·텍스처 형식 | 선행곱 알파, CompressedMono |
| 도구 소스 | DirectXTK may2026의 MakeSpriteFont C# 소스 |
| 생성 아틀라스 | 128×164 |
| 진행 폭·줄 간격 | 모든 글자 13px, 줄 간격 29.056px |

현재는 생성 스크립트와 빌드 중 자동 생성을 추가하지 않는다. 폰트가 정상적으로 설치된 환경에서 MakeSpriteFont에 전달할 기본 인수는 다음과 같다.

```text
MakeSpriteFont.exe "Noto Sans Mono" DebugMono.spritefont /FontSize:16 /CharacterRegion:0x20-0x7e /DefaultCharacter:0x3f
```

이번 생성에서는 미설치 폰트를 인식하지 못하는 CLI 경로를 사용하지 않고, 고정된 may2026의 MakeSpriteFont C# 원문을 수정 없이 임시 빌드했다. 수동 명령의 PrivateFontCollection으로 보관한 TTF를 읽고 해당 Font 객체를 원본 TrueTypeImporter.ImportGlyph에 전달했다. 나머지 Crop, ArrangeGlyphs, PremultiplyAlpha, WriteSpriteFont는 그 도구의 구현을 그대로 호출했다. 원본 소스·설치된 prebuilt CLI는 수정하지 않았다. 생성 프로세스에서 만든 Font와 collection은 작업 후 해제했고 영구 폰트 설치는 하지 않았다.

이 과정은 설치된 폰트 이름을 받는 위 CLI 명령만으로 이번 파일을 바이트 단위 재현할 수 있다는 보장이 아니다. 생성 스크립트는 별도 후속 작업이며, 도입할 때 private 폰트 입력과 도구 버전을 고정하고 출력 대조를 포함한다. 자산을 다시 만들면 원본 식별·생성 조건·문자 범위와 고지를 함께 확인한다.

## 참고

- [원본 TTF - Noto Fonts](https://github.com/notofonts/noto-fonts/blob/ffebf8c1ee449e544955a7e813c54f9b73848eac/hinted/ttf/NotoSansMono/NotoSansMono-Regular.ttf)
- [원본 라이선스 - Noto Fonts](https://github.com/notofonts/noto-fonts/blob/ffebf8c1ee449e544955a7e813c54f9b73848eac/LICENSE)
- [OFL FAQ 5.7 - SIL](https://openfontlicense.org/ofl-faq/)
- [MakeSpriteFont - Microsoft DirectXTK](https://github.com/microsoft/DirectXTK/wiki/MakeSpriteFont)
