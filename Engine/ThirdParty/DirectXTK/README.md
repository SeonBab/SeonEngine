# DirectXTK

D3D11용 Microsoft DirectX Tool Kit를 vcpkg로 사용한다. 현재 baseline의 패키지는 `directxtk[core,tools]` 2026-05-07#1이며 upstream 태그는 `may2026`이다.

- 패키지 설정: [vcpkg.json](../vcpkg.json)
- 빌드: `x64-windows-static-md` — 정적 링크, CRT는 `/MD`·`/MDd`
- 라이선스 원문: [LICENSE.txt](LICENSE.txt)
- 폰트 도구: 설치 루트의 `tools/directxtk/makespritefont.exe`
- tools 기능에 함께 포함되는 `xwbtool.exe`는 오디오 도구다.

## 배포

DirectXTK는 MIT License로 제공된다. 배포하는 소프트웨어에 해당 저작권·허가·면책 문구를 포함한다. 이 폴더의 LICENSE.txt는 설치된 패키지의 `share/directxtk/copyright`에서 가져왔으며 현재 배포 출력으로 자동 복사하지 않는다. 패키지 버전을 변경하면 고지 원문도 함께 확인한다.

폰트 파일과 MakeSpriteFont로 만든 폰트 자산의 사용·배포 조건은 DirectXTK 라이선스와 별도로 확인한다. 이 폴더에는 폰트나 참고 프로젝트의 라이브러리를 복사하지 않는다.

## 참고

- [DirectXTK may2026 - Microsoft](https://github.com/microsoft/DirectXTK/tree/may2026)
- [MIT License - Microsoft DirectXTK](https://github.com/microsoft/DirectXTK/blob/may2026/LICENSE)
- [SpriteFont - Microsoft DirectXTK](https://github.com/microsoft/DirectXTK/wiki/SpriteFont)
