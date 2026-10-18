# Project Settings

## 메시 텍스처 그리기 접근 전환·전체 빌드 복구 — 2026-10-18

Renderer.cpp의 DrawMesh는 mesh.baseColorTexture.textureView를 const 참조로 읽어 바인딩하며 비어 있으면 기존 whiteTextureView를 사용한다. 옛 baseColorTextureView 소스 참조는 제거됐다. 메시 보관 타입·공통 생성 호출·그리기 접근의 연결 후 격리 출력 Debug/Release x64 전체 제품 빌드가 각각 종료0이다. 아래 이전 단계의 컴파일 불가 상태는 해소됐다. 실제 제품 화면/GPU 실행은 미확인이다. 프로젝트 등록이나 기본 체크/흰색 텍스처 초기화는 이번에 바꾸지 않았다.

## 메시 텍스처 생성 호출 전환 — 2026-10-18

Renderer.cpp의 이미지 입력 CreateMesh는 메시 버퍼 준비 후 CreateTexture2D(baseColorImage, newMesh.baseColorTexture)를 호출한다. 직접 텍스처/SRV 생성과 양수 크기 중복 검사는 제거했다. 모두 성공한 뒤에만 출력 교체를 유지한다. DrawMesh의 옛 멤버 참조가 남아 전체 제품은 아직 컴파일 불가능하며, 이번 검증은 서식·공백·함수 외 원본 보존/호출 관계 정적 확인이다. 다음 그리기 접근 전환 후 전체 빌드를 수행한다.

## 메시 텍스처 보관 타입 단독 전환 — 2026-10-18

StaticMeshRenderData.h에 Renderer/Texture2D.h include를 추가하고 baseColorTextureView(SRV)를 FTexture2D baseColorTexture로 교체했다. 헤더 자체 C++20 /W4 /WX 컴파일과 서식 검사는 통과했다. Renderer.cpp의 CreateMesh/DrawMesh는 아직 옛 멤버를 참조하므로 전체 제품은 현재 컴파일 불가능한 전환 중 상태이며, 아래 이전 전체 빌드 성공은 이번 변경 전 결과다. 프로젝트 등록 변경은 없다. 생성 호출과 그리기 접근을 각각 후속 단계에서 전환한 뒤 전체 빌드를 확인한다.

## Renderer 독립 텍스처 생성 API 추가 — 2026-10-18

Renderer.h/.cpp에 [[nodiscard]] bool CreateTexture2D(const FImage&, FTexture2D&)를 추가했다. FTexture2D는 헤더에서 전방 선언하고 구현에서 직접 include한다. 초기화 후 같은 스레드에서 양수 크기 RGBA8 이미지를 단일 밉의 변경 불가 GPU 텍스처/SRV로 복사하며 둘 다 성공한 뒤 출력을 이동 교체한다. 실패 시 기존 출력 유지, 성공 후 CPU 입력 해제 가능, 장치 변경 시 재생성 계약이다. 크기 상한/픽셀 바이트 수는 기존 D3D11 함수가 검사한다. 프로젝트 등록·메시 보관/생성·기본 텍스처 초기화·그리기·종료는 이번에 변경하지 않았다. 서식/범위 검사와 격리 출력 Debug/Release x64 제품 빌드 종료0을 확인했으며 새 함수 실제 호출·GPU 생성·화면 실행은 미확인이다.

## FTexture2D 최소 GPU 자원 타입 등록 — 2026-10-18

Engine/Source/Renderer/Texture2D.h를 SampleGame 프로젝트 ClInclude와 기존 Engine\Source\Renderer 필터에 각 한 번 등록했다. FTexture2D는 private ComPtr<ID3D11ShaderResourceView>와 FRenderer friend 접근을 가지며 복사는 금지하고 기본 생성/이동을 허용한다. 기본 상태는 빈 뷰이며 생성/조회/그리기 API와 별도 cpp는 없다. WindowsHWrapper를 거쳐 SDK를 포함한다. C++20 /W4 /WX 헤더 컴파일·서식·XML 등록 검사를 통과했으며 전체 제품 빌드/실행·GPU 자원 생성은 이번에 수행하지 않았다.

2026-10-18 텍스처 객체 전환 정정: 위 FTexture2D를 UTexture2D : public UObject로 변경했다. Texture2D.h 파일명·경로와 SRV 멤버/Renderer friend는 유지하며 생성자·복사 금지·이동 허용 선언도 새 이름으로 맞췄다. Renderer의 전방 선언·출력 인수·임시 객체와 StaticMeshRenderData의 값 멤버도 함께 변경했다. 기존 GPU 생성/출력 보존·값 보관·이동 코드는 유지한다. UObject는 가상 소멸자의 최소 기반이므로 GC·리플렉션·자원 공유·관리자·스트리밍이 생긴 것은 아니다. UI의 텍스처 소유/참조 연결은 후속이다.

## UImageWidget 최소 기반 등록 — 2026-10-18

Engine/Source/UI/ImageWidget.h의 UImageWidget은 UWidget을 public 상속하는 빈 이미지 UI 기반이다. 기존 CPU 이미지 타입 FImage와 Image/Image.h는 유지한다. SampleGame 프로젝트 ClInclude와 기존 Engine\Source\UI 필터에 새 헤더를 한 번 등록했다. 픽셀/텍스처·위치/크기·설정/그리기 함수·별도 cpp는 없다. 헤더 C++20 /W4 /WX 컴파일·서식·XML 등록 검사를 통과했으며 전체 제품 빌드/실행은 이번에 수행하지 않았다.

2026-10-18 텍스처 독점 소유 추가: UImageWidget에 TUniquePtr<UTexture2D> texture와 SetTexture(TUniquePtr<UTexture2D>&)를 추가했다. 성공하면 입력을 비우고 기존 객체를 즉시 파괴하며 빈 입력은 false로 기존 소유를 유지한다. 비어 있지 않은 객체의 소유권 전달만 검사하며 GPU 준비 여부는 검사하지 않는다. Texture2D.h를 직접 포함해 소유 타입을 완성하고 별도 cpp를 SampleGame ClCompile과 기존 Engine\Source\UI 필터에 각 한 번 등록했다. 독립 C++20 /W4 /WX 검사에서 전달·빈 입력 거부·교체·위젯 파괴·루트 교체·호스트 파괴의 객체 수명을 확인했고 Debug/Release x64 전체 제품 빌드를 통과했다. 게임의 이미지 로딩·GPU 생성·위젯 전달 연결과 UI 그리기는 아직 없다. 제품 창·렌더링 관찰은 수행하지 않았다.

2026-10-18 위치·표시 크기 추가: UImageWidget은 기본값이 모두 0인 FVector2 position·size를 보관한다. SetPosition/SetSize는 값을 복사하고 GetPosition/GetSize는 복사본을 반환하며 값 검사·좌표 변환은 수행하지 않는다. 위치는 클라이언트 영역 왼쪽 위 기준 픽셀 좌표, 크기는 원본 텍스처 해상도와 별개인 픽셀 너비·높이로 사용할 데이터다. SampleGame의 ConfigureGameClient에서 (20, 20), (128, 128)을 지정했다. 변경은 ImageWidget.h/.cpp와 SampleGame.cpp 세 파일이며 프로젝트 등록·SetTexture·GPU 생성·그리기는 유지했다. Debug/Release x64 전체 제품 빌드와 서식·변경 범위 검사를 통과했다. UI 출력이 없어 실제 배치·창 크기 변경·DPI 동작은 확인하지 않았으며 이번 검증은 빌드/정적 확인이다.

## GetRootWidget 읽기 접근 — 2026-10-18

UI/UserWidget.h의 const UWidget* GetRootWidget() const는 내부 루트를 읽기 전용 비소유 포인터로 반환한다. 없으면 nullptr이고 루트 교체/파괴 또는 UserWidget 파괴 뒤 반환 포인터를 사용할 수 없다. getter에는 nodiscard를 붙이지 않는 기존 규칙을 따른다. cpp/프로젝트 등록 변경은 없다. 독립 C++20 /W4 /WX 컴파일·링크·실행과 서식 검사를 통과했고 전체 제품 빌드/화면 실행은 이번 단계에서 수행하지 않았다.

## SetRootWidget 구현과 cpp 등록 — 2026-10-18

UI/UserWidget.cpp를 SampleGame 프로젝트 ClCompile과 기존 Engine\Source\UI 필터에 한 번 등록했다. UUserWidget::SetRootWidget(TUniquePtr<UWidget>&)는 빈 입력/직접 자기 자신을 false로 거부하며 기존 상태를 유지한다. 성공하면 기존 루트를 즉시 파괴하고 새 루트의 소유권을 받아 입력을 비운다. 이전 루트와 그 소유 요소의 비소유 포인터는 교체 후 사용할 수 없다. 독립 수명 검사와 격리 출력 x64 Debug/Release 전체 제품 빌드는 통과했으며 제품 화면 실행은 하지 않았다.

## UUserWidget 루트 소유 멤버 — 2026-10-18

UI/UserWidget.h에 Core/Templates/UniquePtr.h include와 private TUniquePtr<UWidget> rootWidget만 추가했다. 내부 루트 요소 한 개를 단독 소유할 자리이며 기본 상태는 비어 있다. 설정/조회 함수와 별도 cpp·프로젝트 항목 변경은 없다. 헤더 C++20 /W4 /WX 컴파일·서식 검사를 통과했고 전체 제품 빌드/실행은 이번 단계에서 수행하지 않았다.

## UI 공통 사각형 GPU 자원 — 2026-10-18

Renderer.h의 FStaticMeshRenderData imageQuad를 모든 이미지 그리기에 재사용할 내부 자원으로 추가했다. Initialize 마지막에 (0,0)~(1,1)의 흰색 FVertex 4개·전체 UV·인덱스 6개를 가진 지역 FStaticMeshData를 기존 CreateMesh로 생성한다. 별도 텍스처를 만들지 않으며 CPU 데이터는 초기화 후 보관하지 않는다. 공용 생성 함수의 초기화 검사를 만족하도록 기존 장치·스왑 체인 준비/상태 설정 뒤 호출하며 실패 시 Shutdown 후 false다. Shutdown은 바인딩 해제 뒤 imageQuad를 빈 값으로 교체해 GPU 참조를 정리하고 장치를 종료한다. Renderer.h/.cpp 두 파일만 변경했으며 기존 생성·프레임·그리기 함수와 게임·UI 호출부·프로젝트는 유지한다. TODO(seon)로 UI 배칭·전용 정점 도입 시 보관 형식 재검토를 표시했다. Debug/Release x64 전체 제품 빌드와 별도 실제 D3D11 장치/Renderer의 초기화·중복 초기화 거부·종료 반복·재초기화 및 기존 월드 픽셀 유지 검사를 통과했다. imageQuad는 아직 private 준비 자원이며 직접 그리기·버퍼 내용 readback·제품 창·실제 DXGI Present·UI 출력은 확인하지 않았다.

## UI 변환 상수 버퍼 — 2026-10-18

Renderer.h에 월드용 transformConstantBuffer와 별도인 ComPtr<ID3D11Buffer> imageTransformConstantBuffer를 추가했다. Initialize는 기존 D3D11 CreateConstantBuffer/UpdateConstantBuffer로 FMatrix4 크기 64바이트 버퍼를 만들고 항등행렬을 업로드하며 실패 시 Shutdown 후 false다. Shutdown은 VS b0 연결 해제 뒤 두 버퍼를 각각 정리한다. 이미지마다 만들지 않고 Renderer가 하나를 소유하는 UI용 버퍼이며 픽셀 위치·크기 변환 계산·셰이더 바인딩·DrawImage는 아직 없다. Renderer.h/.cpp 두 파일만 바꾸고 기존 월드 변환·프레임·그리기·UI·게임·셰이더·프로젝트는 유지했다. Debug x64 전체 빌드는 종료0이며 Release 최초 빌드는 소스 진단 없이 CL.exe -1073741819로 종료했다. Renderer 소스 해시를 유지한 재시도 빌드는 종료0이다. 별도 오프스크린 검사에서 실제 장치·Renderer 초기화의 생성/업로드 성공 경로, 종료 반복·재초기화와 기존 월드 픽셀 유지를 확인했다. private UI 버퍼의 직접 내용 readback·바인딩·UI 픽셀·제품 창·실제 DXGI Present는 확인하지 않았다.

## ImageWidget 그리기 연결 — 2026-10-18

UImageWidget::Draw는 텍스처가 없으면 그리기를 건너뛰고 true를 반환한다. 있으면 저장된 texture·position·size를 Renderer::DrawImage로 전달하고 결과를 그대로 반환한다. Renderer는 보관하지 않으며 GPU/위치/크기 검사는 기존 Renderer 계약을 따른다. 실행부 → Subsystem → UserWidget → ImageWidget → Renderer 연결을 완료했다. 첫 이미지 표시에는 알파 합성을 요구하지 않으며 투명도 처리는 보류한다. 제품 창 표시는 별도 확인한다.

## 화면 이미지 DrawImage — 2026-10-18

Renderer.h/.cpp에 [[nodiscard]] bool DrawImage(const UTexture2D&, FVector2 position, FVector2 size)를 추가했다. 활성 프레임·현재 장치의 유효한 텍스처·유한한 위치/양수 크기와 생성 행렬의 유한 성분을 검사한다. 왼쪽 위 기준 픽셀 위치/크기와 현재 출력 크기로 행 벡터용 imageToClip을 만들고 imageTransformConstantBuffer에 업로드한다. 기존 VS/PS/입력 레이아웃/샘플러와 imageQuad를 바인딩해 DrawIndexed를 요청한다. 음수 위치는 허용하고 출력 밖은 GPU가 자른다. 월드 행렬/버퍼 데이터는 변경하지 않으며 UI 바인딩을 남긴다. 이후 DrawMesh는 자기 월드 버퍼를 다시 바인딩한다. 알파 합성·ImageWidget override·셰이더/프로젝트 변경은 없다. Debug/Release x64 전체 제품 빌드와 실제 장치의 오프스크린 픽셀 검사에서 위치/크기·네 방향 UV·텍스처 색·여러 이미지 누적·창 크기 변경·카메라 독립·별도 VS 상수 버퍼와 월드/UI/월드 변환 유지를 확인했다. 빈/다른 장치 텍스처·비유한/잘못된 크기·행렬 범위 초과는 그리기 전 거부한다. 제품 창·실제 DXGI Present·위젯 경유 이미지 출력은 아직 관찰하지 않았다.

## 위젯 그리기 요청 전달 — 2026-10-18

UI/Widget.h에 FRenderer 전방 선언과 [[nodiscard]] virtual bool Draw(FRenderer&)를 추가했다. 기본 구현은 Renderer를 사용하지 않고 true를 반환하며 미사용 인수를 [[maybe_unused]]로 표시한다. UUserWidget은 override하여 루트가 없으면 true, 있으면 rootWidget->Draw(renderer) 결과를 그대로 반환한다. Renderer를 멤버로 보관하거나 소유하지 않는다. 세 코드 파일만 변경하고 기본 구현을 헤더에 두어 cpp/프로젝트 등록은 추가하지 않았다. Debug/Release x64 전체 제품 빌드와 독립 C++20 /W4 /WX 검사에서 기반 참조를 통한 가상 호출·동일 Renderer 전달·중첩 루트 성공/실패 전달·루트 교체 후 호출 대상과 소유 수명 유지를 확인했다. Subsystem/실행부 연결과 UImageWidget의 실제 Draw override는 없으며 기본 함수를 상속한 이미지는 아직 그리지 않는다. GPU·제품 창·UI 출력 관찰은 수행하지 않았다.

2026-10-18 Subsystem 순회 추가: GameWidgetSubsystem.h/.cpp에 [[nodiscard]] bool Draw(FRenderer&)를 추가했다. 등록 순서대로 UserWidget::Draw를 호출하며 첫 false에서 즉시 중단하고 빈 목록은 true다. Renderer를 호출 동안만 빌리고 기본 GPU 명령은 추가하지 않는다. Draw 호출 중 등록·제거는 호출자 계약으로 금지하며 자동 감지·보류·스냅샷 순회는 구현하지 않았다. 기존 AddWidget/RemoveWidget 본문과 실행부·위젯·Renderer·프로젝트는 유지했다. Debug/Release x64 전체 제품 빌드와 독립 C++20 /W4 /WX 검사에서 루트까지의 등록 순서·동일 Renderer 전달·첫 실패 뒤 호출 중단·제거 제외·추가 등록 순서·소유 수명 유지를 확인했다. 실행부 호출과 실제 이미지 GPU 그리기·제품 화면 관찰은 아직 없다.

2026-10-18 실행부 UI 요청 연결: EngineMain.cpp의 기존 프레임 실패 조건식에서 Scene.Draw 뒤·EndFrame 앞에 gameClient.GetWidgetSubsystem().Draw(renderer)를 추가했다. UI 요청이 false이면 EndFrame을 건너뛰고 기존 exitCode=1·반복 중단·종료 경로를 사용한다. 실제 ImageWidget은 기본 Draw를 상속해 현재는 요청만 받고 아무것도 그리지 않는다. 다른 소스·프로젝트 변경 없이 Debug/Release x64 전체 제품 빌드 종료0과 실제 조건식을 추출한 별도 오프스크린 검사에서 기본 이미지의 월드 픽셀 유지, 테스트용 루트까지 월드 뒤·Present 앞 호출, UI 실패 시 EndFrame/Present 생략을 확인했다. 테스트용 루트의 메시 픽셀로 순서를 확인한 것이며 제품 ImageWidget의 GPU 그리기나 제품 창·메시지 반복·실제 DXGI Present를 관찰한 것은 아니다.

## SampleGame 기본 사용자 위젯 등록 — 2026-10-18

SampleGame.cpp의 ConfigureGameClient는 MakeUnique<UUserWidget>()로 위젯 하나를 만든다. 2026-10-18 후속으로 MakeUnique<UImageWidget>()을 TUniquePtr<UWidget>에 담아 SetRootWidget에 전달하고 성공한 뒤 gameClient.GetWidgetSubsystem().AddWidget(widget)의 결과를 반환하도록 연결했다. 실패 시 false로 반환하며 소유 중인 지역 위젯/루트는 자동 정리된다. ImageWidget.h와 Widget.h를 직접 포함한다. 소유 방향은 Client → Subsystem → UserWidget → ImageWidget이며 이미지 데이터나 GPU 연결을 추가한 것은 아니다. GameClient.h와 UI/UserWidget.h를 직접 포함하고 실제 인수를 사용하므로 maybe_unused를 제거했다. 기존 World 구성과 프로젝트 항목은 유지한다. 내부 ImageWidget 루트는 있지만 UI GPU 자원·그리기는 없어 화면에는 표시하지 않는다. 격리 출력 x64 Debug/Release 전체 제품 빌드는 종료0이며 실제 제품 실행은 하지 않았다. 아래 빈 성공 구현 상태는 이 절로 정정한다.

2026-10-18 UI 텍스처 준비 연결: ConfigureGameClient는 실행 파일 기준 Content/Textures/Checker.png를 지역 FImage로 읽고 MakeUnique<UTexture2D>()와 빌린 Renderer의 CreateTexture2D로 GPU 자원을 준비한다. 성공하면 UImageWidget::SetTexture로 소유권을 전달하고 기존 UserWidget 루트/등록 경로에 연결한다. 월드와 같은 파일을 사용하지만 별도 로딩·GPU 텍스처이며 공유하지 않는다. 경로·디코딩·GPU 생성 실패는 등록 전에 false로 종료하며 지역 소유 객체는 정리된다. 직접 사용하는 Renderer.h·Texture2D.h를 포함하고 Renderer 인수의 maybe_unused를 제거했다. 다른 함수·엔진 소스·프로젝트·콘텐츠는 변경하지 않았다. Debug/Release x64 빌드 종료0과 제품 D3D11 장치/Renderer·실제 ConfigureGameClient를 사용하는 오프스크린 검사에서 성공, 파일 없음/손상·미초기화 Renderer 실패, 기존 등록 유지와 GPU 텍스처 객체 소유 전달·파괴를 확인했다. UI 텍스처 조회나 UI 픽셀 검사·제품 창·실제 DXGI Present 관찰은 없으며 위치/크기·그리기 연결은 미구현이다.

## Client 게임 구성 함수 연결 — 2026-10-18

Launch/GameEntry.h에 ConfigureGameClient(FGameClient&)를 선언하고 SampleGame.cpp가 게임별 구현을 제공한다. 실행부는 ConfigureGameWorld → ConfigureGameClient → World.CreateRenderStates 순서로 성공 여부를 확인한다. 현재 Client 구성 구현은 maybe_unused 참조 인수와 return true만 있으며 UI를 생성하지 않는다. 파일/프로젝트 항목 추가는 없다. 격리 출력 x64 Debug/Release 전체 제품 빌드 종료0을 확인했으며 실제 제품 실행은 하지 않았다.

2026-10-18 Renderer 참조 연결: 위 초기 진입 형태를 ConfigureGameClient(FGameClient&, FRenderer&)로 변경했다. GameEntry.h에 FRenderer를 전방 선언하고 초기화된 Renderer를 빌린다는 계약을 명시했다. EngineMain은 소유 중인 Renderer를 전달하고 SampleGame 구현은 아직 사용하지 않는 인수에 [[maybe_unused]]를 붙였다. 함수 본문과 World 구성·위젯 생성/등록·실패/종료 흐름은 유지하며 텍스처 생성이나 SetTexture 호출은 추가하지 않았다. 파일/프로젝트 등록 변경 없이 세 코드 파일만 수정했고 Debug/Release x64 전체 제품 빌드와 서식/범위 검사를 통과했다. 제품 창·GPU/UI 출력은 관찰하지 않았다.

## Launch 헤더 include 경로 정정 — 2026-10-18

실제 파일은 Engine/Source/Launch/EngineMain.cpp와 GameEntry.h다. SampleGame.cpp의 include는 Launch/GameEntry.h로 정정했고 두 파일은 프로젝트/필터에 등록돼 있다. 같은 Launch 폴더의 EngineMain.cpp는 기존 GameEntry.h 표기를 유지한다. include 검색 경로와 실행 로직은 바꾸지 않았다. 아래 이전 include 오류에 따른 전체 빌드 실패 상태는 이번 정정 후 격리 출력 Debug/Release x64 제품 빌드 성공으로 갱신한다. 제품 화면 실행은 확인하지 않았다.

## 실행부의 FGameClient 생성 — 2026-10-18

Launch/EngineMain.cpp가 GameClient.h를 포함하고 Renderer 초기화 성공 후 기존 World 범위 안에서 UWorld 다음에 FGameClient 지역 객체를 생성한다. 범위 종료 시 Client/소유 위젯 → World → 기존 Renderer.Shutdown 순서로 정리한다. 게임 구성 함수는 여전히 World만 받으며 UI는 아직 생성하지 않는다. Debug/Release 모두 EngineMain.obj 생성은 확인했으나 전체 빌드는 SampleGame.cpp의 이전 GameEntry.h include 오류로 실패했다. 제품 빌드/실행 성공으로 해석하지 않는다.

## Client의 위젯 Subsystem 접근 — 2026-10-18

FGameClient::GetWidgetSubsystem()은 소유 멤버의 비상수 참조를 반환하는 인라인 getter다. 소유권을 넘기지 않으며 Client가 파괴되면 반환 참조도 사용할 수 없다. getter에는 nodiscard를 붙이지 않는 코드 규칙을 따른다. 파일/프로젝트 등록 변경은 없다. Client를 통한 등록·제거·종료 정리를 독립 콘솔 실행으로 확인했으며 전체 제품 빌드와 실행부 연결은 아직 수행하지 않았다. 아래 접근 함수 없음 기록은 이 절로 정정한다.

## FGameClient 최소 기반 등록 — 2026-10-18

Engine/Source/GameClient.h를 SampleGame 프로젝트 ClInclude와 기존 Engine\Source 필터에 각 한 번 등록했다. UI/GameWidgetSubsystem.h를 포함하고 private FGameWidgetSubsystem widgetSubsystem을 직접 멤버로 소유한다. 별도 cpp·include 경로·실행부 연결은 없다. Subsystem 접근 함수와 입력 기능도 아직 없다. 헤더 컴파일·서식·XML 등록 검사를 통과했으며 전체 제품 빌드/실행은 수행하지 않았다.

## 위젯 제거 계약 반영 — 2026-10-18

FGameWidgetSubsystem::RemoveWidget(UUserWidget*)를 기존 UI/GameWidgetSubsystem.h/.cpp에 추가했다. 등록 객체는 즉시 파괴 후 true, nullptr 또는 미등록 생존 객체는 false로 목록을 유지한다. 실제 파괴 뒤 포인터를 재사용하면 안 된다. 파일/프로젝트 등록 변경은 없다. 독립 콘솔 수명 검사는 통과했고 전체 제품 빌드/화면 실행은 하지 않았다. 아래 제거 미구현 내용은 이 절로 정정한다.

## 위젯 등록 함수 빌드 등록 — 2026-10-18

Engine/Source/UI/GameWidgetSubsystem.cpp를 SampleGame 프로젝트 ClCompile과 기존 Engine\Source\UI 필터에 각 한 번 등록했다. FGameWidgetSubsystem::AddWidget(TUniquePtr<UUserWidget>&)는 빈 입력을 false로 거부하고 유효 입력을 소유 목록 끝으로 이동한다. 성공 시 입력 포인터는 비워진다. 메모리 할당 실패의 false 복구는 제공하지 않는다. 제거·조회·실행부 연결은 아직 없다. 실제 cpp의 독립 C++20 /W4 /WX 컴파일·링크·콘솔 수명 검사는 통과했고 전체 제품 빌드/제품 창 실행은 수행하지 않았다.

## FGameWidgetSubsystem 소유 목록 등록 — 2026-10-18

Engine/Source/UI/GameWidgetSubsystem.h를 SampleGame 프로젝트 ClInclude와 기존 Engine\Source\UI 필터에 한 번 등록했다. Core/Containers/Array.h, Core/Templates/UniquePtr.h, 같은 UI 폴더의 UserWidget.h를 포함한다. 현재 private TArray<TUniquePtr<UUserWidget>> widgets만 보관하며 등록·제거·조회·그리기 API와 별도 cpp는 없다. 헤더 컴파일과 서식·XML 등록 검사를 통과했다. 전체 제품 빌드·실행 연결은 수행하지 않았다.

## UUserWidget 최소 기반 등록 — 2026-10-18

Engine/Source/UI/UserWidget.h를 SampleGame 프로젝트의 ClInclude와 Engine\Source\UI 필터에 한 번 등록했다. 같은 UI 폴더의 Widget.h를 포함하고 UWidget을 public 상속한다. 별도 cpp·컴파일 항목·include 검색 경로는 추가하지 않는다. 현재 구성/동작을 정의할 타입 기반만 있으며 내부 루트·등록·그리기는 없다. 기존 UWidget 등록과 Object.h 위치는 유지한다. 헤더 컴파일과 서식·XML 등록 검사는 통과했고 전체 제품 빌드/화면 실행은 수행하지 않았다.

## UI 최소 기반 등록 — 2026-10-18

UWidget은 Engine/Source/UI/Widget.h에 둔다. Engine/Source를 기준으로 UI/Widget.h를 include하며 UObject 기반의 Object.h는 기존 Source 루트에 유지한다. SampleGame 프로젝트에는 ClInclude로 한 번 등록하고 필터는 Engine\Source\UI를 사용한다. 별도 cpp·컴파일 항목·include 검색 경로는 추가하지 않는다. 현재 UWidget은 UObject를 public 상속하는 빈 기반이며 위치·크기·렌더링 기능은 없다. 헤더 C++20 /W4 /WX 컴파일과 서식·XML 등록 검사를 통과했다. 제품 전체 빌드와 화면 실행은 이번 단계에서 수행하지 않았다.

## Components 디렉토리 정리 — 2026-10-18

ActorComponent・TransformComponent・StaticMeshComponent・CameraComponent・InputComponent의 .h/.cpp 총10개 파일은 실제 `Engine/Source/Components/`에 둔다. Key.h와 InputState.h/.cpp는 `Engine/Source/Input/`에 유지하며 Object.h도 기존 위치에 둔다. 컴포넌트 외부에서는 `Components/ActorComponent.h`, `Components/InputComponent.h`처럼 include한다. 동일 폴더 내 헤더를 파일명으로 포함하는 기존 표기는 유지한다.

사용자가 실제 이동과 프로젝트/필터 등록을 수행했고 후속 작업에서 남은 외부 include13곳(6개 파일)을 정리했다. 생성기나 include 검색 경로를 추가로 바꾸지 않았다. 상속·소유·실행 계약은 유지하며 아래 이전 컴포넌트 파일 위치는 이 절에서 정정한다. 전체 소스/필터 등록과 include 경로·서식·함수 본문 동일성 검사, 격리 출력 Debug/Release x64 빌드를 통과했다. 제품 화면 실행은 확인하지 않았다.

## Core 디렉토리 정리 — 2026-10-18

실제 소스 파일 20개를 `Engine/Source/Core/` 아래로 이동했다. `CoreTypes.h`와 `Containers/`, `Templates/`, `Math/`, `Logging/`, `Misc/`, `Time/`가 대상이다. `Object.h`는 `Engine/Source/Object.h`에 유지한다. Core는 폴더 분류이며 별도 프로젝트·라이브러리·Public/Private 계층은 추가하지 않는다.

include 검색 기준은 기존 `$(SERootDir)Engine/Source` 하나를 유지한다. 이동한 헤더는 `Core/CoreTypes.h`, `Core/Math/Matrix4.h`처럼 실제 경로로 포함한다. 같은 폴더의 구현 파일이 자기 헤더를 포함하는 `#include "Matrix4.h"` 같은 방식은 유지한다. Core를 별도 include 검색 경로로 추가하거나 이전 위치에 호환 헤더를 두지 않는다.

`GenerateProjectFiles.bat`은 다시 사용할 수 있다. 생성기는 `SampleGame/Source` 프로젝트에 한해 게임 소스 외에 `Engine/Source`와 `Engine/Shaders`를 함께 검색한다. 다른 도구 프로젝트에는 엔진 소스를 자동 추가하지 않는다. 소스·필터 목록을 생성하며 기존 FxCompile 설정, Content 등록과 나머지 프로젝트 설정은 보존한다. Content는 필터에도 유지한다. 아래 이전 기록의 Core 경로와 생성기 미사용 방침은 이 절에서 정정하며, 별도 Engine.lib·모듈 구성 설명은 과거 상태다.

프레임 시간 측정 분리(2026-10-18): Engine/Source/Core/Time/FrameTimer.h·.cpp의 FFrameTimer가 steady_clock 기준 시각과 경과 초 계산을 담당한다. main의 지역 객체로 생성하며 생성 시 기준을 잡고 Tick()마다 직전 측정부터 경과한 float 초를 반환하고 기준을 갱신한다. EngineMain은 최소화 검사 앞에서 측정하고 기존 UWorld::Tick(deltaSeconds, input)에 전달한다. 게임 시간 누적·배속·일시정지·고정 갱신·시간 상한·프레임 대기는 추가하지 않는다. SampleGame/Source/SampleGame.vcxproj에 cpp/h를 각 한 번 등록하고 .filters에서는 기존 Engine\Source 분류를 사용한다.

World 입력 전달(2026-10-18): EngineMain은 UWorld::Tick(deltaSeconds, input)에 입력 상태를 const 참조로 전달한다. 각 액터의 BeginPlay 완료 확인→입력 컴포넌트 처리→직접 제거 요청 확인→Actor::Tick 순서이며 입력 콜백도 기존 Tick의 실제 제거 보류 범위 안에서 실행한다. 자기 제거 요청 시 현재 ProcessInput은 완료하고 남은 입력 컴포넌트와 Tick은 생략한다. 액터별 입력 목록은 처리 시 수집하고 그 목록을 처리하는 중 추가한 컴포넌트는 다음 Tick부터 반영한다. World는 입력을 저장하거나 소유하지 않는다. SampleGame의 실제 바인딩 사용은 아직 없다.

입력 컴포넌트 등록(2026-10-18): SampleGame/Source/SampleGame.vcxproj에 Engine/Source/Input/InputComponent.cpp를 ClCompile, InputComponent.h를 ClInclude로 등록하고 .vcxproj.filters에서는 기존 Engine\Source 분류에 표시한다. 입력 상태 조회·바인딩·호출 경로와 게임 액터 부착은 아직 없다.

입력 소스 등록(2026-10-17): SampleGame/Source/SampleGame.vcxproj에 Input/InputState.cpp와 Windows/WindowsInput.cpp를 ClCompile로, Input/Key.h·Input/InputState.h·Windows/WindowsInput.h를 ClInclude로 등록한다. 경로 기준은 Engine/Source이며 .vcxproj.filters에서는 기존 Engine\Source 분류에 표시한다. 입력 상태의 소유자·Windows 메시지·게임 조회 연결은 아직 없다.

카메라 소스 등록(2026-10-17): SampleGame/Source/SampleGame.vcxproj에 Engine/Source/CameraComponent.cpp를 ClCompile, CameraComponent.h를 ClInclude로 등록한다. .vcxproj.filters에서는 기존 Engine\Source 분류에 표시한다. 게임과 공통 엔진을 함께 빌드하는 현재 구성을 유지하며 카메라의 게임 배치나 뷰/투영 연결을 추가하지 않는다.

외부 고지 원문 보존(2026-10-17): .gitattributes의 기존 텍스트/바이너리 규칙을 유지하고 Engine/ThirdParty/Assimp/Licenses/*.txt에 -text 예외를 둔다. Git 줄바꿈 변환으로 배포 고지 원문의 바이트와 index.json의 SHA-256이 달라지지 않게 한다. 원문 공백은 수정하지 않으며 해당 파일은 일반 코드의 공백 검사와 구분해 원문 해시로 확인한다.

현재 Scene 이미지 등록/교체(2026-10-17): FScene::AddStaticMesh(renderer, FModelMeshData, FImage, matrix, outId)와 SetMeshData(id, renderer, FModelMeshData, FImage)가 Renderer의 이미지 포함 CreateMesh를 사용한다. CPU 입력은 호출 뒤 보관할 필요가 없고 파일 읽기/재질 해석은 호출자 책임이다. Add 실패는 기존 목록·출력 ID를 유지하며 ID를 소비하지 않는다. Set은 ID·행렬·순서를 유지하고 실패하면 기존 버퍼와 텍스처를 보존한다. 기존 모델 흰색/FVertex 오버로드와 같은 스레드의 프레임 사이 호출 계약을 유지한다. 텍스처 공유·컴포넌트/게임 이미지 매핑은 추가하지 않았다. 아래 Scene 이미지 미연결 내용은 이전 단계 이력이다.

모델 색상 텍스처 도입 당시(2026-10-17): FRenderer::CreateMesh(FModelMeshData, FImage, outMesh)는 모델 버퍼와 양수 크기의 단일 밉 RGBA8_UNORM 변경 불가 텍스처/SRV를 준비하고 성공 후만 출력을 교체한다. 실패는 이전 메시/텍스처를 보존하고 성공 후 CPU 입력은 해제할 수 있다. FStaticMeshRenderData의 SRV가 텍스처 수명을 유지하며 같은 이미지의 반복 업로드는 공유하지 않는다. 모델 입력 레이아웃/셰이더가 위치와 UV를 사용하고 DrawMesh가 매번 해당 SRV 또는 초기화한1×1 흰색 SRV와 기존 point/clamp 샘플러를 바인딩한다. 기존 체크 텍스처 경로와 셰이더 파일명/프로젝트 항목은 유지한다. 재질색·조명·색상 공간/알파 블렌딩/깊이·Scene/게임 연결은 추가하지 않는다.

현재 DDS 이미지 읽기(2026-10-17): FImageUtils::LoadImageFile은 PNG 단일 프레임과 단일2D BC1/BC2/BC3 UNORM DDS를 지원한다. 확장자 대신 WIC 컨테이너로 판별하며 DDS는 IWICDdsDecoder로 ArraySize1/Depth1/밉1개이상과 형식을 확인한 뒤 GetFrame(0,0,0)을 기존 FImage의 RGBA8로 변환한다. 하위 밉·압축 형식·색상 공간은 보관하지 않는다. 배열·큐브·3D/다른 DDS 형식은 거부하며 실패는 이전 출력을 유지한다. Windows8.1 이상 WIC DDS 코덱 범위에 의존하고 새 라이브러리/프로젝트 항목은 추가하지 않는다. 모델별 GPU 텍스처 연결과 TriWorld 반영은 후속이다.

Scene 모델 메시 연결 도입 당시(2026-10-17): FScene::AddStaticMesh/SetMeshData는 FStaticMeshData와 FModelMeshData 오버로드를 제공했다. 2026-10-18 모델 로딩 결과를 FStaticMeshData로 전환한 뒤, 사용되지 않는 이미지 없는 FModelMeshData용 AddStaticMesh 선언·설명·구현만 Scene.h/.cpp에서 제거했다. 이미지 없는 모델 등록 함수를 제거한 당시에는 공통 등록·이미지 입력 모델 등록·모델 교체 오버로드를 유지했다. 같은 날 다음 단계에서 FModelMeshData+FImage용 AddStaticMesh 선언·설명·구현도 제거했다. Scene 등록은 선택적 이미지 포인터를 받는 공통 FStaticMeshData 함수 하나다. 다음 승인 단계에서 이미지 없는 FModelMeshData용 SetMeshData 선언·설명·구현만 제거했다. 이미지 없는 모델 교체 제거 당시에는 공통 교체와 이미지 입력 모델 교체·Renderer 오버로드를 유지했다. 다음 승인 단계에서 FModelMeshData+FImage용 SetMeshData 선언·설명·구현과 불필요해진 전방 선언도 제거했다. Scene 등록·교체는 FStaticMeshData와 선택적 이미지 포인터를 받는 공통 함수만 남았으며 Renderer 전용 오버로드는 유지한다. 모델 입력도 기존 GPU 자원/행렬/ID 목록을 사용한다. Add는 자원 생성 성공 후만 등록하고 출력 ID를 바꾼다. Set은 ID·행렬·등록 순서를 유지하며 모델/기존 정점 형식 간 교체를 허용한다. 실패 시 기존 자원과 형식을 유지한다. CPU 입력은 호출 후 보관할 필요가 없고 컴포넌트/모델 노드/재질/게임 배치는 아직 연결하지 않았다.

2026-10-18 모델 이미지 생성 API 정리: Scene과 로딩 결과가 공통 형식으로 전환되어 호출되지 않는 FRenderer::CreateMesh(FModelMeshData,FImage,outMesh)의 선언·설명·구현만 Renderer.h/.cpp에서 제거했다. 이미지가 필요한 모델도 CreateMesh(FStaticMeshData,outMesh,&image)를 사용한다. 이미지 입력 모델 생성 함수를 제거한 당시에는 공통 생성과 이미지 없는 모델 전용 생성 함수·공개 CreateTexture2D를 유지했다. 다음 승인 단계에서 이미지 없는 FModelMeshData용 CreateMesh와 전방 선언·ModelData.h include도 Renderer.h/.cpp에서 제거했다. CreateMesh는 이제 FStaticMeshData와 선택적 이미지 포인터를 받는 공통 함수 하나다. 공통 생성·CreateTexture2D·DrawMesh 구현은 유지했다. 다음 승인 단계에서 ModelData.h의 미사용 FModelMeshData 정의·설명과 직접 Vertex.h include만 제거해 이전 메시 타입 정리도 완료했다. FModelData의 공통 메시 배열·재질·노드는 그대로다.

현재 모델 GPU 기반(2026-10-17): 당시 FRenderer::CreateMesh(FModelMeshData, outMesh)는 모델 전용 정점 배열을32바이트 간격으로 복사했다. 2026-10-18 정점 공통화로 FModelVertex를 제거하고 FModelMeshData가 FVertex 배열을 보관하도록 전환했다. 로더는 위치·법선·UV를 기존처럼 채우고 정점 색상은 흰색이다. 모델 전용 CreateVertexBuffer 오버로드를 제거해 기존 공통 업로드를 사용하며 모델 입력은 offsetof(FVertex,position/uv), 그리기 간격은 공통 sizeof(FVertex)=48을 사용한다. 색상·법선은 모델 셰이더에 전달하지 않는다. 정점 통일 당시에는 FModelMeshData와 기존 셰이더/텍스처 선택을 유지했다. 2026-10-18 후속으로 DrawMesh 함수의 모델 분기만 제거해 두 메시가 일반 메시용 셰이더·입력 레이아웃을 공통 사용한다. 모델 정점 색상도 텍스처에 곱하며 로더의 기본 흰색은 기존 텍스처 색상을 유지한다. DrawMesh 공통화 당시에는 모델 전용 자원 초기화/해제·네 바이트코드 의존·EVertexFormat·생성 오버로드를 남겼다. 같은 날 다음 승인 단계에서 미사용 EVertexFormat·vertexFormat 멤버와 모델 생성 시 대입을 제거했다. 그리기·정점 버퍼 배치는 그대로이며 모델 전용 자원과 생성 오버로드 정리는 아직 남아 있다. 정점/삼각형 인덱스 검사와 성공 후 출력 교체 계약은 기존 CreateMesh와 같다. 당시 GPU 메시의 형식 구분으로 체크/모델 셰이더를 선택했으며, 이 구분은 위의 공통화와 미사용 값 제거로 대체되었다. 당시 모델 입력은 위치만 사용했고 이후 UV 텍스처 연결·공통 색상 곱 경로로 확장했다. 법선·materialIndex는 현재도 그리기에 사용하지 않는다. 모델 GPU 기반 도입 당시에는 ModelVertexShader/ModelPixelShader.hlsl도 Main/Shader Model5.0으로 빌드하고 네 바이트코드가 초기화에 필요했다. 2026-10-18 후속으로 Renderer의 모델 전용 셰이더/입력 레이아웃 읽기·생성·멤버·해제를 제거해 초기화에는 TriangleVertexShader/TrianglePixelShader.cso 두 파일만 요구한다. Renderer 정리 당시에는 모델 셰이더 원본/빌드 등록과 D3D11 전용 입력 레이아웃 함수가 남았다. 같은 날 다음 단계에서 호출되지 않는 CreateModelInputLayout의 선언·설명과 구현을 제거했다. 공통 CreateInputLayout은 그대로 사용한다. 2026-10-18 다음 단계에서 ModelVertexShader.hlsl과 SampleGame.vcxproj/.filters의 해당 FxCompile 등록을 제거했다. 정점 셰이더 제거 당시에는 ModelPixelShader.hlsl과 그 등록을 남겼고, 같은 날 다음 승인 단계에서 이 픽셀 셰이더 파일과 프로젝트/필터 등록도 제거했다. 이제 공통 TriangleVertexShader/TrianglePixelShader만 빌드하며 그리기 코드는 변경하지 않았다. 노드·재질·깊이·모델 컴포넌트/게임 배치는 후속이다.

현재 Renderer 메시 책임(2026-10-17): 기본 사각형/defaultMesh와 SetMesh·인수 없는 DrawMesh·Renderer.RenderFrame을 제거했다. 초기 행렬은 항등값이며 GPU 메시는 호출자/Scene이 소유하고 명시 DrawMesh(data)로 전달한다. 게임 회전 경로는 유지한다. 아래 기본 메시 경로 설명은 이전 단계 이력이다.

현재 모델 파일 읽기(2026-10-17): Engine/Source/Renderer/ModelLoader.h/.cpp의 FModelLoader::LoadModelFile(path,outModel)은 상대 경로를 현재 작업 폴더 기준으로 읽고 Assimp 삼각형화·정점 병합·검증·왼손 변환 후 FModelData를 구성한다. 위치/필수 법선/UV0, 기본 색상/외부 색상 텍스처 하나, 노드 계층과 전치한 행렬을 보관한다. 텍스처 참조는 모델 폴더 기준 절대경로이고 이미지 존재/디코딩은 검사하지 않는다. UV는 비텍스처 메시에서 누락 시0이며 텍스처 메시에서 누락되면 오류다. 파서가 읽은 뒤 뼈/애니메이션/모프/내장 이미지는 거부하고 추가 UV/정점 색상/기타 텍스처·UV 변환/카메라/조명은 경고 후 제외한다. 지역 결과가 완성된 뒤만 출력으로 이동해 일반 실패 시 기존 출력을 보존한다. GPU·Actor 생성/추가 단위 보정/저장은 없다. 메모리 부족·경로 인코딩 예외·외부 파서 내부 접근 오류의 false 복구는 보장하지 않는다. 제품 및 독립 CPU 검증 범위와 Assimp 스킨 한계는 루트 Docs/Handoff·ModelImportDecisions에 기록했다.

현재 게임 회전 샘플(2026-10-17): SampleGame이 ASampleActor를 생성하고 게임 Tick이 로컬 Z축을 초당45도 회전시킨다. 메시 구성은 게임 함수, 변환 전달/그리기는 기존 엔진이 담당한다. 엔진 코드는 변경하지 않았다. 실제 제품 창 회전은 미관찰이며 아래 정적 기본 Actor 설명은 이전 단계 이력이다.

현재 Assimp 의존성(2026-10-17): Engine/ThirdParty/vcpkg.json에 assimp를 추가하고 기존 builtin-baseline eb2d3a3279fd019cb7733072d86900d0ad2a1aef를 유지한다. 선택 버전은 6.0.5, 트리플릿은 기존 x64-windows-static-md다. 기존 MSBuild vcpkg 연결을 사용하고 전역 통합/트리플릿/렌더러는 바꾸지 않는다. Temp의 별도6.0.2 도구를 제품 링크로 사용하지 않는다. Assimp/의존성의 설치 패키지 저작권 원문10개를 Engine/ThirdParty/Assimp/Licenses에 바이트 그대로 보관하고 버전/해시를 index.json에 기록한다. 배포 시 README/고지를 함께 제공하며 자동 패키징은 아직 없다. Assimp 사용 허용과 모델·텍스처 사용/재배포 권리는 별개다. 자세한 조건은 [Assimp 안내](../Engine/ThirdParty/Assimp/README.md)를 따른다. 제품 FModelData 로더·게임 화면 연결은 아직 미구현이다.

현재 SampleGame 콘텐츠(2026-10-17): ConfigureGameWorld가 기본 Actor1개와 메시 컴포넌트를 구성하며 엔진 초기 준비/Scene 경로로 중앙 사각형을 그린다. 게임은 CPU 데이터만 제공한다. 실제 제품 창은 미관찰이며 아래 빈 SampleGame/배경만 설명은 이전 단계 이력이다.

현재 초기 렌더 준비(2026-10-17): World::CreateRenderStates를 구현하고 게임 구성 직후 한 번 호출한다. 기존 상태는 건너뛰고 미생성 메시만 준비한다. 실행 중 추가된 컴포넌트는 자동 등록되지 않는다. 등록 수명 통합 때 초기 함수/호출의 존치를 재검토한다. 아래 코드 미구현 설명은 이전 단계 이력이다.

모델 CPU 데이터 도입 당시(2026-10-17): Engine/Source/Renderer/ModelData.h의 FModelData가 FModelMeshData/FModelMaterialData/FModelNodeData 목록을 값으로 소유했다. 2026-10-18 FModelData::meshes와 ModelLoader::ImportMeshes의 출력 배열/지역 타입을 FStaticMeshData로 전환했다. 위치·색상·법선·UV·인덱스·재질 번호와 재질 목록·노드·기존 로딩 검사는 유지한다. 로더 전환 당시에는 FModelMeshData 정의와 이전 Scene/Renderer 오버로드를 유지했다. 이후 각 API와 정의를 차례로 제거했으며 현재는 공통 FStaticMeshData만 사용한다. 당시 FModelVertex는 위치·법선·UV와 기본값0을 제공했다. 2026-10-18 이 타입을 제거하고 FModelMeshData::vertices를 TArray<FVertex>로 변경했다. FVertex는 위치·색상·법선·UV이며 색상 기본값은 흰색이다. 메시의 uint32 indices는 자체 vertices 참조/삼각형3개이며 int32 materialIndex는 materials 번호/-1 없음이다. 재질은 기본 흰색 RGBA와 std::filesystem::path를 소유하며 빈 경로는 없음, 값이 있으면 로더가 원본 모델 기준 참조를 해석한 절대경로다. 노드는 부모 기준 행 벡터 localTransform(루트는 모델 기준), nodes 번호인 int32 parentIndex(-1 루트), meshes 번호인 uint32 meshIndices를 보관한다. 생성자는 SetIdentity로 변환을 초기화하고 목록은 기본 비어 있다. 계층/범위/파일 검증·파싱·GPU 생성은 수행하지 않는다. 기존 FVertex/FStaticMeshData는 유지한다. 공통 데이터를 바로 사용할 방향이며 별도 CPU 런타임 표현을 필수로 추가하지 않는다. 제품 Assimp 연결·모델 표시·자체 저장/재로드는 미구현이다. 선택·대가·변경 이력은 루트 Docs/ModelImportDecisions.md를 따른다.

초기 렌더 준비 설계 정리(2026-10-16, 코드 미구현): 게임 연결 계약과 렌더 상태 준비 시점은 별개다. 제안한 World::CreateRenderStates는 초기 일괄 호출 방식이며 등록 수명 통합 시 제거/유지 여부를 재검토한다. 구체 범위·실패 계약·이행 조건은 문서 말미의 같은 제목 절을 따른다.

현재 게임 구성 함수 연결(2026-10-16): 엔진 GameEntry.h 선언을 게임 SampleGame.cpp가 구현하고, main이 Renderer 준비 뒤 한 번 호출한다. 실패하면 루프를 시작하지 않으며 World는 Renderer보다 먼저 정리한다. 빈 게임 구현/초기 렌더 상태 준비 미구현이므로 배경 화면을 유지한다. 아래 연결 코드 미구현 설명은 이전 단계 이력이다.

현재 게임 연결 선택(2026-10-16): 단일 실행 프로젝트+게임 구성 함수 방식으로 진행한다. 라이브러리·모듈·리플렉션은 배제하지 않고 구체 요구 때 재검토한다. 연결 코드는 아직 미구현이다. 문서 말미의 「게임 구성 함수 선택과 재검토 기준」 절에 이행 조건과 다음 승인 범위를 기록했다.

현재 제품 실행 경로(2026-10-16): main이 Tick→World 변경 전달→World Scene 그리기를 호출한다. 현재 World는 비어 있어 기본 사각형 대신 배경만 그린다. SampleGame 생성은 후속이며 아래 main 미연결/기본 메시 화면 설명은 이전 단계 이력이다.

현재 World 렌더 변경 전달(2026-10-16): `SendAllEndOfFrameUpdates(renderer)`가 생성된 정적 메시 컴포넌트의 메시 변경과 현재 변환을 순서대로 전달한다. 첫 실패는 부분 성공을 유지한 채 false다. Tick/콜백 밖 명시 호출이며 main 연결은 후속이다.

현재 명시 메시 갱신(2026-10-16): `UpdateRenderState(renderer)`가 dirty인 기존 Scene 메시를 교체하고 성공 후만 표시를 지운다. 실패는 기존 GPU 메시/표시를 보존한다. 변환 전달과 자동 호출은 별도다. 아래 메시 처리 미구현 설명은 이전 이력이다.

현재 메시 변경 표시(2026-10-16): 생성된 `UStaticMeshComponent`의 CPU setter는 렌더 상태를 dirty로 표시한다. 성공 생성/해제는 표시를 초기화한다. 표시만으로 GPU 메시를 갱신하지 않으며 실제 처리/자동 호출은 후속이다. 아래 dirty 미구현 설명은 이전 단계 이력이다.

현재 렌더 변환 전달(2026-10-16): `UStaticMeshComponent::SendRenderTransform()`이 생성된 Scene 등록의 행렬만 명시 갱신한다. 부모 합성은 호출 시 계산하며 GPU 메시/ID는 유지한다. 자동 호출과 메시 변경 업로드는 아직 없다.

현재 StaticMeshComponent 렌더 상태(2026-10-16): `CreateRenderState`가 현재 CPU 메시/월드 행렬을 World의 Scene에 명시 등록하고 `DestroyRenderState`가 해제한다. `IsRenderStateCreated`는 보관 ID 유무를 읽는다. 소멸자도 해제하며 CPU setter·변환 변경의 자동 전달과 엔진의 자동 호출은 아직 없다.

현재 제거와 EndPlay 연결(2026-10-16): World가 대상 묶음의 대기 요청·관계·등록을 정리한 뒤 지역 unique_ptr 소유를 유지하며 자손부터 private RouteEndPlay(reason)→파괴를 실행한다. 시작 완료 대상만 EndPlay를 받으며 시작 전/BeginningPlay 대상은 건너뛴다. 명시/지연 제거는 Destroyed, World 소멸자가 남은 대상을 정리할 때는 Quit다. World 종료 중 다른 Actor를 명시 제거한 요청은 Destroyed다. 종료 중 자신의 계층은 비어 있고 미등록/제거 중이며 컴포넌트와 World Scene은 아직 살아 있다. 자기 또는 이미 분리된 대상의 Destroy 재요청은 false다.

private DestroyActorImmediately에 기본값 Destroyed인 이유 인수를 추가하고 World 소멸자는 같은 실제 제거 함수에 Quit를 전달한다. 종료 콜백의 다른 등록 Actor 제거는 기존 Tick 중 보류/Tick 밖 즉시 규칙을 따른다. 현재 묶음은 World 배열에서 분리된 지역 소유여서 다른 등록 Actor의 변경이 그 순회를 무효화하지 않는다. 기반 EndPlay 호출 누락을 강제로 보정하지 않지만 분리된 객체는 실제 제거 경로에서 한 번만 종료/파괴한다. 기존 Scene·소유·현재 자손 선택·부모 변경·대기 요청 정리·공개 DestroyActor 형태는 유지한다. 아래 자동 종료 미연결/소멸자의 public DestroyActor 호출 설명은 이전 단계 이력이다.

현재 Actor 종료 재정의 지점(2026-10-16): Actor.h에 enum class EEndPlayReason:uint8 { Destroyed, Quit }와 protected virtual EndPlay(reason)를 추가했다. 기반 구현은 HasBegunPlay일 때만 HasNotBegunPlay로 전환하고 시작 전/BeginningPlay는 유지한다. 파생 구현은 상태 복귀를 위해 기반 함수를 호출한다. 이유는 현재 전달 인수이며 제거/전체 종료에서 자동 선택/호출하지 않는다. RouteEndPlay/중복 방지는 후속이다. 명시 호출 뒤에도 Actor가 등록된 채 남으면 기존 World Tick에 의해 다시 시작할 수 있다. 이번 EndPlay는 소유/등록/메모리/컴포넌트를 정리하지 않으며 기존 제거 흐름은 유지한다. 아래 EndPlay 미추가 기록은 이전 단계 이력이다.

현재 World의 Scene 소유(2026-10-16): `UWorld`가 `FScene`을 값 멤버로 소유하고 mutable/const `GetScene()` 참조를 제공한다. Scene을 Actor 소유 배열보다 먼저 선언해 Actor·컴포넌트 정리 뒤 Scene을 파괴한다. 생성 시 GPU 준비와 컴포넌트 자동 등록은 하지 않는다.

현재 첫 Tick 전 Actor 시작 처리(2026-10-16): UWorld::Tick이 등록 순서의 각 대상에 대해 직접 제거 요청을 먼저 검사하고 private DispatchBeginPlay를 호출한다. 시작 전 상태일 때만 BeginningPlay로 바꾸고 가상 BeginPlay를 실행하며 완료 전환은 기반 BeginPlay가 맡는다. 미완료 또는 시작 중 직접 제거 요청이면 Tick을 생략한다. 기반 호출 누락은 BeginningPlay 유지/시작 재호출 없음/Tick 보류다. 기존 friend UWorld를 사용하고 새 friend는 추가하지 않았다.

시작 중에도 기존 Tick 전체의 제거 요청 보류/종료 정리를 적용하므로 자기·조상 제거 요청으로 시작 함수 도중 메모리를 회수하지 않는다. 직접 요청되지 않은 자손은 계속 시작/Tick하고 부모 밖으로 이동할 수 있다. 자손 범위는 실제 처리 시점에 결정한다. 각 Actor는 등록 순서로 BeginPlay 직후 Tick하며 전체 Actor의 BeginPlay를 먼저 모아 실행하거나 부모 우선 순서를 보장하지 않는다. 도중 생성은 다음 World Tick부터 시작하지만 제거되는 부모 아래의 새 자손은 시작 전 파괴될 수 있다. 첫 Tick 전 파괴된 Actor는 BeginPlay를 받지 않는다. 제품 World는 여전히 비어 있고 World 별도 BeginPlay/EndPlay·컴포넌트 시작·게임/렌더 연결은 추가하지 않았다. 아래 Dispatch/자동 시작 미연결 기록은 이전 단계 이력이다.

현재 Scene 메시 교체(2026-10-16): `FScene::SetMeshData(id,renderer,data)`가 기존 ID·행렬·순서를 유지하며 GPU 메시만 교체한다. 실패하면 기존 자원을 보존한다. 명시적 변경 시 호출하며 Component의 CPU setter와 자동 연결은 후속이다.

현재 정적 메시 Scene(2026-10-16): `FScene`이 ID별 GPU 메시/월드 행렬을 소유하고 AddStaticMesh·SetLocalToWorld·RemoveStaticMesh·RenderFrame을 제공한다. ID는 Scene 내부에서 제거 후 재사용하지 않는다. Actor/Component/Renderer 주소와 CPU 원본은 보관하지 않는다. World/게임 자동 연결과 main 변경은 후속이다.

현재 기반 BeginPlay의 완료 상태 설정(2026-10-16): AActor::BeginPlay는 actorHasBegunPlay를 HasBegunPlay로 설정한다. 파생 구현이 기반 함수를 호출하면 그 호출 지점부터 HasActorBegunPlay는 true이며, 기반 호출을 생략하면 초기 상태를 유지한다. 파생 함수 전체가 반환된 시점을 보장하는 값은 아니다. BeginningPlay 설정/Dispatch/자동 호출/중복 방지는 아직 없어 IsActorBeginningPlay는 false다. 기존 Tick/소유/계층/제거 정책은 유지한다. 아래 빈 기반 BeginPlay·두 조회 모두 항상 false 기록은 이전 단계 이력이다.

현재 Actor 시작 상태 저장/조회(2026-10-16): AActor의 private 중첩 EActorBeginPlayState:uint8는 HasNotBegunPlay/BeginningPlay/HasBegunPlay를 구분하고 actorHasBegunPlay의 초기값은 HasNotBegunPlay다. public [[nodiscard]] const 조회 HasActorBegunPlay/IsActorBeginningPlay는 각각 완료/실행 중 상태를 비교해 bool을 반환한다. 외부 상태 변경 API는 없다. 아직 상태 전환/Dispatch/자동 BeginPlay는 없어 두 결과는 모두 false이며 빈 기반 BeginPlay를 명시 호출해도 바뀌지 않는다. 기존 Tick/소유/제거 정책은 유지한다. 아래 시작 상태 미추가 기록은 이전 단계 이력이다.

현재 GPU 메시 소유(2026-10-16): `FStaticMeshRenderData`가 메시별 정점/인덱스 버퍼와 개수를 소유한다. `CreateMesh`로 준비하고 `DrawMesh(mesh)`로 재사용하며 빈 자원/다른 장치를 거부한다. 기존 SetMesh/DrawMesh()/RenderFrame은 내부 기본 메시로 유지한다. World·Component 자동 연결은 후속이다.

현재 Actor 시작 재정의 지점(2026-10-16): AActor에 protected virtual BeginPlay()와 빈 기본 구현을 추가했다. 파생 클래스에서 재정의할 수 있으며 외부 호출용 공개 API는 아니다. 생성/Spawn/World Tick/제거/전체 종료에서 자동 호출하지 않는다. DispatchBeginPlay·시작 상태·한 번 실행 보장·컴포넌트 시작 전파는 아직 없고 기존 Tick/제거 흐름은 유지한다. 아래 BeginPlay 미추가 기록은 당시 단계 이력이다.

현재 실행 루프의 World Tick(2026-10-16): EngineMain.cpp의 main이 렌더러 초기화 뒤 지역 UWorld 하나를 만들고 메시지 처리 → steady_clock 시각 갱신 → 창 크기 확인 → World Tick → 기존 RenderFrame 순서로 실행한다. deltaSeconds는 인접 루프 시각 차의 float 초 값이다. 크기0이면 World Tick/렌더링을 생략하지만 시각은 갱신해 복원 시 최소화 구간 전체를 전달하지 않는다. 종료 메시지면 추가 Tick 없이 빠져나온다. 제품 World는 아직 비어 있고 SampleGame Actor 생성/메시의 화면 연결은 후속이다. 고정 시간 간격/시간 누적/클램프/프레임 제한은 추가하지 않았다. 아래 실행 루프 미연결 기록은 당시 단계 이력이다.

직접 제거 요청된 Actor의 Tick 생략(2026-10-16 최신 정정): UWorld::Tick은 pendingDestroyActors에 직접 등록된 대상의 아직 시작하지 않은 Tick을 건너뛴다. 보류 중 IsActorBeingDestroyed는 false이고 부모 변경은 허용한다. 이미 실행 중인 Tick은 중단하지 않으며, 부모만 요청된 자손은 계속 Tick한다. 실제 처리 시점의 자손 수집/요청 정리와 GetActors 포함 규칙은 유지한다. 아래 보류 중 Tick 허용 기록은 이전 단계의 상태다.

현재 프레임 렌더링(2026-10-16): `BeginFrame`에서 크기 대응/clear, 반복 가능한 `DrawMesh`에서 그리기, `EndFrame`에서 Present한다. `RenderFrame`은 세 함수를 호출해 기존 단일 그림 실행을 유지한다. 활성 프레임 상태로 잘못된 순서를 거부하며 메시/행렬 변경은 다음 DrawMesh에 반영한다.

제거 요청 보류의 구현 정정(2026-10-16): World.cpp에 남아 있던 요청 즉시 상태 표시/표시 검색 코드를 대상 주소 보관/요청 소비로 수정했다. 보류 중 Tick·부모 변경과 처리 시점 자손 결정을 문서대로 구현하고, 실제 파괴 전 관련 대기 요청을 정리한다. 이전 GetActors 단계에서 발견한 구현/문서 불일치는 해결했다.

현재 World Actor 조회(2026-10-16): `GetActors(TArray<AActor*>&) const`가 현재 등록된 모든 Actor의 비소유 주소를 등록 순서대로 채운다. 제거 요청/상태로 필터링하지 않으며 실제 등록 해제 전까지 포함한다. 호출자 배열 용량은 재사용하고 객체 수명은 연장하지 않는다.

## 제거 요청 보류와 처리 시점의 자손 — 2026-10-16 최신 정정

World는 비소유 TArray<AActor*> pendingDestroyActors를 보관한다. Tick/종료 정리 중 DestroyActor는 검증된 요청 대상만 중복 없이 기록하고 true를 반환한다. 보류만 된 상태는 IsActorBeingDestroyed가 false이며 Tick과 부모 변경을 기존 규칙대로 허용한다. 실제 제거 중 자신/새 부모의 관계 변경 거부는 유지한다. 순환/다른 World/미등록 등 기존 실패 정책도 유지한다.

private PurgeDestroyedActors는 요청 목록에서 꺼내 실제 제거 함수에 넘긴다. 자손은 각 요청의 실제 처리 시점에 수집한다. 밖으로 이동한 자손은 자기 요청이 없으면 생존하고, 새로 붙은 자손은 포함된다. 요청 대상 자체의 이동은 요청을 취소하지 않는다. 실제 파괴 전에 함께 제거되는 자손의 대기 요청도 목록에서 지워 오래된 주소가 남지 않게 한다. 현재 뒤에서 소비하지만 요청 간 처리 순서를 공개 보장하지 않는다. 종료 정리가 끝날 때까지 bIsTicking을 유지해 중첩 Tick은 무시하고 소멸 중 새 요청도 이어서 처리한다.

Tick 시작 개수까지 인덱스로 다시 읽는 순회와 Tick 중 생성의 다음 호출 지연을 유지한다. Tick 밖/정상 World 종료는 즉시 파괴한다. 요청 주소는 World 소유 객체에만 유효하며 실제 파괴 뒤 외부 Actor/Component 주소는 무효다. 주소 보관 O(R)·중복 검사 O(R)·파괴 전 요청 정리 O(R*M)의 비용이 추가되고 기존 등록/소유 위치 검색은 별도다. 항상 별도 회수·BeginPlay/Destroyed 알림·제품 루프/시간 처리·동시성/예외 복구는 이번에 추가하지 않았다. 아래 요청 즉시 제거 표시/보류 중 부모 변경 차단 설명은 이전 단계 당시 상태다.

현재 메시 변환 조회(2026-10-16): `UStaticMeshComponent::GetLocalToWorld() const`가 소속 Actor의 기본 Transform에서 부모 합성된 행렬을 값으로 반환한다. CPU 메시 데이터는 로컬로 유지한다. 렌더러로 자동 전달하는 실행 연결은 후속이다.

현재 렌더러 변환 입력(2026-10-16): `SetLocalToWorld`와 `SetWorldToClip`이 객체/뷰 행렬을 별도로 받아 값으로 보관한다. CPU에서 `LocalToWorld * WorldToClip`을 합성해 기존64바이트 `LocalToClip` 상수 버퍼로 전달한다. Actor·카메라의 자동 연결은 아직 없다.

## 조건부 지연 제거와 인덱스 Tick — 2026-10-16 최신 정정

Actor.h/.cpp의 bool bActorIsBeingDestroyed와 [[nodiscard]] bool IsActorBeingDestroyed() const로 제거 처리 시작 상태를 표현한다. AActor::Destroy는 World의 bool을 직접 반환한다. Tick 중 true는 자신/자손 표시와 제거 예약 성공이며 순회 종료 뒤 실제 파괴한다. Tick 밖에서는 기존처럼 즉시 파괴한다. 실제 파괴 뒤 Actor/Component 외부 주소는 무효다.

World의 actorsToTick 멤버/복사/슬롯 무효화를 제거했다. Tick 시작 actors.size()를 저장하고 각 인덱스를 매번 다시 읽으며 제거 중 대상을 건너뛴다. 순회 중에는 소유 배열의 제거/압축이 없고 Spawn은 끝에 추가하므로 재할당에도 기존 Actor 주소는 유지된다. 새로운 인덱스는 다음 Tick부터 호출한다. 같은 World 중첩 Tick은 무시한다.

DestroyActor는 null/미등록/다른 World를 false로 유지한다. Tick 중 이미 표시된 등록 대상은 true이고 추가 표시하지 않는다. 표시 때 자신/자손의 소유/관계는 유지한다. 제거 중 자신이나 새 부모가 포함된 SetActorParent는 같은 부모/분리도 false다. Tick 종료의 private PurgeDestroyedActors는 표시된 대상을 탐색해 private DestroyActorImmediately로 관계/등록 분리·자손부터 실제 파괴한다. 정리가 끝날 때까지 bIsTicking을 유지하므로 소멸 중 추가 제거 요청도 표시 후 이어서 처리한다. 이미 등록에서 분리된 대상의 소멸 중 재요청은 false다. 소멸 중 새 생성은 다음 Tick부터 호출한다.

Actor 상태는 GC 유효성/핸들 검사와 다르다. 기본 Tick은 O(N), 표시용 임시 수집은 O(M)이고 정리는 기존 반복 검색/압축 비용을 감수한다. 항상 별도 회수·BeginPlay/Destroyed 알림·제품 루프/시간 처리·활성 Tick 중 World 파괴/동시성·할당 실패 복구는 이번에 추가하지 않았다. 아래 주소 목록/항상 즉시 제거 관련 설명은 이전 단계 당시 상태다.

현재 정적 메시 컴포넌트(2026-10-16): `UStaticMeshComponent`가 `UActorComponent`를 상속하고 CPU `FStaticMeshData`를 복사해 보관·조회한다. Actor/렌더러의 자동 수집·GPU 업로드·제품 화면 연결은 아직 없다. 아래 컴포넌트 미추가 설명은 해당 단계 당시 기록이다.

컴포넌트 타입 정정(2026-10-16): `FActorComponent`·`FTransformComponent`는 `UActorComponent`·`UTransformComponent`로 변경했고 `UActorComponent`가 `UObject`를 상속한다. 아래 이전 타입 이름은 당시 구현 이력이며 파일 이름·소유·수명 정책은 유지한다.

현재 메시 데이터 이름(2026-10-16): `FMeshData`/`Renderer/MeshData.h`는 `FStaticMeshData`/`Renderer/StaticMeshData.h`로 변경했다. 아래 이전 이름은 단계별 구현 이력이며 현재 API는 `SetMesh(const FStaticMeshData&)`다.

## World의 Actor Tick 순회 — 2026-10-16 후속 갱신

UWorld::Tick(float deltaSeconds)를 제공한다. 시작 시점의 World 등록 순서로 각 Actor를 한 번 호출하고 시간값을 그대로 전달한다. private TArray<AActor*> actorsToTick와 bool bIsTicking을 사용해 목록 복사 후 순회하며, 끝나면 목록을 clear하고 상태를 해제한다. 용량은 재사용한다. 같은 World의 중첩 Tick은 무시하며 다른 World의 진행 상태는 독립적이다. 도중 생성한 Actor는 다음 Tick부터 호출하고 부모 변경은 현재 호출 목록/순서를 바꾸지 않는다.

DestroyActor는 기존 검증을 마친 뒤 실제 파괴 전에 제거 묶음에 속한 호출 항목을 nullptr로 바꾼다. Tick은 nullptr를 건너뛰고 Actor 호출 후 객체를 다시 읽지 않는다. 제거된 주소가 새 Actor에 재사용돼도 이번 호출 목록으로 새 객체를 호출하지 않는다. 실패한 제거는 목록을 유지한다. 자기/부모/자손 제거와 소멸 중 중첩 제거에도 즉시 파괴 계약은 그대로다. 호출 후 제거된 Actor/Component에 접근하면 안 된다.

시작 복사/기본 순회 O(N)·추가 공간 O(N), T개 목록과 M개 제거 대상 비교 O(T*M) 비용을 사용한다. 기존 제거 비용은 별도다. 단일 스레드에서 World가 Tick 동안 살아 있어야 한다. 시간 측정/누적/검사/보정·제품 루프 연결·Component Tick/활성화 상태·활성 Tick 중 World 파괴/예외 복구는 추가하지 않았다. 아래 World 자동 순회 미구현 설명은 당시 상태이며 실행 루프의 매 프레임 호출은 아직 없다.

## Actor Tick 재정의 지점 — 2026-10-16

Actor.h/.cpp에 public virtual void AActor::Tick(float deltaSeconds)를 제공한다. 인수는 호출자가 전달한 초 단위 프레임 경과 시간이며 기본 본문은 비어 있다. cpp 인수에 [[maybe_unused]]를 붙여 이름을 유지하면서 기본 구현의 미사용을 표시한다. 파생 Actor는 override할 수 있으며 기존 AActor 생성/등록/계층/제거 동작은 유지한다. 현재 World/실행 루프의 자동 호출·시간 측정/누적/보정·Component Tick 전파·Tick 활성화/등록 상태는 없다. World 자동 순회는 즉시 파괴와 순회 안전성 계약을 정한 뒤 별도 승인 단계에서 구현한다.

## World 전체 소멸의 동일 제거 경로 — 2026-10-16 후속 갱신

~UWorld는 while (!actors.empty())에서 (void)DestroyActor(actors.back().get())를 호출한다. 정상 등록/계층 불변식 아래 매 호출이 제거를 진행하며 남은 배열을 다시 조회한다. 개별 제거와 같은 등록/관계 정리·자손 우선 파괴를 전체 종료에도 적용한다. 소멸 중 남아 있는 다른 등록 Actor의 Destroy 요청을 허용하고, 이미 등록에서 분리돼 아직 살아 있는 대상은 기존 정책대로 거부한다. 제거 순회 알고리즘/소유 배열/SpawnActor/Actor 코드는 변경하지 않았다.

기존 선형 검색/압축 경로를 반복하므로 전체 정리 비용은 O(N²)일 수 있으며 임시 할당/할당 실패 복구 없음도 유지한다. 서로 다른 루트/형제 파괴 순서를 공개 보장하지 않는다. 종료 중 새 생성/임의 재연결 정책·종료 이벤트·GC·지연 회수는 이번 계약으로 추가하지 않는다. 아래 World 기본 소멸자/전체 소멸 중 변경 미지원 기록은 이전 단계 당시 상태다.

## Actor의 제거 요청 — 2026-10-16 후속 갱신

Actor.h/.cpp에 [[nodiscard]] bool AActor::Destroy()를 제공한다. return GetWorld().DestroyActor(this)로 기존 World 제거 함수에 위임한다. 성공 시 반환 전에 자신과 모든 자손/소유 Component가 파괴되므로 호출 뒤 해당 객체에 접근하면 안 된다. 함수도 World 호출 뒤 자신을 다시 조회하지 않는다. 미등록 Actor는 false이며 기존 상태를 유지한다. 실제 제거/관계/등록/재진입 정책은 기존 UWorld::DestroyActor 계약을 따른다. GC·파괴 대기 상태·지연 파괴·새 생명주기 이벤트·World 구현 변경은 없다. 아래 Actor::Destroy 없음 기록은 이전 단계 당시 상태다.

## World의 자손 포함 즉시 제거 — 2026-10-16 후속 갱신

[[nodiscard]] bool UWorld::DestroyActor(AActor*)를 제공한다. null·미등록·다른 World면 false이고 변경하지 않는다. 인수는 null 또는 살아 있는 객체여야 한다. World는 대상과 모든 자손의 소유 위치/임시 공간/기존 부모 연결을 먼저 준비한다. 기존 부모에서 대상을 분리하고 모든 제거 대상의 parent/children을 비운 뒤 unique_ptr를 지역 배열로 옮겨 등록에서 제거한다. World 배열 정리를 마친 뒤 수집 역순으로 reset해 자손을 부모보다 먼저 즉시 파괴한다. 형제 파괴 순서는 공개 보장하지 않는다.

명시적 DestroyActor 도중 파생 소멸자가 다른 등록 대상을 제거/재연결하거나 새 Actor를 생성할 수 있다. 이미 지역 배열로 분리돼 아직 살아 있는 제거 대상은 미등록으로 거부한다. 제거된 외부 Actor/Component 포인터는 무효이며 삭제된 포인터 재요청의 안전성을 보장하지 않는다. 소멸자에서 보이는 자신의 계층은 비어 있고 로컬 값은 그대로다. 대상별 선형 검색 O(N*M), 전체 압축 O(N), 임시 공간 O(M)을 사용한다. 별도 해시맵·종료 이벤트·PendingDestroy·GC·지연 파괴·할당 실패 복구·Actor::Destroy는 없다. World 자체 소멸 중 목록 변경은 지원 범위가 아니다. 기존 World 기본 소멸자는 유지한다. 아래 개별 제거 미구현 설명은 당시 상태다.

## 부모 변경의 KeepLocal/KeepWorld — 2026-10-16 후속 갱신

Actor.h에 enum class EActorTransformRule : uint8 { KeepLocal, KeepWorld }를 둔다. AActor::SetParent(AActor*,EActorTransformRule)와 UWorld::SetActorParent(AActor*,AActor*,EActorTransformRule)의 기본 규칙은 KeepLocal이다. Actor는 규칙을 World에 위임한다. 기존 호출의 기본 동작은 유지하지만 함수 포인터 형식은 규칙 인수가 추가된 새 서명을 사용한다.

World는 등록/동일 World/순환 검사·같은 부모 반환·기존 자식 위치 확인·새 부모 용량 확보 후 KeepWorld에서만 변경 전 기본 컴포넌트의 월드 값을 읽는다. 양쪽 목록과 parent를 갱신한 뒤 SetComponentTransform(previousWorld)으로 새 로컬을 저장한다. nullptr 분리도 같은 규칙을 적용한다. KeepLocal은 로컬을 읽거나 쓰지 않는다. 같은 부모는 재계산 없이 true, 일반 false는 기존 관계/소유/로컬을 유지한다. true는 관계 처리 성공이며 GetRelativeTransform의 표현 한계·대체 계산 때문에 월드 값의 정확 재현을 보장하지 않는다. 별도 재합성 거부·이벤트·할당 실패 복구는 없다. 아래 KeepWorld 미구현 설명은 당시 상태다. Math/TransformComponent·렌더러·게임 연결·개별 제거는 이번에 변경하지 않았다.

## 월드 Transform 설정 — 2026-10-16 후속 갱신

FTransformComponent::SetComponentTransform(const FTransform&)을 추가했다. 부모가 없으면 SetTransform으로 입력을 복사하고, 있으면 newTransform.GetRelativeTransform(parentTransform->GetComponentTransform()) 결과를 SetTransform으로 저장한다. 기존 SetTransform은 로컬 값 직접 복사다. 입력·부모·계층은 변경하지 않고 자신에 계산된 로컬 값을 저장한다. GetRelativeTransform의 유한 값/단위 회전 전제·안전 역수·음수 크기 경로·기준 회전 검사 대체·shear 한계를 따른다. 요청한 월드 값의 정확 재현이나 별도 거부/성공 신호를 보장하지 않는다. 시간/호출 스택은 부모 월드 조회에 따라 O(H)다. Actor/World/Math 소스는 변경하지 않았고 KeepWorld 부모 변경·렌더러 연결은 미구현이다. 아래 월드 setter 미구현 기록은 이전 단계 당시 상태다.

## Actor 소유·계층 설계 갱신 — 2026-10-16, 비소유 배열·연결/분리 완료

채택한 다음 구조는 World가 모든 등록 Actor를 TArray<TUniquePtr<AActor>> 하나로 단독 소유하고, Actor는 비소유 parent와 TArray<AActor*> children을 보관하며, World가 등록 검사·부모 변경·자손 제거 정책을 통제하는 방식이다. Actor의 관계 변경 API는 World에 위임하고, World의 private 관계 갱신을 위해 Actor에 friend class UWorld를 허용하는 방향이다. Actor 자신의 Component 소유는 유지한다. 별도 Entry/계층 해시맵은 도입하지 않고 등록 검색이 병목이면 인덱스 해시맵을 검토한다.

위 설계 중 **비소유 배열·World의 연결/분리·월드 Transform 조회를 구현했다.** 현재 children은 TArray<AActor*>이며 Actor의 friend로 UWorld를 허용한다. Actor::SetParent가 World::SetActorParent에 위임하고 등록 검사는 그 안에서 직접 수행한다. 별도 공개 등록 조회·개별 제거 API는 아직 없다. SpawnActor 결과는 부모 없이 등록되고 이후 관계를 연결/분리할 수 있다. KeepWorld/제거 등 후속 API는 별도 제안·승인 대상이다.

FTransformComponent::GetTransform() const는 저장된 부모 기준 로컬 값의 복사본이다. [[nodiscard]] FTransform GetComponentTransform() const는 부모가 없으면 저장 값을, 있으면 로컬 값 * 부모 기본 TransformComponent의 월드 값을 반환한다. Actor 생성자의 기본 컴포넌트 보유와 World의 순환 없는 계층을 전제로 한다. cpp에서 Actor.h를 포함한다. 매 조회 재귀 계산으로 시간/호출 스택 O(H)이며 캐시·값 변경은 없다. 기존 FTransform 합성의 유한 값/정규화 회전 전제·shear 표현 한계를 유지한다. 현재 SetParent는 로컬을 유지하므로 월드 값은 바뀔 수 있다. 월드 setter·별도 행렬 getter·KeepWorld·렌더러 연결은 추가하지 않았다. 아래 공간 계산 미구현 기록은 각 단계 당시 상태다.

기존 explicit AActor(UWorld&)·GetWorld()의 비소유 연결, SpawnActor의 UWorld& 전달·비소유 T& 반환, Component 단독 소유와 FTransform 합성/상대 변환 계약은 유지한다. 공개 생성자로 직접 만든 Actor는 자동 등록되지 않는다. 새 구조에서도 자식 배열의 관리 공간/할당 비용은 남으며 메모리·성능 우위를 측정한 것은 아니다.

부모 제거 시 자손도 제거하는 방향은 유지하지만 새 구조에서는 World가 이를 명시적으로 처리해야 한다. 삭제 순서·지연 회수·이벤트·형제 순서·Swap Remove·핸들/세대·추가 변환 규칙 API는 아직 확정/구현하지 않는다. 현재 부모 변경 서명과 계약은 아래 완료 절을 따른다. 재합성 차이 또는 특이 기준의 기존 대체 계산만으로 KeepWorld를 거부하는 새 정책도 도입하지 않는다. 아래의 개별 완료 절은 당시 구현 이력이며 이 갱신을 코드 완료로 확대하지 않는다.

## 현재 구성 — 2026-10-15 재시작

현재는 SampleGame/Source/SampleGame.vcxproj 하나로 SampleGame.exe를 만드는 구성이다. 아래의 이전 구성 기록은 현재 프로젝트·모듈·렌더링 기능의 상태를 나타내지 않는다.

| 항목 | 현재 규칙 |
|---|---|
| 솔루션 | 루트 SeonEngine.slnx는 SampleGame/Source/SampleGame.vcxproj 하나를 참조 |
| 프로젝트 | 실행 파일, 콘솔 서브시스템, x64 Debug/Release |
| 도구 | Visual Studio 2026, MSVC v145, Windows SDK 10.0 계열 |
| 소스 | Engine/Source/EngineMain.cpp, Engine/Source/D3D11/D3D11Device.h/.cpp, Engine/Source/Object.h·World.h/.cpp, Engine/Source/Windows/WindowsApplication.h/.cpp·WindowsHWrapper.h, Engine/Source/Actor.h/.cpp·ActorComponent.h/.cpp·TransformComponent.h/.cpp, Containers/Array.h·Templates/UniquePtr.h·Math/Matrix4.h/.cpp·Math/Quat.h/.cpp·Math/Rotator.h/.cpp·Math/Transform.h/.cpp·Math/Vector3.h·CoreTypes.h |
| 진입점 | main → 장치·창·스왑 체인 준비 → 메시지 처리·대기 → 역순 정리. 생성·조회·표시 → 메시지 펌프·대기 → 창·클래스 정리. 정상 종료 코드는 WM_QUIT에서 받고 API 실패는 1 |
| 엔진 include | $(SERootDir)Engine/Source를 등록하고 기존 include 경로를 상속. EngineMain.cpp는 Windows/WindowsApplication.h를 포함 |
| 파일 등록 | 엔진 EngineMain.cpp와 WindowsApplication·World·Actor·ActorComponent·TransformComponent의 cpp/header 및 WindowsHWrapper.h·Containers/Array.h·Templates/UniquePtr.h·Math/Matrix4.h/.cpp·Math/Quat.h/.cpp·Math/Rotator.h/.cpp·Math/Transform.h/.cpp·Math/Vector3.h·CoreTypes.h를 명시 등록. 엔진 파일은 ../../Engine/Source 기준. filters는 표시 폴더만 관리. 2026-10-18부터 생성기가 게임·엔진 소스와 엔진 셰이더를 검색해 목록/필터를 생성함 |
| 엔진·게임 경계 | 같은 프로젝트에 빌드하되 게임 → 엔진 방향. 엔진에서 구체 게임 코드를 참조하지 않음 |
| 공통 설정 | Directory.Build.props/targets를 유지. 별도의 SeonEngine.props는 가져오지 않음 |
| Windows·D3D11 링크 | 실행 프로젝트의 AdditionalDependencies에 dxgi.lib·d3d11.lib·user32.lib, 기존 링크 의존성 상속 |
| 외부 라이브러리 | 기존 vcpkg manifest의 DirectXMath·ImGui 유지. DirectXMath는 Math/Rotator.cpp·Matrix4.cpp·Transform.cpp에서 사용하며 ImGui·UI 연결은 아직 없음 |
| CRT | Debug /MDd, Release /MD. vcpkg 라이브러리는 x64-windows-static-md |
| 최적화 | Debug 비활성, Release MaxSpeed. 두 구성 모두 디버그 정보 생성 |
| 출력 | Binaries/x64/Debug 또는 Release의 SampleGame.exe |
| 중간 파일 | Intermediate/SampleGame/x64/구성/ |
| 현재 제외 | 별도 Engine.lib, 엔진 소비 props, 게임 프레임 갱신·셰이더·화면 렌더링 |

WindowsApplication.cpp는 WindowsHWrapper.h를 통해 Windows.h를 포함하고 내부 WindowProc을 정의한다. 래퍼는 WIN32_LEAN_AND_MEAN과 NOMINMAX만 조건부 정의하고 Windows.h를 읽는다. 프로젝트·필터에 Engine/Source/Windows/WindowsHWrapper.h를 등록했다. 이전 엔진의 TCHAR 치환과 CreateWindow/IsMinimized 매크로 해제는 도입하지 않았다.

WM_DESTROY는 PostQuitMessage(0), 그 외는 DefWindowProcW에 위임한다. GetModuleHandleW로 모듈을 조회하고 WNDCLASSEXW의 lpfnWndProc에 WindowProc을 연결해 RegisterClassExW로 등록한다. 이 클래스의 창을 생성·조회·파괴한 뒤 UnregisterClassW로 해제한다. cbSize=sizeof(windowClass), CS_HREDRAW/CS_VREDRAW, LoadCursorW(nullptr, IDC_ARROW), COLOR_WINDOW+1로 클래스 설정을 준비한다. 시스템 공유 커서는 직접 파괴하지 않으며 LoadCursorW 실패도 오류 출력 후 1을 반환한다.

창 스타일은 WS_OVERLAPPEDWINDOW, 확장 스타일 0, 메뉴 없음이다. RECT{0,0,1280,720}을 AdjustWindowRectEx로 보정하고 right-left/bottom-top을 콘솔에 출력한다. 계산 실패 시 오류를 보존·출력한 뒤 등록 클래스 해제를 시도하고 해제 오류도 별도로 출력한다. 성공 시에는 크기 출력 뒤 창 생성·표시·메시지 반복과 정리로 이어진다.

계산한 스타일과 크기로 CreateWindowExW를 호출한다. 제목은 SeonEngine, 부모·메뉴·마지막 인자는 nullptr다. GetClientRect 성공 뒤 ShowWindow(SW_SHOW)로 표시한다. cpp 내부 PumpMessages(int& outExitCode)는 PeekMessageW(PM_REMOVE)로 메시지를 꺼내고 WM_QUIT에서 false와 종료 코드를 전달한다. TranslateMessage/DispatchMessageW 후 큐가 비면 true를 반환한다. 바깥 반복은 WaitMessage로 기다리며 대기 실패를 오류·1로 처리한다.

기본 창 처리의 닫기 요청은 창 파괴와 WM_DESTROY의 종료 메시지로 이어진다. 반복 종료 후 IsWindow로 남은 창만 파괴하고 클래스를 해제한다. 생성·조회 실패 정리는 유지한다. UpdateWindow·프레임 갱신·렌더링은 추가하지 않았다. SDK 타입과 user32.lib 설정도 유지한다. 제품 빌드는 통과했지만 현재 제품 창 표시·닫기는 도구 앱 승인 문제로 미검증이며 메시지 큐 독립 검사 결과와 구분한다.

언어 표준 C++20, UTF-8, 표준 준수 전처리기, /W4·/WX, 예외 비활성·RTTI 활성 및 기존 매크로는 공통 설정을 그대로 따른다. 기존 엔진 매크로의 필요성 정리는 별도 검토 대상이다. cpp 서식은 루트 .clang-format, 파일 인코딩·줄바꿈은 .editorconfig와 .gitattributes를 따른다.

빌드는 저장소 루트에서 MSBuild SeonEngine.slnx /p:Configuration=Debug /p:Platform=x64로 수행하며 Release도 같은 방식이다. vcpkg 복원을 위해 네트워크와 캐시 쓰기 권한이 필요할 수 있다. 프로그램은 크기를 출력하고 창을 표시한 뒤 종료 메시지까지 기다린다. 현재 두 구성의 빌드와 독립 메시지 큐 검사는 통과했으며 제품 창 표시·닫기와 정상 종료는 Computer Use 앱 사용 미승인으로 미검증이다.

AActor는 TArray<TUniquePtr<FActorComponent>>로 컴포넌트를 단독 소유한다. AddComponent는 기반 타입의 스마트 포인터를 lvalue 참조로 받아 소속이 자신이면 목록으로 이전하고 true를 반환한다. 성공하면 입력은 비워지며 빈 포인터·다른 소속이면 false와 함께 입력·기존 목록을 유지한다. 할당 실패는 false 복구 대상이 아니다. UObject를 상속하는 가상 소멸자가 보관한 컴포넌트를 정리하며 Actor는 복사·이동을 금지한다. GetComponent<T>() const는 FActorComponent 자신 또는 public 파생 타입으로 변환 가능한 첫 컴포넌트를 추가 순서대로 찾아 비소유 T*로 반환하며 없으면 nullptr다. RTTI의 포인터 dynamic_cast를 사용하고 const Actor에서도 수정 가능한 컴포넌트를 반환한다. 반환 주소는 컴포넌트 파괴 시 무효다. 기반 포인터를 통한 파생 Actor 소멸은 지원하며 제거 API는 아직 없다. 같은 실제 타입의 중복은 추가 객체의 AllowsMultipleComponents 정책에 따라 검사한다. FActorComponent는 비소유 AActor 참조와 가상 소멸자를 유지하며 Actor보다 오래 살아서는 안 된다. 게임 진입점은 아직 두 타입을 생성하지 않는다. TArray·TUniquePtr는 각각 std::vector·std::unique_ptr의 최소 별칭이고 MakeUnique는 생성 인수를 std::make_unique로 전달한다.

Math/Vector3.h의 FVector3는 x·y·z를 float로 보관하며 각 기본값은 0.0f다. 별도 생성자·연산·좌표 변환·DirectXMath 의존은 없다. 좌표축·회전 순서는 아래 2026-10-16 결정대로 사용하며 위치 단위는 미정이고 FRotator 각도 입력은 도 단위다. 타입 정의 당시에는 이 규칙이 없었고 Vector3 축 주석은 2026-10-16에 반영했다.

Math/Quat.h의 FQuat는 float x·y·z·w를 보관하며 기본값 (0, 0, 0, 1)은 회전 없음이다. 각 성분은 축별 각도가 아니다. FRotator 전방 선언과 [[nodiscard]] FRotator ToRotator() const가 있으며 Math/Quat.cpp에서 구현한다. 유한하고 정규화된 입력에서 왼손·고정 축 Z→X→Y 각도를 추출해 도 단위로 반환한다. pitch는 [-90,90], yaw·roll은 [-180,180]도 범위의 대표값이다. cosPitch가 1e-6 이하이면 roll=0인 표현을 고른다. 원본은 유지하며 입력 검사·정규화·별도 생성자는 추가하지 않는다. cpp는 Math/Rotator.h를 포함하므로 호출자는 반환 값을 사용할 때 해당 타입 정의를 포함한다. [[nodiscard]] bool IsNormalized() const는 float 길이 제곱과 1의 차이가 0.01f 이하인지 반환한다. 값을 수정하거나 정규화하지 않으며 기존 setter·변환 함수에서 자동 호출하지 않는다. true는 허용오차 내의 근사 판정이고 float 반올림 영향을 받는다. 임의로 지정한 네 성분을 자동 검사·보정하지 않는다. FQuat는 FTransform의 회전 필드로 사용한다. FTransformComponent가 FTransform을 보관하며 Actor 생성자가 기본 컴포넌트를 만들어 소유한다.

Engine/Source/CoreTypes.h는 Windows/MSVC의 기본 정수 타입에 int8·int16·int32·int64 및 uint8·uint16·uint32·uint64 별칭을 직접 연결하고 sizeof의 1·2·4·8바이트 크기를 static_assert로 확인한다. float·double은 그대로 사용한다. 문자 별칭·플랫폼 타입 계층은 추가하지 않으며 기존 main·SDK 타입·수학 데이터의 선언도 이번에 변경하지 않았다.

Math/Transform.h의 FTransform은 FVector3 position{}, FQuat rotation{}, FVector3 scale{1.0f, 1.0f, 1.0f}를 보관한다. 기본값은 위치 0·회전 없음·크기 1배다. [[nodiscard]] bool IsRotationNormalized() const는 rotation.IsNormalized()에 위임해 길이 제곱과 1의 차이가 0.01f 이하인지 반환한다. 위치·크기를 검사하지 않고 모든 필드는 유지하며 setter·변환 함수에서 자동 호출하지 않는다. Math/Transform.cpp의 [[nodiscard]] FMatrix4 ToMatrixWithScale() const는 유한한 위치·크기와 정규화된 유한 회전을 전제로 원점 기준 크기 → 회전 → 이동 행렬을 반환하고 원본은 유지한다. cpp에서 XMMatrixAffineTransformation·XMStoreFloat4x4를 사용하고 결과 16개 성분을 FMatrix4로 복사한다. 입력 검사·정규화는 없다. 헤더는 Math/Matrix4.h를 직접 포함하고 cpp를 프로젝트·기존 Math 필터에 등록했다. 기본 FTransform의 행렬은 항등이며 FMatrix4 자체의 영행렬 기본값은 유지한다. [[nodiscard]] FMatrix4 ToMatrixNoScale() const는 계산용 크기를 (1,1,1,0)으로 고정해 회전 → 이동 행렬을 반환한다. 유한한 위치와 정규화된 유한 회전이 전제이며 저장된 scale은 사용·수정하지 않는다. 기본 결과는 항등이고 입력 검사·보정은 없다. 부모 관계·역변환·렌더러 연결은 없으며 좌표축·회전 순서는 아래 2026-10-16 결정대로 사용하며 위치 단위는 미정이고 FRotator 각도 입력은 도 단위다. FTransformComponent의 데이터 보관에 사용하며 Actor 생성자가 기본 컴포넌트를 만들어 소유한다.

Engine/Source/TransformComponent.h/.cpp의 FTransformComponent는 FActorComponent를 public 상속하고 FTransform transform{}을 보관한다. 생성자는 소속 AActor&를 기반 생성자로 전달한다. GetTransform() const는 복사본을 반환하고 SetTransform(const FTransform&)은 값을 복사한다. GetPosition/GetQuaternion/GetScale은 각 필드의 복사본을 반환하고 SetPosition(FVector3)/SetQuaternion(FQuat)/SetScale(FVector3)은 해당 필드만 복사해 저장한다. 기존 데이터 접근 함수는 각도 변환·정규화·좌표 변환·값 검사를 수행하지 않는다. SetRotation(FRotator newRotation)은 유한한 도 단위 입력을 ToQuat()으로 변환한 뒤 SetQuaternion에 전달해 회전 필드만 변경한다. 위치·크기·입력 객체는 유지한다. GetRotation() const는 GetQuaternion().ToRotator()를 호출해 도 단위 대표 각도의 독립된 FRotator 값을 반환한다. 유한하고 정규화된 저장 회전을 전제로 하며 조회 시 전체 데이터는 유지하고 검사·보정하지 않는다. 각도 범위와 특이점 정책은 ToRotator를 따른다. TransformComponent.h는 Math/Rotator.h를 직접 포함한다. 생성만으로 Actor 목록에 등록되지 않으며 기존 AddComponent/GetComponent로 명시적 보관·조회가 가능하다. 컴포넌트 생성자 자체의 자동 등록·중복 방지·공간 변환·검사·정규화는 없다. AActor 생성자가 별도로 기본 FTransformComponent 하나를 만들어 소유한다.

## 창 설정 전달 — 2026-10-15 정정

위의 고정 제목·크기 설명은 기본값을 뜻한다. WindowsApplication.h의 FWindowDescription은 std::wstring title=L"SeonEngine", int width=1280, int height=720을 제공한다. EngineMain.cpp가 기본 설정 객체를 만들고 RunWindowsApplication(const FWindowDescription&)에 전달한다. 크기는 요청 클라이언트 영역의 유효한 양수 픽셀이며 함수가 RECT로 보정한다. 제목은 c_str()로 CreateWindowExW에 전달한다. 설정은 호출 중에만 읽고 보관하지 않는다. 등록·표시·메시지·정리와 빌드 설정은 유지한다. 별도 엔진 문자열 타입이 없는 현재 단계에서 이 Windows 실행 설정에 한해 std::wstring 직접 사용을 허용한다. 입력 검증·DPI 정책·추가 창 옵션은 아직 없다.

AActor 생성자는 기본 FTransformComponent 하나를 소유 목록의 첫 항목으로 만든다. 생성 직후 GetComponent<FTransformComponent>()로 조회할 수 있다. 같은 실제 기본 Transform은 일반 중복 금지 규칙으로 거부된다. 서로 다른 파생 Transform은 별개 타입으로 추가 가능하며 기본 Transform을 대체하지 않는다. Transform 계열 전체를 정확히 하나로 제한하는 정책은 아니다.

## 창 클래스 등록 함수 분리 — 2026-10-15 정정

WindowsApplication.cpp의 내부 RegisterWindowClass(HINSTANCE, const wchar_t*)가 WNDCLASSEXW 설정·커서 조회·RegisterClassExW와 해당 실패 로그를 담당한다. 성공 true/실패 false이며 RunWindowsApplication은 실패를 종료 코드 1로 전달한다. UnregisterClassW와 창 생성·메시지·정리는 실행 함수에 남긴다. 공개 API·프로젝트·빌드 옵션과 기존 API 호출 내용은 유지한다.

FActorComponent의 virtual bool AllowsMultipleComponents() const는 같은 실제 타입의 복수 추가 허용 정책을 조회하며 기본 구현은 false다. 필요한 파생 타입만 타입별 고정값 true를 재정의하고 재정의하지 않은 자손은 상위 정책을 물려받는다. 상속 계열 전체를 하나의 제한 그룹으로 묶지 않는다. AddComponent는 빈 포인터·소속 검사 뒤 이 가상 함수를 호출한다. false일 때 typeid(*객체)로 기존 목록의 같은 실제 타입이 있는지 검사해 거부하고, true일 때는 복수를 허용한다. 거부 시 호출자 포인터와 기존 목록은 유지된다.

## 엔진 진입점 소유 — 2026-10-15 정정

표준 main은 Engine/Source/EngineMain.cpp에 있다. SampleGame 실행 프로젝트가 이 소스를 ../../Engine/Source/EngineMain.cpp로 컴파일하고 filters는 Engine/Source에 표시한다. 기존 게임 Main.cpp 등록은 제거했다. 게임 개발자는 게임 코드에서 콘텐츠·규칙을 작성하고 엔진 실행 진입점·플랫폼 처리·종료 순서를 수정하지 않는 책임 경계를 따른다. 현재 게임 소스 폴더가 비어 있어도 실행 프로젝트는 엔진 소스로 빌드된다. 게임별 설정·게임 생성·갱신 연결 API는 후속이며 현재 기본 설정과 실행 동작은 동일하다. 별도 Engine.lib·프로젝트 분리·운영체제 파일 접근 제한은 이번에 도입하지 않았다.

AActor::GetComponents<T>(TArray<T*>& outComponents) const는 FActorComponent 자신 또는 public 파생 타입만 허용한다. 출력 배열을 비운 뒤 자기 목록에서 T와 파생 타입의 모든 객체를 추가 순서대로 채운다. 일치가 없으면 빈 배열이며 확보된 공간은 재사용하고 부족하면 늘어난다. 반환 포인터는 비소유·수명 연장 없음·컴포넌트 파괴 시 무효이며 const Actor에서도 수정 가능한 객체를 가리킨다. 출력 배열 변경은 Actor 소유 목록을 변경하지 않는다.

좌표·회전 규칙(2026-10-16 결정): +X 오른쪽·+Y 위·+Z 앞의 왼손 좌표계를 사용한다. 양의 회전은 해당 양의 축에 대한 왼손 법칙이며 각도 합성은 고정 기준 축 Z(roll)→X(pitch)→Y(yaw) 순서다. 기준 축은 회전을 표현하는 공간의 처음 축이고 자동으로 월드 축을 뜻하지 않는다. 내부 회전 저장은 FQuat를 유지한다. 위치 단위·행렬 및 렌더링 구현은 미정이며 FRotator 사용자 각도는 도 단위를 선택했다. 기존 수학 타입의 데이터와 접근 함수는 그대로이며 Vector3 축 주석은 확정 방향으로 정합화했고 FRotator 도 단위 데이터도 추가했다. 승인된 변환 함수 ToQuat은 아래 계약을 따른다.

Math/Rotator.h의 FRotator는 float pitch(X)·yaw(Y)·roll(Z)을 도 단위로 보관하며 기본값은 모두 0이다. 기존 엔진과 같은 도 입력·계산 시 라디안 변환 방향을 선택했지만 좌표계·회전 순서는 새 왼손·고정 축 Z→X→Y를 따른다. [[nodiscard]] FQuat ToQuat() const는 유한한 도 단위 입력을 라디안으로 바꿔 XMQuaternionRotationRollPitchYaw로 계산하고 정규화된 FQuat 값을 반환한다. 원본은 유지하며 왼손·고정 축 Z→X→Y 순서다. DirectXMath.h는 Rotator.cpp에서만 포함하고 공개 헤더는 Math/Quat.h를 포함한다. NaN·무한대 검사·각도 보정은 없으며 역변환은 FQuat::ToRotator()가 담당한다. 컴포넌트 SetRotation(FRotator)은 이 ToQuat 변환을 사용한다. 위치 단위는 계속 미정이다.

## UObject와 Actor 기반 — 2026-10-16 정정

사용처가 없는 UGameInstance를 보류하고 GameInstance.h·프로젝트/필터 등록을 제거했다. Object.h의 가상 소멸자를 가진 UObject는 유지한다. Actor.h는 Object.h를 포함하고 AActor : public UObject 및 ~AActor() override를 제공한다. Actor.cpp의 소멸자 정의·컴포넌트 소유/추가/조회·복사/이동 금지와 FActorComponent는 유지한다. AActor 이름은 Actor 계열 규칙에 따라 유지한다. GC·리플렉션·객체 표·생성 API·게임 인스턴스/수명 연결은 없다.

Math/Matrix4.h/.cpp의 FMatrix4는 float m[4][4]를 행 우선으로 보관한다. m[행][열]의 인덱스는 0부터 시작하며 기본값은 모두 0인 영행렬이다. SetIdentity()는 전체를 FMatrix4{}로 초기화한 뒤 네 대각 성분을 1로 설정한다. 이전 자체 엔진과 같은 초기화 방식으로 2026-10-16에 정정했다. header/cpp를 실행 프로젝트와 기존 Math 필터에 등록했다. [[nodiscard]] FMatrix4 operator*(const FMatrix4& other) const는 두 원본을 유지하고 곱을 값으로 반환한다. cpp에서만 DirectXMath의 XMLoadFloat4x4·XMMatrixMultiply·XMStoreFloat4x4를 사용한다. 별도로 채택한 행 벡터 규칙에서 A*B는 A 다음 B를 적용한다. 공개 헤더의 배열·영행렬 기본값과 기존 수학 타입은 유지한다. [[nodiscard]] FMatrix4 Inverse() const는 cpp의 XMMatrixInverse로 유한한 입력의 역행렬을 반환하고 원본은 유지한다. 계산 행렬식이 정확히 0·비유한이거나 결과 성분이 비유한이면 항등행렬로 대체한다. 대체값은 실제 역행렬·성공 여부를 의미하지 않는다. 별도 작은 행렬식 임계값·입력 보정은 없다. GPU 전달은 없으며 FTransform의 행렬 생성은 ToMatrixWithScale/ToMatrixNoScale에서 담당한다. 셰이더 배치는 별도다.

## WindowsApplication 수명 분리 — 2026-10-16 정정

WindowsApplication.h/.cpp에 FWindowsApplication을 두고 instance·window·bClassRegistered를 보관한다. Initialize는 준비·생성·표시와 실패 시 부분 정리 시도, PumpMessages는 스레드 큐 처리, Shutdown은 남은 자원 정리와 성공 여부 반환이다. RunWindowsApplication은 지역 객체로 이들을 조합하고 WaitMessage를 반복 쪽에 유지한다. 복사/이동은 불가하고 Shutdown은 명시적으로 호출한다. 기본 소멸자의 자동 정리는 없다. 빈 Shutdown 반복은 허용하지만 전체 재시작·다중 창/스레드 계약은 없다. SDK 핸들 멤버를 위해 WindowsApplication.h도 WindowsHWrapper.h를 포함한다. 이전 cpp-only·함수 지역 핸들 설명은 이 절로 정정한다. 공개 실행 함수·프로젝트·옵션·기존 창 스타일과 크기는 유지한다.

## D3D11 장치·즉시 컨텍스트 — 2026-10-16

Engine/Source/D3D11/D3D11Device.h/.cpp의 FD3D11Device를 프로젝트에 등록하고 Engine/Source/D3D11 필터를 추가했다. AdditionalDependencies에 d3d11.lib를 추가하며 기존 user32.lib·상속은 유지한다. 헤더는 WindowsHWrapper.h·d3d11.h·wrl/client.h를 사용한다. Initialize는 기본 어댑터의 하드웨어 feature level 11_0을 flags 0으로 요청한다. 생성 실패 HRESULT는 D3D11 카테고리 오류 로그와 false, main은 1로 전달한다. 현재 장치 준비 중복도 false로 거부하며 기존 자원은 보존한다. Shutdown은 context/device ComPtr를 순서대로 Reset하며 소멸자 자동 참조 해제도 가능하다. 복사/이동은 제공하지 않는다.

EngineMain.cpp가 장치를 준비한 뒤 기존 Windows 실행을 호출하고 결과 반환 전에 정리한다. 스왑 체인·창 출력·프레임 갱신·셰이더·렌더링은 아직 없다. SDK의 WRL ComPtr를 직접 사용하며 별도 엔진 COM 래퍼는 만들지 않는다. Debug 구성도 flags 0이므로 D3D 디버그 레이어가 켜졌다는 뜻이 아니다. WARP 자동 전환·어댑터 선택은 없다. 공통 props/targets·의존성 manifest·Windows 소스는 변경하지 않는다.

## D3D11 스왑 체인과 엔진 실행 순서 — 2026-10-16 정정

D3D11/D3D11SwapChain.h/.cpp를 실행 프로젝트·기존 D3D11 필터에 등록하고 dxgi.lib를 추가했다. FD3D11SwapChain은 bool Initialize(ID3D11Device*, HWND)/void Shutdown()을 제공한다. 장치의 DXGI 어댑터에서 IDXGIFactory2를 얻고 CreateSwapChainForHwnd를 사용한다. 창 모드·RGBA8·버퍼 2개·MSAA 없음·FLIP_DISCARD·STRETCH·알파 무시이며 Width/Height=0으로 HWND의 클라이언트 크기를 따른다. Windows 10 이상 범위의 flip discard를 선택했다. DXGI 자동 Alt+Enter 전환은 비활성화한다. resize·전체 화면·렌더 타깃·Present는 없다.

FD3D11Device::GetDevice()와 FWindowsApplication::GetWindowHandle()은 비소유 값을 반환한다. EngineMain.cpp에서 장치 → 창 → 스왑 체인 초기화와 펌프/기존 WaitMessage 반복, 스왑 체인 → 창 → 장치 정리를 조합한다. RunWindowsApplication 선언/정의는 제거했다. 단계 실패 시 앞서 준비한 자원을 역순으로 정리하고 1을 반환한다. 중복 스왑 체인 초기화·null 입력·HRESULT 실패는 D3D11 로그/false이며 부분 COM 결과는 지역 ComPtr로 정리한다. 이전 RunWindowsApplication·스왑 체인 미구현 설명은 이 절로 정정한다. 장치 실제 생성과 창 없는 DXGI 팩터리 확인은 통과했지만 실제 HWND 스왑 체인 생성/닫기는 Computer Use 앱 미승인으로 미검증이다.

## Transform 합성 계약 — 2026-10-16 채택, 음수 크기 지원

Math/Transform.h/.cpp의 [[nodiscard]] FTransform operator*(const FTransform& other) const는 A 다음 B의 성분 합성 A*B를 계산하고 두 입력을 유지하며 값을 반환한다. 유한한 위치·크기와 정규화된 유한 회전이 입력 전제다. 어느 입력의 축이든 크기<0이면 operator*의 해당 분기에서 두 행렬과 축별 desiredScale을 직접 계산하고 내부 ConstructTransformFromMatrixWithDesiredScale을 호출한다. 음수가 없으면 기존 XMQuaternionMultiply(firstRotation,secondRotation) 회전·크기 성분별 곱·B 크기/회전/이동을 A 위치에 적용하는 기본 경로다. 음수 경로는 두 행렬을 곱하고 결과 크기를 입력의 축별 곱으로 유지하며 위치는 행렬 원점에서 가져온다. 축 길이를 제거하고 결과 크기 부호를 반영해 회전을 추출·정규화한다. 축 길이 제곱>=1e-8이면 정규화하고 처리 후 어느 축의 XYZ 절댓값이 모두<=1e-4이면 회전을 항등값으로 두며 위치·크기는 유지한다. 입력 검사·입력 회전 정규화·오버플로 복구는 없다. 프로젝트·데이터 멤버·기존 수학 함수는 유지한다.

로컬/월드 계층을 도입하면 FTransform으로 합성하고 결과를 행렬로 변환한다. 일반 행렬 곱과 항상 같은 결과나 모든 shear 보존은 계약이 아니다. 표현 차이만으로 합성 실패를 도입하지 않고, 향후 KeepWorld도 재합성 차이만으로 거부하지 않는 방향이다. 합성과 상대 변환의 기본/음수 경로는 구현했고 계층·KeepWorld는 현재 구현되어 있지 않다. 단일 호출 래퍼 MultiplyUsingMatrixWithScale은 제거했고 보조 함수 세 개는 Transform.cpp 내부에만 두며 공개 타입·프로젝트 등록은 유지한다. 새 왼손 ZXY·도 입력·행 벡터·위치 단위 미정 규칙을 유지한다. 선택 이유와 이전 근거는 루트 설계 기록에서 관리한다.

GetRelativeTransform(const FTransform& other) const는 [[nodiscard]] FTransform 값 반환이다. 유한 값·단위 회전이 전제이며 other는 기준 값이다. other.IsRotationNormalized() 실패 시 기본 Transform 전체를 반환하고 성공하면 기준 위치를 뺀 뒤 켤레 회전과 축별 안전 역수로 상대 위치·회전·크기를 구한다. abs(other.scale.axis)<=1e-8f인 축의 역수는 0이다. 두 입력을 유지하며 자동 정규화·부모 저장·bool 실패 반환은 없다. 기준 회전 조회의 0.01 길이 제곱 허용오차는 정확한 단위 회전 보장이 아니다. 0/작은 기준 크기는 재합성 월드 보존을 보장하지 않는다. 안전 역수는 메서드 내부에 두고 기본/음수 경로가 공유한다. 어느 입력이든 음수 크기 축이 있으면 자신의 행렬*기준 행렬.Inverse()를 계산하고 크기=자기 크기*기준 안전 역수로 기존 ConstructTransformFromMatrixWithDesiredScale에서 재구성한다. 기준 역변환 실패는 기존 FMatrix4::Inverse의 항등행렬 대체 계약을 따르며 전체 상대 Transform 항등 반환이나 월드 보존 성공을 뜻하지 않는다(2026-10-16).

## 이전 구성 참고 — 현재 규칙 아님

아래는 재시작 전 설정 기록이다. 현재와 다른 모듈 경로·Engine.lib·WinMain·생성기 자동 실행·셰이더·객체 시스템에 관한 설명은 새 엔진의 지시나 구현 완료 근거로 사용하지 않는다. 새 구성과 겹치는 공통 도구 설정은 위 현재 구성 및 실제 파일을 기준으로 확인한다.

## 언어 표준

모든 프로젝트, 모든 구성에서 C++20(`/std:c++20`)을 쓴다. (`Directory.Build.props`, 속성 페이지: C/C++ → 언어 → C++ 언어 표준)

전처리기는 표준 준수 전처리기(`/Zc:preprocessor`)를 쓴다. (`Directory.Build.props`의 `UseStandardPreprocessor`, 속성 페이지: C/C++ → 전처리기 → 표준 준수 전처리기 사용) 가변 인자 매크로에서 인자가 없을 때의 쉼표는 C++20 `__VA_OPT__`로 처리한다(`Format __VA_OPT__(,) __VA_ARGS__`). 비표준 확장 `, ##__VA_ARGS__`는 쓰지 않는다.

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
- C++ / HLSL / C# 파일 들여쓰기: Tab, 폭 4

## `.clang-format`

`.clang-format`으로 코드 포매팅을 통일한다(Code Convention 1.1 Formatting, 적용 범위). clang-format은 파일에서 가장 가까운 폴더의 `.clang-format`을 읽는다. Visual Studio는 내장 clang-format으로 이 파일을 읽는다(도구 → 옵션 → 텍스트 편집기 → C/C++ → 코드 스타일 → 서식에서 clang-format 지원이 켜져 있어야 한다).

| 파일 | 적용 대상 | 내용 |
|---|---|---|
| `.clang-format` (저장소 루트) | 엔진 코드, 게임 코드 (게임 코드는 권장 기본값) | 공통 서식. 공백으로 줄을 맞추지 않고(`AlignConsecutiveAssignments` / `Declarations` / `Macros`: `None`), 줄 끝 주석만 맞춘다(`AlignTrailingComments`: `Always`) |

`Engine/ThirdParty/.clang-format`은 서드파티 폴더의 서식 정리와 include 정렬을 끈다(Code Convention 6.3 예외 조항).

- **`WhitespaceSensitiveMacros`** — 인자 안의 공백이 결과에 영향을 주는 매크로를 등록한다. 엔진의 `SE_STRINGIZE`(플랫폼 헤더 경로)와 clang-format 기본 다섯 이름(`BOOST_PP_STRINGIZE`, `CF_SWIFT_NAME`, `NS_SWIFT_NAME`, `PP_STRINGIZE`, `STRINGIZE`)을 함께 적는다. 이 목록은 기본 목록을 앞에서부터 덮어쓰므로, 기본 이름을 빼지 않으려면 함께 적어야 한다. clang-format 버전을 바꾸면 `clang-format --dump-config`로 실제로 읽힌 목록과 새 기본 이름을 확인한다.
- 옵션은 파일 마지막 줄 `...`(YAML 문서 끝 표시) 앞에 넣는다. 뒤에 넣으면 clang-format이 설정을 읽지 못한다.

## `.gitattributes`

줄바꿈을 각자의 `core.autocrlf` 설정과 관계없이 고정한다.

- 텍스트 파일: 저장소에는 LF, 작업 폴더에는 CRLF
- 이미지, 오디오, 폰트, 바이너리 파일: 변환하지 않음 (`binary`)
- 예외: `.bat` / `.cmd`는 CRLF, `.sh`는 LF로 고정

## 폴더 구조

CoreUObject의 최소 UObject를 구현하고 Engine/Source/CoreUObject/Public/UObject/Object.h와 Private/UObject/Object.cpp를 생성기로 등록했다. Engine 프로젝트는 CoreUObject/Public·Private를 include 경로에 추가하고 SampleGame은 CoreUObject/Public과 Engine/Public을 추가했다. 최소 UEngine의 Engine/Public/Engine/Engine.h와 Private/Engine/Engine.cpp 및 GameInstance.h / GameInstance.cpp와 WorldContext.h / WorldContext.cpp, GameEngine.h / GameEngine.cpp, Public/Engine/EngineLoop.h와 Private/Launch/LaunchEngineLoop.h / cpp, Private/Renderer/D3D11/D3D11Viewport.h / cpp의 클래스 골격도 생성기로 등록했다. 공통 ETransformRule의 Engine/Public/Engine/EngineTypes.h도 생성기로 등록했다. 기존 Engine.lib에 함께 빌드하며 별도 프로젝트 / 라이브러리 / DLL은 추가하지 않는다.

저장소 루트에 솔루션을 두고, 엔진과 게임 프로젝트를 최상위 폴더로 나눈다. 빌드 결과는 루트에 모은다.

```text
SeonEngine/
├─ SeonEngine.slnx
├─ Directory.Build.props   모든 프로젝트 공통 설정 (먼저 읽힘)
├─ Directory.Build.targets 모든 프로젝트 공통 설정 (마지막에 읽힘, vcpkg 연결)
├─ GenerateProjectFiles.bat  프로젝트 파일 목록 / 필터 생성
├─ Engine/
│  ├─ Build/BatchFiles/   빌드 도구 스크립트
│  ├─ Source/
│  │  ├─ Engine.vcxproj
│  │  ├─ Core/            모듈 (Public/, Private/)
│  │  ├─ CoreUObject/     객체 기반 모듈 (Public/, Private/)
│  │  ├─ InputCore/       공통 입력 타입 (Public/, Private/)
│  │  ├─ SlateCore/       위젯 입력 기반 (Public/, Private/)
│  │  ├─ Slate/           게임 화면 위젯 (현재 Public/Widgets/SViewport.h)
│  │  └─ Engine/          모듈
│  └─ ThirdParty/
│     └─ vcpkg.json       vcpkg로 받는 외부 라이브러리 목록 (manifest)
├─ SampleGame/            게임 프로젝트
│  └─ Source/
│     ├─ SampleGame.vcxproj
│     └─ SampleGame/      모듈 (Public / Private 없음)
├─ Docs/
├─ Binaries/              빌드 결과 (git 무시)
└─ Intermediate/          중간 파일 (git 무시)
   └─ vcpkg_installed/    vcpkg가 빌드할 때 설치하는 라이브러리
```

- 엔진과 게임 프로젝트는 같은 구조를 갖는다. 셰이더(`Shaders/`), 에셋(`Content/`), 설정(`Config/`) 폴더는 처음 필요할 때 각 폴더 아래에 추가한다.
- 현재 엔진 내부 셰이더 소스는 `Engine/Shaders/Private/`에 `.hlsl` 파일로 둔다. `PositionOnlyVertexShader.hlsl`은 Engine.vcxproj의 FxCompile로 Vertex / 5.0 / Main / TreatWarningAsError=true를 지정하고 `$(OutDir)Shaders\PositionOnlyVertexShader.cso`에 자동 컴파일한다. `SolidColorPixelShader.hlsl`도 같은 설정에서 종류를 Pixel로 지정하고 `$(OutDir)Shaders\SolidColorPixelShader.cso`에 자동 컴파일한다. 런타임은 실행 파일 폴더의 Shaders 아래 두 .cso를 Init에서 읽는다. 어느 파일이든 없거나 비어 있으면 초기화 실패다. vertex / pixel GPU 객체 생성까지 연결했으며 유효하지 않은 코드도 초기화 실패다. 현재 FRenderer.Init에서 vertex / pixel 셰이더와 input layout을 생성한다. 프레임 바인딩 / 정점 버퍼 소비 / Draw는 후속이며 현재는 배경 Clear / Present다.
- 게임 프로젝트 이름은 `<이름>Game`으로 짓고, 게임 모듈 이름은 프로젝트 이름과 같게 한다.
- `Engine/`과 게임 폴더에는 빌드 결과를 두지 않는다.

## 프로젝트

| 프로젝트 | 종류 | 담는 모듈 | include 경로 |
|---|---|---|---|
| `Engine` | 정적 라이브러리 (`.lib`) | `Core`, `CoreUObject`, `InputCore`, `SlateCore`, `Slate`, `Engine` | Core·CoreUObject·InputCore·Engine의 Public / Private, SlateCore·Slate의 Public |
| `SampleGame` | 실행 파일 (`.exe`, Windows 서브시스템) | `SampleGame` | 엔진 모듈의 `Public`, 자기 모듈 폴더 |

- `SampleGame`은 `Engine`을 프로젝트 참조로 연결한다. 빌드 순서와 `Engine.lib` 링크가 자동으로 처리된다.
- 엔진을 사용하는 프로젝트는 `Microsoft.Cpp.props`를 읽고 프로젝트 종류를 정한 뒤 `Engine/Build/SeonEngine.props`를 명시적으로 import한다. 이 파일은 엔진의 Public include 경로와 현재 D3D11 구현에 필요한 `d3d11.lib` 링크를 제공한다. 프로젝트 자체 include 경로는 기존 값을 이어 붙인다. Engine 프로젝트 참조는 각 소비 프로젝트에 유지하며 props가 자동으로 생성하지 않는다.
- `SeonEngine.props`는 import한 프로젝트에만 적용한다. 실행 파일 / DLL 프로젝트에는 링크 의존성을 제공하고 정적 라이브러리 프로젝트에서는 최종 링크를 수행하지 않는다. 별도 import가 없는 도구에는 D3D11 의존성을 전파하지 않는다. 루트의 공통 빌드 설정과 엔진 소비 설정은 구분한다.
- 진입점(`WinMain`)은 `Engine.lib`에 있다. 게임 프로젝트는 엔진과 게임 코드를 실행 파일로 링크하는 단위다.
- 게임 프로젝트에는 `.cpp`가 하나 이상 있어야 한다. 컴파일된 `.obj`가 없으면 CRT가 링크되지 않아 시작 코드(`WinMainCRTStartup`)를 찾지 못한다.
- 게임 프로젝트의 include 경로에는 엔진 모듈의 `Public`만 넣는다. 엔진 내부 헤더를 include하면 빌드가 실패한다(Code Convention 2.1 파일 구성 참고).
- 프로젝트에서 다른 폴더를 가리킬 때는 `$(SERootDir)`(저장소 루트)을 쓴다. `$(SolutionDir)`은 솔루션 없이 프로젝트만 빌드하면 값이 달라진다.

## 프로젝트 파일 생성 (`GenerateProjectFiles.bat`)

소스 파일을 추가, 삭제, 이동한 뒤 저장소 루트의 `GenerateProjectFiles.bat`을 실행한다. 각 `.vcxproj`의 파일 목록(`ClInclude` / `ClCompile` / `None`)과 `.vcxproj.filters`가 폴더 구조대로 다시 만들어진다.

- **준비물**: .NET 10 이상의 SDK. Visual Studio의 C++ 워크로드만으로는 설치되지 않는다. 설치 관리자에서 ".NET 데스크톱 개발" 워크로드를 추가하거나 SDK를 따로 설치한다.
- **스크립트**: 본체는 `Engine/Build/BatchFiles/GenerateProjectFiles.cs`이고, bat이 `dotnet run`으로 실행한다. 빌드 결과는 `Intermediate/DotNET/`에 생긴다.
- **대상**: 저장소 안의 모든 `.vcxproj`(`Binaries`, `Intermediate`, `ThirdParty` 제외). 기본은 프로젝트 폴더 아래의 `.h` / `.cpp`를 모은다. 현재 `SampleGame/Source` 프로젝트에는 `Engine/Source`와 `Engine/Shaders`를 함께 포함한다. 필터 이름은 프로젝트 기준 상대 경로에서 앞의 `..`를 제거한 폴더 경로다.
- **셰이더**: `.vcxproj`가 `Source/`에 있으면 옆의 `Shaders/` 아래 `.hlsl` / `.hlsli`도 `None` 항목으로 넣는다(예: `Engine/Shaders` → Engine 프로젝트의 `Shaders\Private` 필터). 명시적인 FxCompile 등록이 없는 파일의 기본값은 편집용 None이다. FxCompile 전용 ItemGroup은 손으로 관리하며 생성기가 메타데이터를 보존하고 None 중복을 제거한다. .filters도 해당 파일을 FxCompile로 표시한다. 등록은 검색 범위에 있는 단일 .hlsl 상대 경로만 허용하고 중복 / 변수 / 와일드카드 경로는 오류로 처리한다.
- **`.vcxproj`의 소스 목록은 직접 고치지 않는다**. 다음 실행 때 폴더 내용으로 덮어쓴다. VS의 "새 항목 추가"로 만든 파일도 실행하면 필터가 폴더에 맞춰진다. 목록 밖의 설정은 그대로 둔다.
- **자동 생성 목록에 파일별 설정은 쓸 수 없다**: 소스 목록 `ItemGroup`에 파일별 설정(메타데이터), `Condition`, 다른 항목이 있으면 오류를 내고 아무 파일도 바꾸지 않는다.
- 내용이 바뀐 파일만 쓴다. 바뀌지 않았으면 열려 있는 VS가 다시 로드를 묻지 않는다.
- 출력은 BOM 없는 UTF-8, CRLF다. `.bat`은 첫 줄들에서 코드페이지를 UTF-8(65001)로 바꾼 뒤 한국어를 쓴다(cmd는 배치 파일을 현재 코드페이지로 한 줄씩 읽는다).

## 공통 설정 (`Directory.Build.props` / `.targets`)

저장소 루트의 `Directory.Build.props`는 MSBuild가 모든 `.vcxproj`에 자동으로 import한다. 새 프로젝트도 따로 설정하지 않아도 공통 설정이 적용된다.

| 파일 | 담는 설정 |
|---|---|
| `Directory.Build.props` | 루트 경로(`SERootDir`), 출력 / 중간 디렉터리, 아래 "컴파일러 설정" 전부 |
| `Engine/Build/SeonEngine.props` | 엔진 소비 프로젝트의 Public include 경로, 현재 D3D11 링크 의존성. 명시적으로 import |
| 각 `.vcxproj` | 프로젝트 종류, include 경로, 프로젝트 전용 전처리기 정의(`_LIB`, `_WINDOWS`), 링커 / 매니페스트, 프로젝트 참조 |

| 설정 | 값 |
|---|---|
| 출력 디렉터리 (`OutDir`) | `$(SERootDir)Binaries\$(Platform)\$(Configuration)\` |
| 중간 디렉터리 (`IntDir`) | `$(SERootDir)Intermediate\$(MSBuildProjectName)\$(Platform)\$(Configuration)\` |

- 모든 프로젝트의 `.exe`, `.lib`, DLL은 `Binaries`의 같은 폴더에 모은다. 중간 파일은 프로젝트별로 나눈다.
- 공통 설정은 이 파일을 직접 고친다. VS 속성 페이지에서 바꾼 값은 해당 `.vcxproj`에만 저장된다. 속성 페이지에서는 이 파일에서 온 값이 굵지 않은 글씨로 보인다.
- 이 파일은 프로젝트 앞부분에서 import되므로 `$(ProjectName)`처럼 프로젝트가 정의하는 속성을 쓸 수 없다. `$(MSBuildProjectName)`처럼 MSBuild가 미리 정의하는 속성을 쓴다.
- MSBuild는 C# 프로젝트(`dotnet run`으로 실행하는 `.cs` 도구 포함)에도 이 파일을 import한다. 속성(`PropertyGroup`)은 `.vcxproj` 조건을 붙여 C++ 프로젝트에만 적용한다. C#도 쓰는 이름(`OutDir` 등)이 새어 들어가지 않게 하기 위해서다.
- `.vcxproj`에서 목록형 설정(전처리기 정의, include 경로, 추가 옵션)을 넣을 때는 `%(PreprocessorDefinitions)`처럼 기존 값을 이어 붙인다. 빠뜨리면 공통 값이 사라진다.
- 저장소 루트의 `Directory.Build.targets`는 MSBuild가 모든 `.vcxproj`의 **마지막**에 import한다. 프로젝트 본문에서 정해지는 값(`Platform`, `UseDebugLibraries` 등)이 필요한 설정(vcpkg 연결)을 둔다. 이 파일도 `.vcxproj` 조건을 붙인다.

## vcpkg

외부 라이브러리는 vcpkg manifest 모드로 받는다. 목록은 `Engine/ThirdParty/vcpkg.json`이고, 빌드할 때 자동으로 설치된다.

| 설정 | 위치 | 값 |
|---|---|---|
| 사용자 전역 통합 끊기 | `Directory.Build.props` | `VCPkgLocalAppDataDisabled` = `true`. 개발 PC에 `vcpkg integrate install`이 되어 있어도 그 설정(classic 창고)을 쓰지 않는다 |
| manifest 모드 | `Directory.Build.props` | `VcpkgEnableManifest` = `true`, `VcpkgManifestRoot` = `Engine\ThirdParty\` |
| 설치 폴더 | `Directory.Build.props` | `VcpkgManifestInstalledBaseDir` = `Intermediate\vcpkg_installed\` (git 무시) |
| triplet | `Directory.Build.props` | `VcpkgUseStatic` + `VcpkgUseMD` = `true` → `x64-windows-static-md` (라이브러리는 정적, CRT는 엔진과 같은 `/MD`) |
| vcpkg 위치와 연결 | `Directory.Build.targets` | `VCPKG_ROOT` 환경 변수가 있으면 그 vcpkg, 없으면 Visual Studio 내장 vcpkg(`$(VsInstallRoot)\VC\vcpkg\`)의 `vcpkg.targets`를 import한다. 찾지 못하면 빌드 오류로 안내한다 |

**설치 목록** (`builtin-baseline` `eb2d3a32...`, vcpkg 2026-07-27로 확인)

| 패키지 | 버전 | 이유 | 설치되는 것 |
|---|---|---|---|
| `directxmath` | 3.21 (2026-06-12) | 엔진 수학 타입의 내부 계산 | 헤더 |

- **DirectXMath 출처**: vcpkg판을 쓴다. vcpkg include 경로가 Windows SDK 경로보다 먼저 검색되어 `#include <DirectXMath.h>`는 SDK판(3.19)이 아니라 vcpkg판을 가리킨다. 목록에 적어 버전을 baseline으로 고정한다.
- **새 PC 준비**: Visual Studio Installer에서 "vcpkg 패키지 관리자" 구성 요소를 설치한다. 다른 vcpkg를 쓰려면 `VCPKG_ROOT` 환경 변수를 지정한다.
- **첫 빌드**: 목록의 라이브러리를 내려받아 컴파일하므로 인터넷이 필요하고 오래 걸릴 수 있다. 이후에는 캐시를 쓴다.
- **설치 위치 확인**: 빌드 출력(상세도 "보통" 이상)에 `Using triplet "x64-windows-static-md" from "...\Intermediate\vcpkg_installed\..."`가 나온다.
- vcpkg는 설치 폴더의 `lib\*.lib`를 전부 자동으로 링크한다(전이 의존성 포함).
- vcpkg 명령을 직접 실행할 때는 `Engine/ThirdParty/`에서 실행하거나 `--x-manifest-root`로 위치를 넘긴다.

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
| RTTI | 켬 (`/GR`) | 언어 → 런타임 형식 정보 사용 |
| 전처리기 정의 | `SE_PLATFORM_WINDOWS`, `SE_BUILD_DEBUG`, `SE_BUILD_RELEASE` (구성별 0 / 1), `SE_PLATFORM_HEADER_NAME=Windows`(플랫폼 헤더 경로를 만드는 이름) | 전처리기 → 전처리기 정의 |


InputCore 최소 키 타입(2026-10-12): Engine/Source/InputCore/Public/InputCoreTypes.h와 Private/InputCoreTypes.cpp를 기존 Engine.lib에 함께 빌드한다. Engine.vcxproj는 InputCore/Public·Private를 include 경로에 포함하고, 공통 Engine/Build/SeonEngine.props는 소비 프로젝트에 InputCore/Public만 제공한다. 별도 라이브러리 / DLL은 추가하지 않는다.

SlateCore 최소 입력 응답(2026-10-13): Engine/Source/SlateCore/Public/Input/Reply.h를 기존 Engine 프로젝트·Engine.lib에 등록한다. Engine.vcxproj 및 Engine/Build/SeonEngine.props는 SlateCore/Public 검색 경로를 제공한다. 사용처는 Input/Reply.h를 포함한다. 별도 SlateCore 프로젝트·라이브러리·DLL은 없다.

SlateCore 최소 위젯 입력(2026-10-13): Public/Widgets/SWidget.h는 기존 Engine 프로젝트의 헤더 목록·filters에 등록한다. 기존 SlateCore/Public 검색 경로를 사용한다. 추가 cpp·별도 라이브러리는 없다.

키 이벤트 SlateCore 배치(2026-10-13): Events.h와 Events.cpp를 SlateCore/Public/Input 및 SlateCore/Private/Input에 등록한다. 기존 Engine 쪽 파일은 제거하며 별도 호환 헤더를 두지 않는다. Public 검색 경로는 기존 설정을 재사용하고 새 Private 검색 경로는 추가하지 않는다. 구현은 기존 Engine.lib에서 함께 컴파일한다.

SlateCore 화면 입력 인터페이스(2026-10-13): Public/Rendering/RenderingCommon.h를 기존 Engine 프로젝트·filters에 등록한다. 기존 SlateCore/Public 검색 경로를 사용하며 추가 cpp·별도 빌드 단위는 없다.

Slate 최소 게임 화면 위젯(2026-10-13): Engine/Source/Slate/Public/Widgets/SViewport.h를 기존 Engine 프로젝트·filters에 등록한다. Engine.vcxproj와 Engine/Build/SeonEngine.props에 Slate/Public 검색 경로를 추가하며 사용처는 Widgets/SViewport.h를 포함한다. 별도 Slate 프로젝트·라이브러리·DLL·Private 검색 경로는 없다.

게임 입력 종류 헤더(2026-10-13): Engine/Public/Engine/EngineBaseTypes.h를 생성기로 기존 Engine 프로젝트·filters에 등록한다. 기존 Engine/Public 검색 경로를 사용하며 별도 cpp·라이브러리·리플렉션 설정은 없다.

게임 입력 인수 헤더(2026-10-13): Engine/Public/InputKeyEventArgs.h를 기존 Engine 프로젝트·filters에 등록한다. 기존 Engine/Public·InputCore/Public 검색 경로를 사용하고 생성자는 헤더 안에 정의한다. 추가 cpp·검색 경로·별도 빌드 단위는 없다.

화면 클라이언트 키 계약(2026-10-13): Engine/Public/ViewportClient.h를 기존 Engine 프로젝트·filters에 등록한다. 기존 Engine/Public 검색 경로를 재사용하고 헤더 내 짧은 기본 정의만 제공한다. 추가 cpp·설정 경로·라이브러리는 없다.

게임용 화면 클라이언트 기반(2026-10-13): Engine/Public/Engine/GameViewportClient.h·Private/Engine/GameViewportClient.cpp를 기존 Engine 프로젝트·filters에 등록한다. 기존 Engine·CoreUObject 공개 검색 경로를 사용한다. 별도 프로젝트·라이브러리·검색 경로는 추가하지 않는다.

입력 컴포넌트 소스 등록(2026-10-13): Engine/Source/Engine/Public/InputComponent.h·Private/InputComponent.cpp를 기존 프로젝트 생성기로 Engine.vcxproj와 filters에 등록했다. Public 검색 경로와 단일 Engine.lib 구성을 재사용하며 새 모듈·include 설정·SampleGame 프로젝트/filters 변경은 없다. 입력 기능 사용·런타임 등록은 이 프로젝트 등록과 별개다.

## 최소 로그 함수 — 2026-10-15

Engine/Source/Logging/Log.h/.cpp의 ELogLevel(Info/Warning/Error)과 WriteLog를 사용한다. 두 파일은 SampleGame/Source/SampleGame.vcxproj 및 Engine/Source/Logging 표시 필터에 등록했다. WindowsApplication.cpp의 일반 정보와 API 실패 출력 12곳을 공통 함수로 전달한다. 기존 오류 코드 저장·반환·정리 흐름은 유지한다.

Debug/Release 모두 세 레벨을 stderr에 출력한다. 메시지 조립은 호출부 std::format, 함수 입력은 std::string_view다. 이전 SE_LOG·카테고리·Release 필터 정책은 새 코드에 적용하지 않는다. 현재 규칙은 [CodeConvention 5.3](Conventions/CodeConvention.md), 출력 실패·수명·스레드 한계도 해당 절을 따른다. 공통 빌드 옵션·외부 의존성은 변경하지 않았다.

## 로그 카테고리 인자 확장 — 2026-10-15

위 최소 로그 함수의 두 인자 API를 WriteLog(std::string_view category, ELogLevel level, std::string_view message)로 확장했다. 출력은 [카테고리][레벨] 메시지이며 빈 카테고리는 General로 표시한다. WindowsApplication 호출 12곳은 Windows를 지정한다. category도 호출 중에만 읽으며 저장하지 않는다. 레벨·stderr·Debug/Release 동일 출력 정책은 유지하며 카테고리 등록·필터·문자열 검증은 제공하지 않는다. 프로젝트 파일·외부 의존성·빌드 옵션은 이번 단계에서 변경하지 않았다.

## FString·FStringView 별칭 — 2026-10-15

Engine/Source/Containers/SeonString.h에 FString = std::string과 FStringView = std::string_view를 정의하고 프로젝트 header 목록·기존 Containers 필터에 등록했다. Logging/Log.h/.cpp의 카테고리·메시지 인자는 FStringView를 사용한다. 출력과 호출 방식은 유지한다. TCHAR·문자 타입 계층·변환기는 추가하지 않았고 Windows 경계의 기존 std::wstring 예외도 유지한다. 규칙은 CodeConvention 3.12/5.3이며 이전 로그의 std::string_view 직접 사용 예외는 이 단계에서 종료한다. 형식 문자열 편의 함수는 별도 후속 범위다.

## 로그 형식 문자열 함수 — 2026-10-15

Logging/Log.h에 WriteLog(FStringView category, ELogLevel level, std::format_string<ArgTypes...> format, ArgTypes&&... args) 템플릿을 정의했다. Log.cpp의 출력 전용 함수는 SE::Private::WriteLogMessage로 분리했다. 창 코드 12곳의 std::format을 생략했으며 기존 카테고리·출력·오류 처리 흐름은 유지한다. 완성 문자열은 "{}", message로 전달한다. FString/FStringView를 유지하고 TCHAR는 추가하지 않는다. 프로젝트·필터·빌드 옵션·외부 의존성은 변경하지 않았다. 현재 규칙은 CodeConvention 5.3을 따른다.

## 백 버퍼 RTV 준비 — 2026-10-16

FD3D11SwapChain::Initialize는 스왑 체인 생성 뒤 GetBuffer(0)·CreateRenderTargetView로 백 버퍼 RTV도 준비한다. 모두 성공해야 true이며 실패 시 지역 COM 참조를 정리한다. 성공 로그는 기존 스왑 체인 로그 다음 D3D11 back buffer render target view created.다. Shutdown은 RTV → 스왑 체인 순서로 Reset한다. 프로젝트·링크 라이브러리·main·대기 정책은 바꾸지 않았다. RTV 바인딩·Clear·Present·ResizeBuffers는 아직 없다. 앞선 렌더 타깃 미구현 설명은 그 단계 당시 상태다.

## 고정 색 화면 출력 — 2026-10-16

현재 main의 반복은 PumpMessages → Clear → Present이며 WaitMessage는 제거했다. 색은 RGBA (0.05,0.10,0.20,1)다. GetContext는 비소유 즉시 컨텍스트 getter이고 Clear는 같은 장치의 유효한 컨텍스트·float 4개·준비된 RTV를 전제로 한다. Present는 준비된 체인에 Present(1,0)을 호출한다. FAILED일 때 로그/false를 전달하고 main이 정리 후 1을 반환한다. 비실패 상태는 true이며 실제 표시 완료 보장은 아니다. 프로젝트·링크 설정은 그대로다. Draw·리사이즈·가려짐 대기 최적화·게임 Tick은 아직 없다. 앞선 Clear/Present 미구현·WaitMessage 유지 기록은 해당 단계 당시 상태다.

## 창 클라이언트 크기 조회 — 2026-10-16

WindowsApplication.h/.cpp에 bool GetClientSize(int& outWidth,int& outHeight) const를 추가했다. 유효한 창에서 GetClientRect의 너비·높이를 반환하며 실패는 Windows 로그/false이고 출력 인자는 유지한다. 출력에는 서로 다른 변수를 사용한다. 프로젝트·링크 설정·main은 그대로다. 호출부 연결·변경 감지·ResizeBuffers는 후속이다.

## 백 버퍼 리사이즈 연결 — 2026-10-16

main은 초기/매 반복 GetClientSize를 조회하고 0 크기에서는 출력하지 않는다. 마지막 정상 크기와 달라지면 FD3D11SwapChain::Resize(device,width,height)를 호출한다. RTV 해제·ResizeBuffers·RTV 재생성이 모두 성공해야 크기를 갱신한다. API 실패는 로그/false이고 main이 역순 정리 후 1을 반환한다. Initialize/Resize는 private CreateBackBufferRenderTargetView를 공유한다. 장치가 같고 백 버퍼가 외부에 참조/바인딩되지 않은 상태가 전제다. 프로젝트·링크·장치 생성 설정은 그대로다. 깊이 버퍼·WM_SIZE 연결·최소화 대기 최적화는 없다. 앞선 ResizeBuffers 미구현 기록은 해당 단계 당시 상태다.

## 색 지우기 책임 변경 — 2026-10-16

SwapChain::Clear를 제거하고 Device::ClearRenderTargetView(ID3D11RenderTargetView*,const float[4])로 옮겼다. main은 SwapChain::GetBackBufferRenderTargetView()의 비소유 포인터를 즉시 전달한다. RTV 소유·리사이즈·Present는 SwapChain, 소유 컨텍스트 명령은 Device가 담당한다. 리사이즈/종료 뒤 이전 포인터를 사용하지 않는다. 현재는 main이 참조를 보관/바인딩하지 않는다. 기존 GetContext·프로젝트·링크·출력 색·오류 정책은 유지한다. 이전 SwapChain Clear/RTV 외부 노출 없음 기록은 이번 변경으로 정정한다.

## 최소 삼각형 HLSL 빌드 등록 — 2026-10-16

SampleGame.vcxproj의 FxCompile에 Engine/Shaders/Private/TriangleVertexShader.hlsl(Vertex)·TrianglePixelShader.hlsl(Pixel)을 등록한다. 진입점 Main·모델 5.0·TreatWarningAsError=true이며 ObjectFileOutput은 $(OutDir)Shaders의 같은 이름 .cso다. SEPrepareShaderOutput은 FxCompile 전에 출력 폴더를 생성한다. filters의 Engine/Shaders/Private에 두 항목을 표시한다. Debug/Release의 FXC 기본 디버그 정보·최적화 설정을 유지한다. 생성기·공통 설정·링크 라이브러리·런타임 C++는 변경하지 않았다. 셰이더 로딩·GPU 객체·바인딩·Draw는 후속이다.

## D3D11 셰이더 파일 읽기 등록 — 2026-10-16

D3D11Shader.h/.cpp를 SampleGame 프로젝트와 Engine/Source/D3D11 필터에 등록했다. ReadShaderBytecode는 filesystem::path를 입력받아 TArray<uint8>에 파일 전체를 읽고 실패 시 로그/false와 기존 출력 보존을 제공한다. 새 include 경로·링크 라이브러리·공통 옵션은 없다. 내부 자유 함수이며 main 연결·실행 파일 기준 경로·셰이더 객체 생성은 아직 없다.

## FPaths 기준 경로 등록 — 2026-10-16

Misc/Paths.h/.cpp와 Engine/Source/Misc 필터를 SampleGame 프로젝트에 등록했다. FPaths::BaseDir()는 optional<filesystem::path>로 실행 파일의 부모 폴더를 반환한다. 현재 작업 폴더·저장소 루트와 구분하며 폴더를 생성하거나 존재 검사하지 않는다. Windows API는 cpp의 WindowsHWrapper include 뒤에서 직접 사용한다. 별도 플랫폼 계층·새 링크 라이브러리·공통 설정·main 변경은 없다. 셰이더/Content/월드 경로 연결은 후속이다.

## main의 셰이더 바이트코드 읽기 — 2026-10-16

EngineMain.cpp는 장치/창 생성 전에 FPaths::BaseDir() 아래 Shaders의 TriangleVertexShader.cso와 TrianglePixelShader.cso를 읽는다. 둘 다 성공하면 각각 바이트 수를 로그로 남기고 기존 초기화를 진행하며 어느 단계든 실패하면 1이다. CWD에 의존하지 않고 exe 옆 .cso 두 파일이 실행 필수가 된다. 프로젝트·링크·셰이더 빌드는 변경하지 않았다. GPU 셰이더 생성/바인딩/Draw는 후속이다.

## Device 셰이더 생성 API — 2026-10-16

Device.h/.cpp에 [[nodiscard]] CreateVertexShader/CreatePixelShader를 추가했다. 입력은 const TArray<uint8>&, 출력은 해당 ID3D11VertexShader/ID3D11PixelShader의 ComPtr&다. 장치 미준비·빈 입력·API 실패는 로그/false이며 출력은 유지한다. 성공은 출력 참조 교체다. 공개 서명에 필요한 Containers/Array.h·CoreTypes.h를 직접 포함한다. Device의 셰이더 멤버·main 연결·프로젝트·링크 설정·공통 옵션은 변경하지 않았다.

## UWorld 최소 계약 — 2026-10-16

Engine/Source/World.h/.cpp를 SampleGame.vcxproj의 ClInclude/ClCompile과 기존 Engine/Source 필터에 각각 한 번 등록한다. UWorld는 UObject public 파생 타입이며 기본 생성자·override 가상 소멸자·복사/이동 금지·private TArray<TUniquePtr<AActor>> actors{}를 제공한다. 생성자/소멸자는 World.cpp에서 default 정의한다.

SpawnActor 템플릿 추가로 헤더는 Actor.h를 직접 포함하며 cpp의 기존 Actor.h include/생성자/소멸자 정의는 유지한다. World는 아래 SpawnActor<T>로 생성한 액터를 소유 목록에 등록한다. 외부에서 World 목록을 직접 수정하는 API·부모 연결 변경·제거 요청·Tick·GC는 아직 없다. Actor의 부모/자식 보관·조회는 아래 계약을 따른다. AActor는 아래 계약대로 World 연결 생성자를 사용하고 컴포넌트 소유·main은 유지한다. U 접두사는 UObject 상속을 뜻하며 런타임 객체 등록을 뜻하지 않는다.

## main의 셰이더 객체 생성 연결 — 2026-10-16

main은 파일 읽기·장치 생성 후 두 지역 ComPtr에 정점/픽셀 셰이더를 생성한다. 두 생성이 성공하면 Triangle vertex and pixel shaders created.를 기록한 뒤 창 준비를 진행한다. 생성 실패와 후속 초기화 실패는 이미 보유한 셰이더를 장치보다 먼저 해제하고 1을 반환한다. 정상 종료도 셰이더 Reset 뒤 장치 Shutdown이다. 바인딩/Draw·프로젝트·링크·셰이더 빌드는 변경하지 않았다.

## VS/PS 바인딩 연결 — 2026-10-16

Device::SetVertexShader/SetPixelShader가 소유 컨텍스트에 같은 장치의 셰이더를 연결한다. nullptr는 해제이고 준비된 컨텍스트가 전제다. main은 출력 반복 Clear 뒤 두 셰이더를 연결하고 반복 종료 뒤 nullptr로 해제한 후 기존 소유 참조를 정리한다. 초기화 실패는 바인딩 전이다. 컨텍스트 참조가 생기므로 이전 무바인딩 정리 설명은 이번 절로 정정한다. RTV/뷰포트/Draw·프로젝트·링크·디버그 레이어 설정은 변경하지 않았다.

## AActor의 World 연결 계약 — 2026-10-16

Actor.h/.cpp의 생성자는 explicit AActor(UWorld& inWorld)이며 인자 없는 생성은 제공하지 않는다. Actor.h는 UWorld를 전방 선언하고 private UWorld* const world를 저장한다. 생성자는 world(&inWorld)를 초기화한 뒤 기존 기본 TransformComponent를 만든다. [[nodiscard]] UWorld& GetWorld() const는 저장한 World를 반환한다. const Actor에서도 World를 수정할 수 있으며 null 반환·등록 조회·수명 연장은 제공하지 않는다.

World는 Actor보다 오래 살아야 한다. Actor 소멸자는 소유 컴포넌트를 정리하고 World나 자식 포인터의 대상을 파괴하지 않는다. SpawnActor<T>를 쓰면 World가 생성·등록·소유한다. AActor(world)로 직접 만든 Actor는 여전히 자동 등록되지 않는다. Actor의 UObject 상속·가상 소멸·복사/이동 금지·컴포넌트 추가/조회 정책은 유지한다. 프로젝트 등록·World.cpp·Object.h·main은 변경하지 않는다.

## 렌더 타깃·뷰포트 연결 — 2026-10-16

Device::SetRenderTarget(rtv)는 RTV 하나/깊이 없음, nullptr는 OM 전체 렌더 타깃 해제다. SetViewport(int,int)는 원점0·깊이0~1·입력 크기를 설정하며 준비된 컨텍스트와 양수/장치 지원 범위 크기가 전제다. main은 매 출력에서 RTV/뷰포트를 설정하고 리사이즈 직전·반복 종료 시 RTV 연결을 해제한다. 이전 백 버퍼 무바인딩 설명은 현재 호출자 해제 계약으로 정정한다. SwapChain 동작·프로젝트·링크·셰이더·토폴로지/Draw는 변경하지 않았다.

## 최소 삼각형 그리기 연결 — 2026-10-16

Device::SetPrimitiveTopology/Draw를 추가하고 main이 VS/PS 연결 뒤 TRIANGLELIST·Draw(3,0)를 수행한다. 준비된 컨텍스트·출력/셰이더 등 필요한 상태가 전제다. 현재 VS는 SV_VertexID이므로 정점 버퍼/입력 레이아웃 없이 실행한다. 기존 RTV 해제·리사이즈·셰이더 정리·프로젝트·링크·셰이더 소스는 유지한다. 일반 메시·텍스처·카메라·월드 렌더링은 추가하지 않았다. 이전 Draw 미연결 기록은 당시 상태다.

## 위치 정점 버퍼 생성 API — 2026-10-16

Device.h/.cpp에 [[nodiscard]] CreateVertexBuffer(const TArray<FVector3>&,ComPtr<ID3D11Buffer>&)를 추가했다. Math/Vector3.h를 직접 포함하고 cpp는 limits를 사용한다. 장치/빈 입력·UINT 바이트 범위를 검사하며 IMMUTABLE/VERTEX_BUFFER·입력 초기 데이터로 생성한다. API 실패는 HRESULT 로그/false, 모든 검사 실패는 기존 출력 유지다. 셰이더·main·입력 레이아웃/바인딩·프로젝트·링크 옵션은 변경하지 않았다.

## SpawnActor 최소 계약 — 2026-10-16

World.h에 template<typename T> requires std::derived_from<T,AActor> && std::constructible_from<T,UWorld&>의 [[nodiscard]] T& SpawnActor()를 정의한다. Actor.h·concepts·utility를 직접 포함하고 AActor 전방 선언은 제거한다. 명시한 T에 World를 전달해 MakeUnique로 생성 → 주소 보관 → actors.push_back(std::move(actor))로 소유 등록 → 비소유 T& 반환 순서다.

생성자는 소유 목록 등록 전에 실행된다. 반환 참조의 수명은 소유한 World 수명까지이며 호출자는 delete하지 않는다. 목록 추가로 소유 포인터 컨테이너가 재배치되어도 별도 할당된 액터 주소는 유지된다. 실패 bool/null 반환·할당 실패 복구·생명주기 이벤트·main 사용은 제공하지 않는다. 직접 Actor(world) 생성은 자동 등록하지 않는 기존 계약을 유지한다. World.cpp·Actor.h/.cpp·프로젝트·공통 설정은 이 단계에서 수정하지 않는다.

## 위치 입력 레이아웃·버퍼 연결 API — 2026-10-16

Device.h/.cpp에 CreateInputLayout(VS bytecode,ComPtr<ID3D11InputLayout>&)·SetInputLayout·SetVertexBuffer를 추가했다. 레이아웃은 POSITION0 float3·slot0·offset0·per-vertex이고 버퍼 연결은 slot0·stride sizeof(FVector3)·offset0이다. 생성 실패는 로그/false·기존 출력 유지, 두 연결의 nullptr는 해제다. 준비된 컨텍스트·같은 장치의 호환 자원이 전제다. main·제품 셰이더·프로젝트·링크·공통 설정은 유지하고 실제 제품의 POSITION 전환은 아직 없다.

## Actor 부모·자식 조회 계약 — 2026-10-16

Actor.h는 int32 사용을 위해 CoreTypes.h를 직접 포함한다. private AActor* parent=nullptr와 TArray<AActor*> children{}는 모두 비소유 관계 데이터다. GetParent() const·GetChildCount() const·GetChild(int32 index) const는 모두 [[nodiscard]]이며 부모 포인터·직접 자식 수·직접 자식 포인터를 반환한다. GetChild는 index<0을 먼저 검사하고 size_type으로 변환한 인덱스>=children.size()이면 nullptr다.

조회는 계층·소유권을 변경하지 않는다. 포인터는 비소유이며 대상 파괴 시 무효이고 const Actor에서도 자식 수정이 가능하다. 자식 목록 자체를 외부에 공개하지 않고 후손 검색도 하지 않는다. 초기 부모 없음·자식0이며 이후 SetParent/SetActorParent로 실제 자식 연결·분리가 가능하다. 기존 World 연결·SpawnActor·컴포넌트 소유 정책은 유지한다. Actor의 default 소멸자는 컴포넌트 소유 목록을 정리하고 자식 포인터 배열은 대상 Actor를 삭제하지 않는다. 연결된 계층의 World 전체 소멸은 후속 부모 연결 검사에서 확인했다. 개별 부모 제거와 자손 제거는 아직 없으며 별도 후속이다.

## 제품 POSITION 입력 전환 — 2026-10-16

TriangleVertexShader.hlsl은 Main(float3 position:POSITION)에서 float4(position,1)을 반환한다. main은 기존과 같은 세 FVector3 위치로 버퍼/입력 레이아웃을 생성·소유하고 매 출력에 연결한다. 종료에는 두 입력 연결을 해제한 뒤 소유 참조를 장치보다 먼저 해제한다. 초기화 실패 시 이미 만든 자원도 정리한다. 새 성공 로그는 Triangle vertex buffer and input layout created.다. 기존 FXC/Main/5.0·출력 경로·픽셀 셰이더·프로젝트/링크·카메라/인덱스 정책은 유지한다. 예전 SV_VertexID 설명은 이전 단계다.

## 상수 버퍼 생성 함수 — 2026-10-16

Device.h/.cpp에 [[nodiscard]] CreateConstantBuffer(uint32,ComPtr<ID3D11Buffer>&)를 추가했다. 준비된 장치·양수16바이트 배수·최대64KiB를 검사하고 DEFAULT/CONSTANT_BUFFER·CPUAccess0으로 생성한다. 초기 내용은 미정이고 실패 시 로그/false·기존 출력 유지다. main·셰이더·프로젝트·링크·기존 자원 정책은 유지하며 데이터 갱신·VS 바인딩은 아직 없다.

## 상수 버퍼 갱신 API — 2026-10-16

Device.h/.cpp에 [[nodiscard]] UpdateConstantBuffer(ID3D11Buffer*,const void*,uint32)를 추가했다. 같은 장치의 DEFAULT/CONSTANT_BUFFER 전체 크기만 허용하며 null/준비 상태·소속/종류/크기 실패는 로그/false와 기존 내용 보존이다. true는 UpdateSubresource 호출 완료이며 GPU 완료가 아니다. data는 byteSize만큼 읽을 수 있어야 한다. main·HLSL·슬롯 연결·프로젝트·링크·공통 설정은 유지한다.

## VS 상수 버퍼 슬롯 setter — 2026-10-16

Device.h/.cpp에 SetVertexShaderConstantBuffer(ID3D11Buffer*)를 추가했다. VS 슬롯0에 하나를 연결하고 nullptr면 해제한다. 준비된 컨텍스트·같은 장치 상수 버퍼가 전제이며 셰이더가 읽기 전 내용을 갱신해야 한다. 현재 main/HLSL·프로젝트·링크·공통 설정은 유지하고 제품에서 함수 사용은 아직 없다.

## 행렬 상수 버퍼의 제품 연결 — 2026-10-16

TriangleVertexShader의 b0 cbuffer는 row_major float4x4 LocalToClip이며 행 벡터 mul을 사용한다. main은64바이트 FMatrix4에 SetIdentity를 호출하고 상수 버퍼 생성/초기 갱신 후 매 출력 VS 슬롯0에 연결한다. 생성/갱신 및 이후 초기화 실패는 보유 상수 참조를 정리하며 종료에는 슬롯 연결 해제 뒤 소유 참조를 장치보다 먼저 놓는다. 초기 행렬은 항등으로 유지하고 매 프레임 갱신하지 않는다. 새 로그는 Triangle transform constant buffer initialized.다. 프로젝트·링크·PS·카메라/시간 정책은 변경하지 않았다.

## FTransform에서 삼각형 이동 행렬 생성 — 2026-10-16

EngineMain.cpp의 직접 FMatrix4/SetIdentity 설정을 FTransform triangleTransform → position={0.25f,0,0} → const FMatrix4 localToClip=ToMatrixWithScale()로 바꿨다. 기본 회전 없음·크기1을 유지한다. 기존64바이트 상수 버퍼 생성/최초 갱신·VS b0 연결·row_major/HLSL 곱·해제 흐름은 유지하며 행렬은 초기화 때 한 번만 계산한다.

이 단계에서 Device·Math·셰이더·프로젝트 파일/공통 빌드 설정은 변경하지 않았다. 현재 이동은 카메라 없는 클립 변환 예제이며 자동 갱신/게임 연결은 없다.

## 32비트 인덱스 버퍼 생성 — 2026-10-16

FD3D11Device.h/.cpp에 [[nodiscard]] bool CreateIndexBuffer(const TArray<uint32>& indices,ComPtr<ID3D11Buffer>& outBuffer)를 추가했다. 준비된 장치·비어 있지 않은 배열·UINT ByteWidth 표현 범위를 검사하고 IMMUTABLE/INDEX_BUFFER·CPU 접근 없음으로 입력 데이터를 복사한다. 지역 ComPtr가 생성 성공한 뒤에만 출력으로 이동하므로 검사/API 실패 시 로그/false와 기존 출력 보존을 보장한다. 버퍼는 호출자가 소유하며 Device는 따로 보관하지 않는다.

버퍼 생성에는 정점 개수나 토폴로지를 받지 않으므로 인덱스 값의 정점 범위나 개수의3배수 여부는 검사하지 않는다. 입력을 보관하는 역할과 그리기 유효성은 구분한다. uint32 데이터만 지원하며 이후 IA 연결에서 대응 형식 DXGI_FORMAT_R32_UINT를 지정해야 한다. 생성 자체가 그 형식을 메타데이터에 저장하는 것은 아니다. main·셰이더·인덱스 바인딩·DrawIndexed는 이번에 변경하지 않았다. 프로젝트·링크·빌드 설정은 유지한다.

## 인덱스 버퍼 연결과 DrawIndexed — 2026-10-16

Device.h/.cpp에 SetIndexBuffer(ID3D11Buffer*)와 DrawIndexed(uint32 indexCount,uint32 startIndexLocation,int32 baseVertexLocation)를 추가했다. SetIndexBuffer는 준비된 컨텍스트에 같은 장치의 INDEX_BUFFER를 DXGI_FORMAT_R32_UINT·바이트 오프셋0으로 연결하고 nullptr면 해제한다. DrawIndexed는 세 인자를 그대로 컨텍스트에 전달한다. 유효한 입력/셰이더/출력 상태와 인덱스·정점 범위는 호출자가 보장하며 main·셰이더·프로젝트는 그대로다.

인덱스 연결/그리기는 Device의 명령 함수이며 버퍼 생성/소유와 분리한다. 호출자는 같은 장치의 유효한32비트 버퍼와 그리기 상태를 준비하고 사용 후 연결을 해제한다. 현재 제품 main에는 두 함수 호출을 추가하지 않았다.

## main의 인덱스 사각형 연결 — 2026-10-16

EngineMain.cpp의 위치 배열을 좌하·좌상·우상·우하 순서의 정점4개로 바꾸고 uint32 인덱스 {0,1,2,0,2,3}을 추가했다. 정점 버퍼/입력 레이아웃 뒤 인덱스 버퍼를 생성해 지역 ComPtr로 소유한다. 매 출력 SetIndexBuffer → TRIANGLELIST → DrawIndexed(6,0,0)을 호출한다. 종료 전 인덱스 바인딩을 nullptr로 해제하고 상수 버퍼 다음에 소유 인덱스 버퍼 참조를 정리한다. 인덱스 생성 실패는 기존 레이아웃/정점/셰이더/장치를 정리하고1을 반환하며, 이후 초기화 실패 경로에도 인덱스 버퍼 정리를 추가했다.

주황색·FTransform의 X=0.25 이동·초기64바이트 행렬 갱신·VS b0·리사이즈 흐름은 유지한다. 지역 변환 이름과 성공 로그는 Quad로 맞췄다. 기존 TriangleVertexShader/PixelShader 파일 이름과 FXC/로딩 경로는 그대로 재사용하며 셰이더 자체는 변경하지 않았다. Device·Math·Windows·World·프로젝트/공통 빌드 설정은 이번에 변경하지 않았다. 현재 제품은 기존 비인덱스 삼각형 대신 인덱스 사각형을 그린다.

## main에서 FRenderer로 렌더링 책임 분리 — 2026-10-16

Engine/Source/Renderer/Renderer.h/.cpp의 구체 클래스 FRenderer에 장치·스왑 체인·셰이더·입력 레이아웃·정점/인덱스/변환 상수 버퍼 소유와 크기 보관을 옮겼다. Initialize(HWND,int,int)는 셰이더/자원/체인을 준비하고 자원 생성 중 실패하면 Shutdown으로 확보한 참조를 정리한다. RenderFrame(int,int)는 양수 크기 변화 시 RTV 해제 → Resize → 성공 크기 보관 후 기존 Clear/바인딩/DrawIndexed/Present를 수행한다. false면 호출자는 반복을 중단하고 Shutdown한다. Shutdown은 준비된 컨텍스트가 있으면 바인딩을 해제하고 체인 → 버퍼/셰이더 → 장치를 정리한 뒤 크기0/미초기화로 되돌린다. 빈 상태·부분 초기화 뒤 반복 정리를 허용하며 복사/이동은 금지한다.

main은 창 생성/표시 → 최초 크기 조회 → 렌더러 Initialize → 메시지 펌프/크기 조회/0 크기 건너뛰기/RenderFrame 반복 → 렌더러 Shutdown → 남은 창/클래스 Shutdown과 종료 코드 전달만 맡는다. 이전 GPU 준비 → 창 생성 순서를 창 생성 → 렌더러 준비로 바꿨다. 부분 GPU 실패 때 main이 여러 ComPtr를 직접 정리하던 중복을 제거했다. 명시적 정리 호출은 렌더러가 먼저지만 사용자가 닫아 WM_DESTROY가 이미 처리된 경우보다 앞서 OS 창을 파괴한다는 뜻은 아니다. Windows 메시지 동작 자체는 변경하지 않았다.

정점4개/인덱스6개·주황색·X=0.25 변환·64바이트 b0·32비트 인덱스·TRIANGLELIST/DrawIndexed(6,0,0)·Present(1,0)의 출력 계약을 유지했다. 기존 D3D11 클래스·Math·Windows·World·제품 HLSL/FXC 설정은 유지하고 프로젝트/필터에는 Renderer.h/.cpp만 등록했다. 사각형은 당분간 렌더러의 시연 데이터이며 게임/메시 입력 계약을 새로 정하지 않았다. IRenderer·RHI·EngineLoop 계층·정점 색상 기능은 추가하지 않았다. 새 소스/헤더는 SampleGame.vcxproj와 Engine/Source/Renderer 필터에 등록하며 별도 프로젝트/라이브러리를 만들지 않는다.

## Actor 자식 배열 비소유 전환 — 2026-10-16

Actor.h의 children은 TArray<AActor*>이다. GetChild는 index<0 또는 TArray<AActor*>::size_type으로 비교한 범위 밖이면 nullptr, 유효하면 children[index]를 반환한다. .get()은 제거했고 함수 시그니처·직접 자식 조회·비소유 반환 계약은 유지한다. Actor 소멸은 Component를 정리하고 World나 자식 포인터 대상을 삭제하지 않는다.

World의 소유 배열·SpawnActor, Actor의 필수 World 생성자·GetWorld·Component 소유/조회는 유지한다. friend·관계 변경/등록 조회/개별 제거·해시맵·핸들은 추가하지 않았다. 기존 생성/소멸 검사와 전체 Debug·Release 빌드로 검증하고, 실제 자식 연결/정리는 후속 API에서 검증한다.

## FVertex와 정점 색상 전달 — 2026-10-16

Renderer/Vertex.h에 GPU 자원을 소유하지 않는 FVertex를 추가했다. FVector3 position{}와 float color[4]={1,1,1,1}로 로컬 위치·선형 RGBA를 보관한다. 기본값은 원점/불투명한 흰색이다. Device::CreateVertexBuffer는 TArray<FVertex>를 받아 sizeof(FVertex)로 UINT 크기를 검사/생성하고 SetVertexBuffer는 같은 크기를 stride로 쓴다. 현재 MSVC 데이터 배치는 position 오프셋0·color 오프셋12·총28바이트다. CreateInputLayout은 offsetof와 표준 레이아웃/단순 복사 가능 컴파일 검사를 사용해 슬롯0의 POSITION0 float3·COLOR0 float4 두 원소를 연결한다. 생성 실패 시 기존 출력 보존 계약은 유지한다.

FRenderer의 시연 정점은 좌하 빨강·좌상 초록·우상 파랑·우하 노랑으로 바꿨다. 기존 위치4개·인덱스 {0,1,2,0,2,3}·X=0.25 이동·변환 상수 버퍼·리사이즈/정리 흐름은 유지한다. VSOutput은 SV_POSITION 뒤 COLOR0을 출력하고 PSInput도 같은 필드 순서로 받는다. VS는 색을 넘기고 PS는 보간된 input.color를 반환한다. 파일 이름/FXC Main/5.0/출력 경로는 유지한다. Vertex.h만 프로젝트/기존 Renderer 필터에 등록했다. 알파는 모두1이며 블렌딩/투명도 기능을 새로 추가하지 않았다. 예전 위치 전용 버퍼/고정 색상 PS 설명은 이전 단계다. 엔트리포인트는 복구한 FRenderer 조합 버전이며 색상 처리를 포함하지 않는다.

## RGBA8 텍스처 생성 함수 — 2026-10-16

D3D11Device.h/.cpp에 [[nodiscard]] CreateTexture2D(uint32 width,uint32 height,const TArray<uint8>& rgbaPixels,ComPtr<ID3D11Texture2D>& outTexture)를 추가했다. 준비된 장치, 각1~16384인 가로/세로와 정확히 width*height*4바이트인 행 패딩 없는 RGBA8 입력을 요구한다. 크기 상한을 먼저 검사한 뒤 rowPitch=width*4와 size_t 전체 바이트 수를 계산한다. RGBA8_UNORM·MipLevels1·ArraySize1·SampleCount1/Quality0·IMMUTABLE·SHADER_RESOURCE·CPU 접근/기타 플래그0으로 생성하고 지역 성공 참조만 출력으로 이동한다. 검사/API 실패는 로그/false이며 기존 출력은 유지한다. 반환 자원은 호출자가 소유하며 Device는 별도 참조나 CPU 데이터를 보관하지 않는다.

기존 main·Renderer·정점·제품 셰이더·프로젝트/빌드 설정은 변경하지 않았다. SHADER_RESOURCE 생성 플래그만 지정했고 SRV·샘플러·UV·이미지 파일 디코딩·제품 화면 연결은 아직 없다. 따라서 제품 화면은 기존 정점 색상 사각형 그대로다. 텍스처 포맷은 선형 UNORM이며 sRGB 뷰/색 관리·밉 생성·배열·멀티샘플·갱신 API는 이번 계약에 포함하지 않는다.

## World 부모 연결·분리 계약 — 2026-10-16

World.h/.cpp의 [[nodiscard]] bool SetActorParent(AActor* actor,AActor* newParent)와 Actor.h/.cpp의 [[nodiscard]] bool SetParent(AActor* newParent)를 제공한다. Actor는 GetWorld에 위임하며 friend class UWorld로 private 관계 접근을 허용한다. null 또는 살아 있는 객체가 인수 전제이고 원시 포인터 생존 검증/복구는 제공하지 않는다.

등록 검사는 함수 내부의 actors 주소 검색이다. null 대상·자기 자신·미등록 자신/새 부모·다른 World·자손 연결은 false다. 검사 뒤 같은 부모/이미 루트는 변경 없이 true이며, nullptr 부모는 루트 분리다. 미등록 Actor의 no-op도 false다. 기존 부모 목록에서 자신을 찾지 못해도 false다.

새 부모 배열의 용량을 먼저 준비한 뒤 기존 목록에서 제거하고 새 부모 끝에 추가한 다음 parent를 바꾼다. 같은 부모는 먼저 반환하므로 새 배열 reserve가 기존 배열 반복자를 무효화하지 않는다. 현재 erase/끝 추가 구현은 영구 형제 순서 보장으로 확정하지 않는다. false이면 기존 관계·소유·저장 Transform을 유지하며 할당 실패의 bool 복구는 제공하지 않는다.

액터 소유는 계속 World에 있고 저장 Transform/Component 값은 변경하지 않는다. 월드 Transform 계산·KeepWorld·제거/이벤트·별도 등록 조회·해시맵·핸들은 추가하지 않았다. 새 함수의 독립 계층 검사는 실행했지만 제품 main·게임/화면에 Actor 계층을 연결하지 않았다.

## 텍스처 기본 SRV 생성 함수 — 2026-10-16

Device.h/.cpp에 [[nodiscard]] CreateShaderResourceView(ID3D11Texture2D*,ComPtr<ID3D11ShaderResourceView>&)를 추가했다. 장치 미준비·null 텍스처·다른 장치 소속은 로그/false로 거부한다. GetDevice의 임시 참조는 ComPtr로 관리하고 설명 nullptr로 원래 형식/전체 범위의 기본 SRV를 요청한다. 실제 생성 API가 실패해도 기존 출력은 유지하며 성공한 지역 ComPtr만 출력으로 이동한다. Device가 SRV를 멤버로 보관하지 않고 호출자가 소유한다. 현재 생성한 RGBA8 단일 밉 텍스처에서는 TEXTURE2D/RGBA8_UNORM/MostDetailedMip0/MipLevels1이다.

main·Renderer·정점·제품 셰이더·프로젝트/공통 설정은 변경하지 않았다. SRV 생성만 추가했으며 셰이더 슬롯 연결·샘플러·UV·샘플링/화면·파일 디코딩은 아직 없다. 기본 설명으로 생성 가능한 텍스처가 대상이며 typeless 형식 선택/밉 일부 범위 등 사용자 지정 뷰 설명 API는 도입하지 않았다.

## 샘플러 상태 생성 함수 — 2026-10-16

Device.h/.cpp에 [[nodiscard]] CreateSamplerState(const D3D11_SAMPLER_DESC& description,ComPtr<ID3D11SamplerState>& outState)를 추가했다. 준비된 장치에서 설명을 그대로 D3D11 생성 API에 전달하고 성공한 지역 ComPtr만 출력으로 이동한다. 장치 미준비/API 실패는 로그/false이며 기존 출력은 유지한다. 반환 상태는 호출자가 소유하고 Device는 별도 멤버 참조나 설명 포인터를 보관하지 않는다. 세부 설정 유효성 검사는 D3D11 API가 맡는다.

필터링·주소 모드·LOD 등의 정책은 호출자가 정한다. 이번에 제품 기본 샘플러 정책을 고정하지 않았고, main·Renderer·텍스처·SRV·정점·제품 셰이더·프로젝트/공통 설정은 변경하지 않았다. 샘플러 생성만 추가했으며 슬롯 연결·실제 텍스처 샘플링·제품 화면 사용은 아직 없다.

## PS의 SRV·샘플러 슬롯0 연결 — 2026-10-16

Device.h/.cpp에 SetPixelShaderResourceView(ID3D11ShaderResourceView*)와 SetPixelShaderSamplerState(ID3D11SamplerState*)를 추가했다. 각각 준비된 즉시 컨텍스트에 PSSetShaderResources(0,1,&view), PSSetSamplers(0,1,&sampler)를 호출한다. PS t0/s0만 바꾸며 같은 장치의 유효한 자원을 전달하는 것은 호출자 책임이다. SRV는 출력에 겹쳐 연결된 자원이 아니어야 한다. 컨텍스트도 참조를 유지하므로 소유 ComPtr Reset만으로 연결을 해제할 수 없다.

nullptr는 해당 명시적 연결을 해제한다. sampler nullptr는 샘플링 비활성화가 아니라 D3D11의 기본 샘플러 상태를 사용한다는 뜻이며, 조회 API는 이 명시적 상태 없음에 대해 null을 반환한다. 두 함수는 생성/소유/유효성 순회 검사/자동 초기화/오류 bool 반환을 추가하지 않고 기존 Device 명령 함수의 준비 상태 전제를 유지한다.

main·Renderer·정점·제품 셰이더·프로젝트/공통 설정은 유지했다. 새 함수는 제품 프레임에서 아직 호출하지 않으며 제품 PS에 Texture2D/SamplerState 또는 UV 입력을 추가하지 않았다. 실제 샘플링/화면 연결과 종료의 새 슬롯 해제는 해당 제품 연결 단계에서 함께 구현한다. 현재 제품 화면은 기존 정점 색상 사각형 경로다.

## UV와 체크 텍스처 샘플링 연결 — 2026-10-16

이전 정점 색상 전용 단계를 확장했다. Math/Vector2.h의 FVector2는 float x/y와 기본값0만 제공한다. 당시 FVertex는 position·color·uv를 저장했고 기본 UV는(0,0), 배치는 위치0·색상12·UV28·총36바이트였다. 2026-10-18 공통 정점 준비로 끝에 FVector3 normal{}을 추가했다. 기존 초기화·오프셋은 유지하고 normal36·총48바이트이며 기본0은 유효한 표면 방향이 아니다. 같은 날 읽기 순서를 우선해 position·color·normal·uv로 정정하고 SampleGame의 네 정점 초기화도 함께 맞췄다. 현재 배치는 위치0·색상12·법선28·UV40·총48바이트다. 이전 끝 배치는 당시 호환성 선택이며 현재 순서는 구현 순서나 GPU 필수 규칙을 뜻하지 않는다. 법선은 아직 입력 레이아웃/셰이더에 전달하지 않는다. Device의 입력 레이아웃은 POSITION0 float3·COLOR0 float4·TEXCOORD0 float2이며 모두 슬롯0/정점별 입력이다. 오프셋은 offsetof, 정점 버퍼 크기와 stride는 sizeof(FVertex)로 계산한다. Vector2.h만 프로젝트와 기존 Math 필터에 추가했다.

FRenderer가 2×2 RGBA8_UNORM 단일 밉 불변 텍스처·기본 SRV·Point/Clamp 샘플러를 ComPtr로 소유한다. CPU 데이터는 위쪽 행부터 흰색255/회색64, 아래쪽은 회색64/흰색255이며 알파는 모두255다. 좌하·좌상·우상·우하 정점 UV는(0,1)·(0,0)·(1,0)·(1,1)이다. VS는 변환한 위치와 보간할 색상/UV를 출력한다. PSInput도 SV_POSITION·COLOR0·TEXCOORD0 순서를 맞추고 BaseTexture(t0).Sample(BaseSampler(s0),input.uv)*input.color를 반환한다.

Initialize에서 텍스처→SRV→샘플러 순서로 생성하며 어느 단계든 실패하면 Shutdown/false다. 샘플러 설명은 Point, U/V/W Clamp, MipLODBias0, MaxAnisotropy1, ComparisonNever, BorderColor0, MinLOD0, MaxLOD D3D11_FLOAT32_MAX다. 정책은 Renderer가 선택하고 Device 생성 함수는 범용 설명 전달 계약을 유지한다. RenderFrame에서 PS t0/s0를 연결하며 Shutdown은 해당 슬롯을 먼저 null로 해제한 뒤 샘플러·SRV·텍스처를 정리한다. 컨텍스트→SRV→텍스처 참조와 소유 ComPtr를 함께 정리하는 목적이다. 기존 main·윈도우·정점 위치/색·인덱스·X=0.25 변환·리사이즈 계약은 유지한다.

이미지 파일 로딩·에셋 계층·머티리얼·sRGB/색 관리·밉 생성·블렌딩·디버그 레이어는 추가하지 않았다. 이전의 UV/샘플링 미연결 설명은 각 단계 당시 상태이며 현재 경로는 실제 제품 셰이더로 샘플링한다.

## FImage와 PNG 파일 읽기 — 2026-10-16


Engine/Source/Image/Image.h의 FImage는 int32 sizeX/sizeY와 TArray<uint8> rawData를 보유한다. 기본값은0/0/빈 배열이다. 정상 로드 결과는 단일2D 이미지의 위쪽 행부터 R/G/B/A 순서로 저장한 행 패딩 없는 RGBA8이며 길이는 sizeX*sizeY*4다. 알파를 미리 곱하지 않는다. 파일 경로·GPU 자원·색상 공간 메타데이터는 보관하지 않는다.

Image/ImageUtils.h/.cpp의 FImageUtils::LoadImageFile(const std::filesystem::path&,FImage&)은 Windows WIC로 단일 프레임 PNG를 읽는다. 파일 이름을 WIC에 직접 전달하고 컨테이너 GUID를 PNG로 제한하므로 확장자만으로 형식을 판정하지 않는다. 빈 경로/내장 NUL은 거부한다. 상대 경로는 현재 작업 폴더 기준이며 실행 파일 기준 경로를 자동으로 붙이지 않는다. 실제 제품의 이미지 위치/배포/기준 경로는 다음 연결 단계에서 결정한다.

처리는 COM 준비→WIC factory→파일 decoder→PNG/프레임 검사→크기/버퍼 범위 검사→32bppRGBA converter→CopyPixels→완성된 지역 FImage 이동 순서다. 각 차원을 먼저 양수 int32 범위로 제한하고 uint64로 stride=width*4·전체 크기를 계산한다. CopyPixels 인수의 UINT 범위와 배열 max_size를 넘으면 할당 전에 거부한다. 이 CPU 읽기 함수는 D3D11 최대 텍스처 차원에 결합하지 않는다. GPU 업로드 시 Device의 별도 크기 제한을 적용해야 한다.

일반 검사/WIC API 실패는 로그와 false를 반환하며 기존 출력의 크기/배열을 유지한다. 성공한 지역 결과만 std::move로 출력에 옮긴다. 메모리 부족은 기존 예외 비활성화/배열 정책을 따르며 false 복구를 보장하지 않는다. WIC가 복구해 받아들이는 일부 손상/잘림 PNG까지 엄격히 거부한다는 보장은 없다. 파일 전체 무결성 검사기나 애니메이션 PNG 검증기는 구현하지 않았다. GIF/JPEG/BMP 컨테이너는 지원 대상이 아니다.

매 호출 CoInitializeEx(MTA)를 시도한다. S_OK/S_FALSE는 지역 WIC 참조가 모두 파괴된 뒤 CoUninitialize로 대응한다. RPC_E_CHANGED_MODE이면 기존 apartment를 유지한 채 WIC 사용을 시도하고 그 실패한 초기화에는 대응 해제를 하지 않는다. 그 밖의 초기화 실패는 로그/false다. 전역 COM 소유자·장치·이미지 모듈·가상 인터페이스는 추가하지 않았다.

프로젝트/필터에 Image.h·ImageUtils.h/.cpp와 Image 필터를 등록하고 프로젝트 링크에 windowscodecs.lib·ole32.lib만 추가했다. 외부 패키지/공통 빌드 설정은 바꾸지 않았다. 이름은 이전 엔진에서 사용한 일반 이미지 보조 함수 묶음 FImageUtils를 유지했다. 현재 함수는 파일 읽기 하나이며 GPU 텍스처 생성이나 renderer 호출은 포함하지 않는다. 제품은 기존 사용자 확인 완료 체크무늬 경로를 유지한다.

## PNG 파일을 FRenderer 출력에 연결 — 2026-10-16

코드로 만든 체크 픽셀 배열을 Engine/Content/Textures/Checker.png 원본 파일로 옮겼다. 2×2 RGBA8 PNG이며 위쪽 행 흰색255/회색64, 아래쪽 회색64/흰색255·알파255로 이전 배열과 같다. 화면의 의도된 모양은 유지하고 데이터 공급 경로만 파일 읽기로 바꿨다. GPU 자원 소유·UV·셰이더·색상 곱·Point/Clamp·행렬·크기 변경·Shutdown 흐름은 유지한다.

SampleGame.vcxproj는 Content 항목으로 원본 PNG를 등록하고 SECopyEngineTexture(AfterTargets=Build)의 Copy로 $(OutDir)Content/Textures에 복사한다. 프로젝트 필터에는 Engine/Content/Textures로 표시한다. 복사 대상은 이 PNG 하나이며 전체 Content 검색/패키징 계층은 추가하지 않았다. 빌드가 실제 실행되면 원본 누락은 Copy 오류로 실패하며 디렉터리는 Copy가 만든다. SkipUnchangedFiles=true는 크기와 마지막 수정 시각이 같은 파일을 건너뛴다. 해시 동기화가 아니며 같은 크기/시각을 의도적으로 유지한 내용 변경 감지나 제거된 파일 정리는 보장하지 않는다. Visual Studio의 최신 상태 판정으로 MSBuild 자체를 생략한 경우까지 이 target 실행을 보장하지 않는다. 첫 실행은 재빌드로 갱신한다.

당시 FRenderer::Initialize는 FPaths::BaseDir로 얻은 실행 파일 부모에 Content/Textures/Checker.png를 합친 명시적 std::filesystem::path를 FImageUtils::LoadImageFile에 전달한다. 정상 FImage의 sizeX/sizeY를 uint32로 명시 변환하고 rawData와 함께 기존 Device::CreateTexture2D에 전달한다. 디코더의 정상 결과가 양수 int32라는 전제와 Device의1~16384 검사·정확한 바이트 수 계약을 재사용한다. 로드 실패는 Shutdown/false이며 코드 생성 체크무늬로 자동 대체하지 않는다. 호출자는 기존대로 초기화 실패 시 종료한다.

당시 CPU image는 초기화의 지역 값이고 성공 후 사라져도 GPU 텍스처/SRV는 Renderer의 ComPtr가 소유했다. FImage를 프레임마다 읽거나 보관하지 않는다. 다른 PNG를 표시하려면 출력 파일을 바꾼 뒤 초기화/재실행해야 하며 실시간 재로딩은 없다. 셰이더 경로와 마찬가지로 실행 파일 기준이라 현재 작업 폴더에 의존하지 않는다. 코드 컨벤션3.5에 따라 같은 함수의 baseDir는 std::optional<std::filesystem::path>, shaderDir/texturePath는 std::filesystem::path로 명시했다. 해당 경로 변수는 auto 허용 예외가 아니다.

2026-10-18 책임/연결 정정: 체크 파일 선택·로딩은 SampleGame::ConfigureGameWorld로 이동했다. UStaticMeshComponent::SetBaseColorImage는 optional<FImage>에 CPU 복사본을 보관하고 dirty를 표시한다. 생성/갱신은 Scene을 거쳐 FRenderer::CreateMesh(meshData,outMesh,const FImage* baseColorImage=nullptr)에 호출 중 포인터를 전달한다. 새 오버로드는 추가하지 않았다. 지정 이미지까지 생성한 뒤 출력 GPU 메시를 교체하고 실패 시 기존 GPU 자원을 유지한다. nullptr면 기본 흰색이며 잘못 지정한 이미지는 실패다. Renderer의 전역 체크 텍스처/뷰와 파일 읽기는 제거했고 메시별 FTexture2D 또는 흰색을 연결해 정점 색상과 곱한다. Component는 재생성을 위해 CPU 이미지를 유지하며 실패 시 CPU 설정과 dirty도 유지한다. 이미지 제거 API·공유 캐시·부분 갱신은 추가하지 않았다. 기존 Content 복사 설정과 모델 경로·셰이더는 유지한다.

## FMeshData와 실제 인덱스 개수 — 2026-10-16

Renderer/MeshData.h에 FMeshData를 추가했다. TArray<FVertex> vertices·TArray<uint32> indices 두 CPU 배열을 소유하며 기본 상태는 빈 배열이다. 인덱스 값은 vertices의 요소 번호를 나타낸다. UObject 상속·GPU 자원·생성 함수·파일 경로·자동 검증·재질·Transform은 추가하지 않는다. 데이터 보관 형식이며 그 자체가 GPU 메시나 파일 에셋 객체는 아니다.

FRenderer::Initialize의 기존 정점4개와 인덱스 {0,1,2,0,2,3}를 const FMeshData quadMesh로 묶었다. Device::CreateVertexBuffer에는 quadMesh.vertices, CreateIndexBuffer에는 quadMesh.indices를 전달한다. 정점 위치/색상/UV·인덱스 순서는 유지한다. GPU 버퍼는 계속 FRenderer의 ComPtr가 소유하며 이 지역 CPU 데이터는 초기화 종료 시 사라진다. 버퍼 생성은 데이터를 GPU 자원에 복사하므로 프레임에서 지역 배열 주소를 보관하지 않는다.

FRenderer에 uint32 indexCount=0을 추가했다. 모든 자원/스왑 체인 초기화 성공 후 quadMesh.indices.size()를 static_cast<uint32>로 저장하고, RenderFrame은 DrawIndexed(indexCount,0,0)을 호출한다. 기존 고정 인수6을 제거했다. CreateIndexBuffer가 이미 indices.size()<=UINT_MAX/sizeof(uint32)를 검사하고 성공한 뒤에만 변환하므로 개수가 uint32에 들어간다. 현재 정점/인덱스가 고정된 유효한 사각형이라는 전제는 유지한다. 별도 인덱스 범위 검증/일반 메시 입력 API를 추가하지 않았다.

Shutdown은 indexCount를0으로 되돌린다. 중복 초기화 거부는 기존 자원과 개수를 유지하고, 중간 실패는 기존 Shutdown 경로로 정리한다. 프로젝트와 기존 Renderer 필터에 MeshData.h만 등록했다. Renderer.h는 uint32를 직접 사용하므로 CoreTypes.h를 포함한다. PNG 파일·셰이더·D3D11 API·버퍼/텍스처 소유·입력 레이아웃·리사이즈/정리·공통 설정은 유지한다. 사각형 데이터를 렌더러 밖에서 받는 기능은 아직 없다.

## 외부 FMeshData를 받는 SetMesh — 2026-10-16


FRenderer.h/.cpp에 [[nodiscard]] bool SetMesh(const FMeshData& meshData)를 추가했다. 헤더는 FMeshData를 전방 선언하고 구현에서 기존 MeshData.h를 사용한다. 초기화된 렌더러의 렌더링과 같은 스레드에서 프레임 사이에 호출한다. 성공하면 다음 RenderFrame부터 새 메시를 그린다. 미초기화/Shutdown 후·빈 정점·빈 인덱스·3의 배수가 아닌 인덱스 개수·vertices.size() 이상인 인덱스 값은 로그/false다. 삼각형 목록 topology에 맞춘 계약이며 빈 메시로 그리기를 해제하는 API는 아니다.

검증 후 지역 ComPtr 두 개에 Device::CreateVertexBuffer·CreateIndexBuffer를 호출한다. 두 생성이 모두 성공한 뒤에만 멤버 vertexBuffer·indexBuffer를 std::move로 교체하고 실제 indices.size()를 uint32 indexCount에 저장한다. 일반 검사/자원 생성 실패는 기존 멤버/개수를 유지한다. 정점 생성 성공 후 인덱스 생성 실패라면 지역 새 정점 버퍼만 자동 정리된다. 기존 Device 생성 함수의 크기 검사/출력 보존 계약을 재사용하며 인덱스 개수 변환은 성공한 ByteWidth 범위 검사 뒤에 수행한다. 여러 스레드에 대한 원자성이나 GPU 작업 완료 대기를 제공한다는 뜻은 아니다.

SetMesh는 즉시 IA 연결을 바꾸지 않는다. 이전 프레임에 연결된 버퍼의 컨텍스트 참조는 다음 RenderFrame의 기존 SetVertexBuffer/SetIndexBuffer 연결 또는 Shutdown의 해제까지 유지될 수 있다. 멤버 소유 참조 교체와 컨텍스트 참조 교체는 다른 시점이다. 여러 번 SetMesh를 호출하면 다음 프레임은 마지막 성공 메시를 사용한다. Copy/업로드가 완료된 호출자의 CPU 데이터는 보관하지 않으며 호출 후 수정/해제할 수 있다. GPU 버퍼는 계속 FRenderer가 소유한다.

Initialize의 기본 사각형 생성은 유지했고 새 API를 제품 main/게임/Actor/Component에 연결하지 않았다. SetMesh는 현재 한 메시를 교체할 뿐 여러 메시 목록·메시 에셋 객체·동적 버퍼 갱신·Transform/재질/텍스처 변경·게임 확장 지점을 만들지 않는다. 기존 행렬/PNG/SRV/샘플러/셰이더/DrawIndexed/리사이즈/종료 계약은 유지한다. FMeshData 자체는 보관 형식이며 이번 검증 책임은 사용하는 SetMesh에 둔다. 퇴화 삼각형·정점 NaN/무한대·감김 방향 등의 추가 유효성 검사는 하지 않는다.

## FStaticMeshData로 이름 변경 — 2026-10-16

이전→이후: Renderer/MeshData.h의 FMeshData를 Renderer/StaticMeshData.h의 FStaticMeshData로 변경했다. 정점/인덱스의 구성·기본값·소유·메모리 표현은 유지한다. FRenderer의 전방 선언·기본 사각형 지역 타입·SetMesh 인수 타입·include와 프로젝트/필터의 헤더 경로를 새 이름으로 맞췄다. 함수 이름 SetMesh와 매개변수 이름 meshData, 버퍼 생성/검증/교체/그리기/정리 로직은 유지했다. 2026-10-18 공통 형상 준비로 FStaticMeshData에 int32 materialIndex=-1을 추가했다. 함께 사용하는 재질 목록의 번호 하나이며 -1은 미지정이다. 기존 두 배열 초기화는 유지되고 생략한 번호는 -1이다. 현재 Renderer는 이 번호를 읽지 않으며 모델의 FModelMeshData/FModelVertex 제거와 재질 선택 연결은 후속 작업이다. 과거 이름의 별칭이나 중복 헤더는 남기지 않았으며 호출자는 새 타입/헤더를 사용한다.

Static은 정적 형상용 CPU 정점·인덱스 묶음이라는 용도다. const 강제·메시 교체 금지·Actor 이동/회전/크기 변경 금지를 뜻하지 않는다. SetMesh로 데이터를 교체하는 기존 계약은 그대로다. 현재는 GPU 자원/애니메이션/스키닝/파일 자산 정보를 보관하지 않는다. FStaticMeshData는 UObject나 Unreal UStaticMesh 자산을 재현한 타입이 아니다.

FStaticMeshComponent라는 후속 컴포넌트 이름에 맞춰 데이터 이름도 구체화하는 선택이다. 이번에는 데이터 이름만 변경했고 컴포넌트 자체는 추가하지 않았다. 기본 PNG 사각형·인덱스 개수·GPU 자원 소유·API 실패 보존·게임/Actor 미연결 상태는 유지한다.

## Component의 UObject 상속과 U 접두사 — 2026-10-16

이전→이후: 독립 다형 기반 FActorComponent를 UObject를 public 상속하는 UActorComponent로 바꾸고, 그 파생 FTransformComponent를 UTransformComponent로 바꿨다. ActorComponent.h는 Object.h를 직접 포함하고 소멸자는 ~UActorComponent() override로 선언한다. ActorComponent.cpp·TransformComponent.h/.cpp의 생성자/함수 정의, Actor.h/.cpp의 기본 생성·단독 소유 배열·AddComponent·GetComponent/GetComponents 제약/조회와 World.cpp의 Transform 조회를 새 이름으로 맞췄다. 파일 이름·프로젝트 등록 경로는 유지한다.

현재 계층은 UObject→UActorComponent→UTransformComponent다. 기존 UObject 파생인 AActor는 Actor 계열 접두사 예외로 A를 유지한다. FStaticMeshData·FTransform·FVector 등 CPU 값 타입은 UObject를 상속하지 않으므로 F를 유지한다. 후속 컴포넌트의 목표 이름은 UStaticMeshComponent이며 이번에는 생성하지 않았다.

U는 UObject의 직접/간접 상속을 뜻한다. 현재 UObject는 가상 소멸자만 있는 최소 기반이다. GC·리플렉션·객체 등록·Outer·자동 이벤트·Component Tick을 추가하지 않았다. Actor는 TUniquePtr<UActorComponent> 배열로 컴포넌트를 단독 소유하고 owner는 비소유 AActor&로 유지한다. 소속 Actor가 컴포넌트보다 오래 살아야 하며 생성 자체의 자동 등록은 없다. UObject 상속이 수명 연장이나 공유 소유를 제공하지 않는다.

Actor 생성 시 기본 UTransformComponent 하나를 소유 목록에 넣는 동작·동일 실제 타입의 복수 허용 정책·실패 시 AddComponent 입력 보존·GetComponent/GetComponents의 비소유 포인터·기본 로컬/월드 Transform 계산·World의 관계/제거/Tick 흐름은 유지한다. 기반 포인터 파괴는 기존 UActorComponent*뿐 아니라 UObject*에서도 파생 소멸자를 호출할 수 있다. Getter의 타입 제약은 여전히 컴포넌트 자신/파생 타입이고 GetComponent<UObject>는 허용하지 않는다.

## UStaticMeshComponent의 CPU 메시 보관 — 2026-10-16

Engine/Source/StaticMeshComponent.h/.cpp에 UActorComponent를 상속하는 UStaticMeshComponent를 추가했다. 생성자는 AActor&를 기존 기반 생성자로 전달하고 기본 FStaticMeshData의 두 배열은 비어 있다. 생성만으로 Actor 목록에 등록하거나 GPU 상태를 만들지 않는다. Actor에 붙일 때는 기존 TUniquePtr<UActorComponent>·AddComponent 소유 이전 API를 사용한다. Actor의 기본 컴포넌트는 계속 UTransformComponent 하나이며 StaticMeshComponent를 자동 생성하지 않는다.

SetMeshData(const FStaticMeshData&)는 CPU 데이터를 멤버에 복사하고 GetMeshData() const는 const FStaticMeshData&를 반환한다. 정점 위치/색/UV와 인덱스를 독립적으로 보관하며 입력 배열 주소를 장기 저장하지 않는다. 컴포넌트마다 자체 CPU 복사본을 소유하므로 원본 수정/clear나 다른 컴포넌트의 변경이 저장 데이터에 영향을 주지 않는다. getter는 새 복사본이 아닌 멤버의 읽기 전용 참조를 빌려준다. 컴포넌트 파괴 후 무효이며 setter 뒤 배열 원소 포인터/참조는 다시 얻어야 한다.

setter는 void이며 빈 데이터와 아직 유효성을 검사하지 않은 데이터도 보관한다. 논리적으로 빈 데이터 저장은 허용하지만 배열 용량을 반드시 반환한다는 보장은 없다. 검사/GPU 생성/오류 bool은 기존 FRenderer::SetMesh 책임이다. 컴포넌트의 빈 데이터 저장을 현재 GPU 메시 해제나 화면 지우기로 해석하지 않는다. 실제 renderer 호출/빈 데이터 처리/갱신 전달은 후속 연결에서 결정한다. 메모리 부족은 기존 예외 비활성화/배열 정책을 따르며 실패 bool 복구를 제공하지 않는다.

AllowsMultipleComponents는 재정의하지 않아 동일 실제 타입의 복수 추가 불허 기본 정책을 따른다. 서로 다른 파생 실제 타입은 기존 정책상 별개이며 상속 그룹 전체를 하나로 제한하지 않는다. GetComponent/GetComponents에서 기반 UStaticMeshComponent로 파생 타입까지 조회할 수 있다. Actor가 소유하면 Actor 제거와 함께 컴포넌트/CPU 배열이 정리된다. owner는 비소유 참조이며 생성/조회만으로 객체 수명이 연장되지 않는다. UObject 기반 가상 소멸은 상속하며 GC/등록/Component Tick을 추가하지 않았다.

프로젝트/기존 Engine/Source 필터에 새 헤더와 cpp를 각1회 등록했다. 별도 폴더·GPU 자원·UStaticMesh 자산 클래스·공유 소유·재질·Transform 저장·렌더러 참조·dirty flag·이벤트·파일 읽기·main/게임 연결은 추가하지 않았다. Actor/World/Transform/Renderer와 기존 GPU 출력 흐름은 유지한다.

## 객체·뷰 행렬 입력 분리 — 2026-10-16

FRenderer.h/.cpp에 [[nodiscard]] bool SetLocalToWorld(const FMatrix4& matrix), SetWorldToClip(const FMatrix4& matrix)를 추가했다. 초기화 후 렌더링과 같은 스레드의 프레임 사이에 호출한다. LocalToWorld는 객체 로컬→월드, WorldToClip은 월드→클립의 완성된 행렬을 받는다. 렌더러가 두 FMatrix4를 값 멤버로 보관하고 Actor/Component/카메라 주소나 입력 참조를 장기 보관하지 않는다. FMatrix4를 값 멤버로 쓰므로 Renderer.h가 Math/Matrix4.h를 직접 포함한다.

우리 행 벡터 규칙의 합성 순서는 LocalToClip=LocalToWorld*WorldToClip이다. SetLocalToWorld는 새 입력*보관한 WorldToClip, SetWorldToClip은 보관한 LocalToWorld*새 입력을 계산한다. 기존 Device::UpdateConstantBuffer로 지역 합성 결과64바이트를 전달한 뒤 성공한 경우에만 해당 CPU 멤버를 입력 값으로 바꾼다. false이면 기존 보관 값을 유지한다. 초기화 전/Shutdown 후에는 로그와 false이며 갱신하지 않는다. 성분의 유한성·역행렬 가능성·투영 행렬 의미를 검사하지 않는다.

Initialize는 기존 X=0.25 이동을 initialLocalToWorld로 만들고 initialWorldToClip은 SetIdentity로 명시 설정해 합성/초기 업로드한다. 모든 자원/스왑 체인 준비 성공 후 두 CPU 값과 bInitialized를 확정한다. 중복 초기화 거부는 그 전에 반환하므로 기존 행렬을 보존한다. Shutdown은 두 CPU 값을 FMatrix4{}의0 행렬로 되돌리고 준비 상태를 해제한다. FMatrix4 기본값은 항등이 아니라0이며 준비 전 값은 그리기에 사용하지 않는다. 재초기화는 다시 기본 이동/항등 뷰를 설정한다.

GPU 쪽은 기존 VS b0/64바이트/row_major LocalToClip 하나를 유지한다. 셰이더의 mul(float4(position,1),LocalToClip), Device API, 버퍼 소유/연결/정리, 메시/PNG/인덱스/입력 레이아웃은 바꾸지 않았다. true는 UpdateSubresource 갱신 요청까지 수락했다는 뜻이며 GPU 완료나 실제 화면 표시 성공을 보장하지 않는다. UpdateSubresource 자체는 HRESULT를 반환하지 않는 기존 계약을 따른다.

객체와 카메라 입력의 책임을 구분한 단계이며 카메라 클래스·View/Projection 생성·Actor Transform 자동 수집·컴포넌트 렌더 등록·게임 호출·다중 메시 장면은 추가하지 않았다. 현재 WorldToClip 기본 항등은 기존 카메라 없는 시연 공간을 유지하는 값이지 일반적인 카메라 투영 구현이 아니다. 제품 main은 두 setter를 새로 호출하지 않고 기존 기본 사각형을 계속 그린다.

## StaticMeshComponent의 LocalToWorld 조회 — 2026-10-16

UStaticMeshComponent::GetLocalToWorld() const는 소속 AActor의 필수 기본 UTransformComponent를 조회하고 GetComponentTransform().ToMatrixWithScale()로 부모 합성이 반영된 로컬→월드 FMatrix4를 값으로 반환한다. 기본 Actor에서는 항등 행렬이다. 헤더는 반환 값의 완전한 타입을 위해 Math/Matrix4.h를 직접 포함하고, cpp가 Actor.h/TransformComponent.h를 포함한다.

별도의 Transform/캐시를 저장하지 않아 다음 조회는 부모 이동·회전·크기·부모 변경을 반영한다. 같은 Actor의 메시 컴포넌트는 같은 기본 Transform을 사용하며 컴포넌트별 추가 오프셋은 없다. 기존 FTransform의 유한 값/정규화 회전 전제와 TRS·비균일 크기/회전 합성의 전단 표현 한계를 그대로 따른다. 모든 부모 조합을 정확한 행렬 곱으로 재현한다는 뜻은 아니다.

메시 정점·색·UV·인덱스는 로컬 데이터로 유지하고 반환 행렬만 별도로 제공한다. 반환 값을 수정해도 Actor/컴포넌트는 바뀌지 않는다. owner는 비소유 참조이며 호출 동안 Actor와 기본 Transform이 살아 있어야 한다. Actor 생성자의 필수 Transform과 제거 API 부재 계약을 사용하므로 누락 시 항등으로 숨기는 분기는 추가하지 않았다.

## World의 등록 Actor 조회 — 2026-10-16

World.h/.cpp에 void UWorld::GetActors(TArray<AActor*>& outActors) const를 추가했다. 출력 배열을 clear한 뒤 World의 소유 배열을 등록 순서대로 읽어 각 unique_ptr의 get() 주소를 push_back한다. 루트뿐 아니라 등록된 자식/파생 Actor도 포함하고 부모 트리 순서로 정렬하지 않는다. 생성만 하고 등록하지 않은 Actor와 다른 World의 Actor는 포함하지 않는다.

제거 요청/제거 상태로 필터링하지 않는다. 아직 등록된 대상은 제거가 보류돼도 포함하고 실제 등록 해제 뒤 다시 조회하면 제외한다. 이전에 얻은 배열은 자동 갱신되지 않는다. const World에서도 수정 가능한 비소유 AActor*를 제공하며 소유권이나 객체 수명을 연장하지 않는다. 개별 Actor 또는 World의 실제 파괴 후 해당 주소를 역참조하면 안 된다. 출력 배열 수정은 World 소유 목록을 바꾸지 않는다.

호출할 때 기존 원소는 지우고 배열 용량은 재사용한다. 용량이 부족하면 일반 TArray 할당 정책을 따르며 메모리 부족의 bool 복구는 없다. 조회 중 등록 배열을 동시에 변경하는 스레드 안전성은 제공하지 않는다. 조회 목록을 순회하며 Actor를 제거하면 자손 등 다른 항목도 무효가 될 수 있으므로 이 목록을 수명 보장 순회 장치로 해석하지 않는다.

## 제거 요청 보류 구현과 문서의 불일치 수정 — 2026-10-16

GetActors 단계에서 확인한 World.cpp와 World.h/Actor.h/문서의 불일치를 수정했다. 이전 구현은 Tick 중 요청 즉시 현재 자손까지 bActorIsBeingDestroyed=true로 표시하고 Purge가 표시된 Actor를 찾았다. 이후 구현은 기존 문서 결정대로 pendingDestroyActors에 요청 대상만 중복 없이 보관하고, Purge가 요청을 꺼내 처리한다. 정책을 새로 바꾼 것이 아니라 기록된 계약에 구현을 맞췄다. 불일치가 발생한 경위는 이번 작업에서 확인하지 않았다.

DestroyActor는 기존 null/미등록/다른 World 검증과 Tick 밖 즉시 제거를 유지한다. Tick/종료 정리 중에는 이미 실제 제거 중인 등록 대상 또는 이미 보관한 대상의 반복 요청을 true로 받고, 새 요청만 배열에 추가한다. 요청만 보류된 Actor와 자손은 제거 상태를 켜지 않아 같은 Tick의 자기 차례와 기존 부모 변경을 허용한다. 따라서 요청 후에도 같은 프레임에 추가 게임 동작이 발생할 수 있다.

PurgeDestroyedActors는 pendingDestroyActors가 빌 때까지 back()/pop_back()으로 요청을 꺼낸 뒤 DestroyActorImmediately에 전달한다. 실제 처리 시점의 현재 자손을 수집하므로 요청 뒤 밖으로 이동한 자손은 자기 요청이 없다면 생존하고 새로 붙은 자손은 함께 제거된다. 요청 대상 자신의 부모 변경은 요청을 취소하지 않는다. 여러 요청 사이의 처리 순서를 공개 계약으로 보장하지 않는다.

DestroyActorImmediately는 기존 관계 검증 뒤 함께 파괴할 subtree의 모든 대기 요청을 std::erase_if로 지운다. 소멸자 호출 전에 이 정리를 끝내므로 소멸 중 같은 주소에 새 Actor를 생성해도 이전 요청이 새 객체에 적용되지 않는다. 이후 기존 상태 표시·부모/자식 분리·소유 배열 정리·자손부터 실제 파괴 순서를 유지한다. 실제 파괴 뒤 외부 주소는 무효다. GetActors는 아직 등록된 보류 대상을 포함하고 실제 등록 해제 뒤 제외하는 기존 계약을 유지한다.

## 렌더러 프레임 시작·그리기·표시 분리 — 2026-10-16

Renderer.h/.cpp에 [[nodiscard]] bool BeginFrame(int width,int height), DrawMesh(), EndFrame()을 추가했다. 이전 RenderFrame의 크기 대응/RTV·viewport·clear → VS/PS/texture/sampler/상수·정점·인덱스 바인딩과 DrawIndexed → Present를 각각 나눴다. 기존 RenderFrame은 BeginFrame→DrawMesh→EndFrame을 차례로 호출하는 단일 메시 편의 함수로 유지했다. EngineMain.cpp는 그대로 같은 함수를 호출한다.

새 bool bFrameActive는 기본 false다. BeginFrame은 초기화 전·이미 활성 프레임·0 이하 크기를 로그와 false로 거부한다. 거부 시 기존 활성 프레임은 유지하고 resize/clear/Present를 하지 않는다. 크기 변화 시 기존처럼 RTV를 해제하고 Resize 성공 후 크기를 저장한다. Resize 실패 시 활성 상태로 들어가지 않는다. 정상 경로는 타깃/viewport 설정과 배경색 clear를 한 번 수행한 뒤 true로 바꾼다.

DrawMesh는 활성 프레임이 없으면 로그/false다. 활성 프레임에서는 현재 메시·행렬·기존 PNG/셰이더 상태로 바인딩하고 DrawIndexed를 요청하며 true를 반환한다. 반복 호출할 수 있고 clear/resize/Present는 하지 않는다. true는 그리기 명령 요청이며 GPU 완료나 화면 표시 보증이 아니다. EndFrame은 활성 프레임이 없으면 로그/false이고, 활성 상태를 먼저 false로 바꾼 뒤 Present의 bool을 반환한다. Present 실패 후 반복 EndFrame으로 재표시하지 않는다. DrawMesh를 호출하지 않은 빈 프레임도 허용한다.

Shutdown은 기존 자원 정리 뒤 bFrameActive도 false로 만든다. 진행 중 프레임은 Present 없이 중단하며 다음 초기화는 비활성 상태다. 프레임 API와 setter는 같은 스레드에서 순서대로 호출한다. Resize/Present 등 프레임 처리 실패 시 기존 정책대로 반복을 중단하고 Shutdown한다. 잘못된 호출 순서 거부를 자동 오류 복구/프레임 롤백으로 해석하지 않는다.

SetMesh와 두 행렬 setter는 기존 구현 그대로이며 프레임 사이뿐 아니라 DrawMesh 호출 사이에도 사용할 수 있다고 주석을 맞췄다. 성공한 새 메시/행렬은 다음 DrawMesh부터 사용한다. SetMesh는 호출마다 새 GPU 정점/인덱스 버퍼를 생성하는 기존 방식이다. 이번 단계는 같은 메시를 다른 변환으로 여러 번 그리는 기능이며 여러 메시의 GPU 자원을 효율적으로 보관/선택하는 구조는 후속이다.

## 메시별 GPU 자원 소유와 선택 그리기 — 2026-10-16

Renderer/StaticMeshRenderData.h에 FStaticMeshRenderData를 추가했다. 비공개 ComPtr<ID3D11Buffer> 정점/인덱스 버퍼와 uint32 인덱스 개수를 소유하며 FRenderer만 friend로 생성·검사·바인딩한다. 기본값은 빈 자원이다. UObject 상속이 없는 일반 자원 타입이므로 F 접두사를 쓴다. 복사 생성/대입은 삭제하고 이동 생성/대입은 기본 구현을 사용한다. 기본 이동은 정수 개수를 원본에도 남길 수 있지만 ComPtr는 이전되므로 원본은 버퍼 검사에서 그리기를 거부한다. 새 헤더는 WindowsHWrapper를 직접 포함하며 프로젝트/기존 Renderer 필터에 각1회 등록했다.

FRenderer::CreateMesh(const FStaticMeshData&,FStaticMeshRenderData&)는 초기화 후 같은 스레드에서 호출한다. 기존 SetMesh의 비어 있지 않은 정점/인덱스·3의 배수·범위 내 인덱스 검사와 Device의 버퍼 크기/API 검사를 사용한다. 지역 newMesh에 정점/인덱스 버퍼를 모두 만든 뒤 개수를 설정하고 출력에 이동 대입한다. 실패하면 지역 부분 자원만 정리되고 기존 출력은 유지한다. CPU 데이터는 GPU로 복사하며 호출 후 원본을 보관할 필요가 없다. CreateMesh는 내부 기본 메시 선택을 변경하지 않는다.

FRenderer::DrawMesh(const FStaticMeshRenderData&)는 활성 프레임·두 버퍼 존재·0이 아닌 인덱스 개수를 검사한다. 두 버퍼의 GetDevice로 얻은 장치가 현재 렌더러 장치와 같은지 확인하고, 모두 통과한 뒤 기존 셰이더/공통 텍스처/상수/레이아웃과 입력 메시의 버퍼/개수를 사용해 DrawIndexed한다. 실패 시 새 바인딩이나 그리기를 요청하지 않는다. 행렬은 기존 SetLocalToWorld/SetWorldToClip 설정을 사용하며 clear/Present는 하지 않는다. true는 GPU 명령 요청이지 완료나 제품 화면 표시 성공이 아니다.

호출자가 외부 GPU 메시를 소유하고 렌더러는 호출 중에만 입력 객체를 빌린다. 객체 주소를 장기 보관하지 않지만 D3D11 컨텍스트는 바인딩한 버퍼의 COM 참조를 유지할 수 있다. CPU 메시와 GPU 메시 사이 자동 변경 추적은 없으며 CPU 데이터 변경 후에는 명시적으로 다시 CreateMesh해야 한다. 외부 자원은 소멸/이동 대입/빈 값 대입 때 참조를 해제한다. Renderer::Shutdown은 내부 기본 메시와 바인딩만 정리하며 외부 메시를 비우지 않는다. 외부 버퍼가 남으면 생성 장치의 수명/메모리도 유지될 수 있다. 장치가 바뀐 재초기화에서는 이전 메시를 그리려 하지 말고 다시 생성한다.

기존 Renderer의 vertexBuffer/indexBuffer/indexCount 멤버는 FStaticMeshRenderData defaultMesh로 묶었다. Initialize는 기존 사각형 버퍼와 개수를 기본 메시로 준비하고 Shutdown에서 빈 자원으로 되돌린다. SetMesh는 CreateMesh(data,defaultMesh), 인수 없는 DrawMesh는 DrawMesh(defaultMesh)로 위임한다. RenderFrame·Begin/End·EngineMain·기본 메시/PNG/행렬·셰이더는 기존 동작을 유지한다.

## Scene의 정적 메시 등록·행렬 갱신·제거·그리기 — 2026-10-16

Renderer/Scene.h/.cpp에 FScene을 추가했다. private 중첩 FStaticMeshEntry는 uint64 meshId·FStaticMeshRenderData·FMatrix4 localToWorld를 보관하고 TArray가 등록 순서대로 단독 소유한다. FScene은 일반 F 타입이며 복사/이동을 금지했다. Actor·Component·World·Renderer 주소나 CPU 원본 데이터는 저장하지 않는다. 모든 API는 렌더링과 같은 스레드에서 사용하고 RenderFrame 순회 도중 Scene을 변경하지 않는다.

AddStaticMesh(FRenderer&,const FStaticMeshData&,const FMatrix4&,uint64&)는 ID 고갈을 먼저 검사하고 지역 항목의 GPU 데이터를 renderer.CreateMesh로 준비한다. 성공하면 행렬을 복사하고 소유 배열에 이동 추가한 뒤 outMeshId를 확정하고 다음 ID를 증가시킨다. 반환 false이면 기존 Scene/출력 ID/다음 ID는 유지한다. 메모리 부족의 복구는 기존 예외 비활성화/배열 정책을 따르며 bool로 보장하지 않는다. 같은 메시 데이터도 호출마다 별개 ID/버퍼를 만들며 자동 중복 제거는 없다.

ID는 Scene 인스턴스 안에서만 유효한 uint64 값이다. 1부터 발급하고 제거 후 재사용하지 않으며 0은 무효다. 마지막 유효 ID 발급 뒤 unsigned 증가로 nextMeshId가0이 되면 이후 등록은 로그/false다. 서로 다른 Scene의 같은 숫자 ID는 구분하지 못하므로 항상 발급한 Scene과 함께 사용한다. 기본 nextMeshId=1이며 전체 항목을 제거해도 초기화하지 않는다. 성공 여부와 성공 시에만 갱신하는 출력 ID를 나누는 이 API는 out 인수를 제한하는 일반 컨벤션의 국소 예외이며 헤더 NOTE로 이유를 기록했다. optional 반환으로 바꾸거나 저장소 전체 규칙을 변경하지 않았다.

SetLocalToWorld(id,matrix)는 등록 목록을 선형 검색해 행렬만 복사한다. GPU 버퍼를 생성하거나 즉시 갱신하지 않는다. RemoveStaticMesh(id)는 해당 항목을 찾아 erase하고 소유 GPU 참조를 해제한다. 배열이 이동해도 다른 항목 ID와 등록 순서는 유지한다. 두 함수는 없는 ID/0이면 false이며 기존 항목은 유지한다. 컨텍스트의 이전 바인딩이 버퍼 참조를 보유할 수 있으므로 제거 반환이 GPU 메모리의 즉시 최종 해제를 의미하지 않는다.

RenderFrame(renderer,width,height)는 BeginFrame 성공 뒤 등록 순서대로 SetLocalToWorld(entry.matrix)와 DrawMesh(entry.renderData)를 호출하고 EndFrame 결과를 반환한다. 빈 Scene은 clear 후 Present하며 렌더러의 내부 기본 메시를 대신 그리지 않는다. WorldToClip·텍스처·기타 공통 설정은 호출자가 준비한 렌더러 값을 사용한다. 마지막 LocalToWorld/바인딩은 렌더러에 남기며 이전 값을 복원하지 않는다.

2026-10-18 프레임 조율 분리 정정: 위 FScene::RenderFrame을 Draw(FRenderer&)로 변경하고 크기 인수와 BeginFrame/EndFrame 호출을 제거했다. Scene은 활성 프레임 안에서 같은 메시 순서·행렬 설정·DrawMesh만 수행하며 빈 Scene은 true로 아무것도 하지 않는다. EngineMain이 기존 갱신 처리 뒤 BeginFrame(width,height) → Scene.Draw(renderer) → EndFrame()을 호출한다. 같은 실패 조건식의 단락 평가로 실패 이후 단계는 실행하지 않고 기존 반복 중단·종료 경로를 사용한다. Scene·Renderer는 진행 프레임을 자동 복구하지 않는다. 전체 실행 경로의 빈 월드도 실행부의 BeginFrame이 화면을 지우고 EndFrame이 표시를 요청한다. 세 코드 파일만 바꿨으며 Debug/Release x64 전체 빌드와 별도 오프스크린 장치/픽셀 검사에서 기존 월드 색·빈 Draw의 무변경·호출자가 EndFrame할 때만 Present 요청·실패 전달을 확인했다. 제품 창·실제 DXGI Present·UI 출력은 관찰하지 않았다. UI 요청 전달과 GPU 그리기는 후속 별도 범위다.

BeginFrame 거부 시 기존 활성 프레임을 닫지 않는다. 행렬/그리기 도중 실패하면 추가 그리기와 EndFrame/Present 없이 false다. 이때 프레임은 활성 상태로 남을 수 있으므로 기존 렌더러 실패 계약대로 반복을 중단하고 호출자가 Renderer::Shutdown해야 한다. EndFrame의 Present 실패도 false로 전파하지만 이 경우 Present는 이미 시도했고 활성 상태는 닫혔다. Scene이 빌린 렌더러의 종료/오류 복구를 자동으로 맡지 않는다.

등록 자원은 생성한 장치에서만 그릴 수 있다. 다른 렌더러/재초기화의 장치로 RenderFrame을 호출하면 기존 DrawMesh 장치 검사에서 거부한다. Scene은 CPU 원본을 보관하지 않아 자동 재업로드하지 않는다. 등록 항목을 제거/재등록하거나 Scene을 새로 구성해야 한다. Scene 소멸은 소유 항목/COM 참조만 정리하고 Actor·World·Renderer를 파괴하지 않는다. GPU 버퍼가 남는 동안 생성 장치의 수명이 유지될 수 있는 기존 비용은 같다.

## Scene의 등록 GPU 메시 교체 — 2026-10-16

Scene.h/.cpp에 [[nodiscard]] bool SetMeshData(uint64 meshId,FRenderer& renderer,const FStaticMeshData& meshData)를 추가했다. 기존 등록 ID를 선형 검색하고 일치하면 renderer.CreateMesh(meshData,entry.renderData)의 결과를 직접 반환한다. 없는 ID/0/제거한 ID는 false이며 GPU 생성도 요청하지 않는다. Scene 로컬 ID 규칙은 그대로이며 다른 Scene의 같은 숫자 ID를 구분하는 기능은 없다.

교체 대상의 ID·행렬·등록 순서와 nextMeshId는 바꾸지 않는다. CreateMesh의 지역 자원 준비/성공 후 이동 대입을 사용해 입력 검사·장치 준비·버퍼 생성 실패 시 기존 GPU 메시를 보존한다. 성공하면 해당 항목은 다음 Scene RenderFrame에서 새 버퍼로 그린다. CPU 원본은 저장하지 않으며 호출 후 수정/clear해도 업로드된 자원은 독립적이다. 다른 항목의 자원과 렌더러 내부 기본 메시 선택은 바꾸지 않는다.

호출자가 데이터 변경 시 같은 렌더링 스레드에서 명시적으로 호출한다. 매 프레임 교체하거나 변경을 자동 감지하지 않는다. 재초기화 후 새 장치의 renderer로 호출하면 같은 ID/행렬로 해당 항목만 재업로드할 수 있다. 다른 항목이 이전 장치 자원이면 그 항목도 별도로 갱신해야 한다. 이전 단계의 제거/재등록 또는 Scene 재구성 외에 이 갱신 경로를 추가한 것이다. Component::SetMeshData와 Scene::SetMeshData는 아직 자동 연결되지 않았다.

## World별 Scene 소유와 접근 — 2026-10-16

World.h에 Renderer/Scene.h 직접 포함과 FScene scene 값 멤버를 추가했다. Scene은 actors 소유 배열보다 먼저 선언한다. World.cpp에 [[nodiscard]] FScene& GetScene()과 [[nodiscard]] const FScene& GetScene() const를 정의해 같은 멤버를 각각 수정 가능/읽기 전용 참조로 빌려준다. 반환 참조는 소유권을 이전하거나 World 수명을 연장하지 않으며 World 소멸 후 무효다. const World에서 Scene을 수정하는 접근은 제공하지 않는다.

UWorld 기본 생성자는 그대로 default다. 각 World의 Scene은 빈 등록 목록/ID1 시작 상태로 독립 생성하며 GPU 자원이나 Renderer를 자동 생성/초기화하지 않는다. 기존 World 소멸자 본문은 DestroyActor 경로로 모든 Actor를 정리한다. 그 뒤 멤버가 선언의 역순으로 파괴되므로 Scene은 Actor·컴포넌트 정리 후에 소멸한다. Actor 소유·부모 관계·Tick/BeginPlay·제거 보류/정리 로직은 변경하지 않았다.

World는 Actor/Component와 별도로 자기 Scene의 GPU 메시/행렬을 소유한다. Scene은 Actor/Component/World/Renderer 주소를 보관하지 않는 기존 계약을 유지한다. Renderer는 여전히 외부에서 준비해 Scene API에 전달한다. Actor 생성/컴포넌트 변경/제거가 Scene에 자동 반영되지는 않는다. 현재 main의 지역 World에도 빈 Scene이 생기지만 main 코드와 기존 RenderFrame 호출을 바꾸지 않아 제품 화면은 기존 기본 메시다.

## StaticMeshComponent의 렌더 상태 생성·해제·조회 — 2026-10-16

StaticMeshComponent.h/.cpp에 bool CreateRenderState(FRenderer&), void DestroyRenderState(), bool IsRenderStateCreated() const와 소멸자 override를 추가했다. bool 조회/생성은 [[nodiscard]]다. private uint64 sceneMeshId=0만 보관하고 CoreTypes.h를 직접 포함한다. GPU 버퍼·Renderer/Scene 포인터·변경 플래그는 컴포넌트에 추가하지 않았다.

CreateRenderState는 이미 ID가 있으면 로그/false로 중복 생성을 거부한다. 그렇지 않으면 소속 Actor→World→GetScene().AddStaticMesh(renderer,meshData,GetLocalToWorld(),sceneMeshId)를 호출한다. 현재 CPU 메시와 부모 합성 월드 행렬을 등록하며 결과 bool을 그대로 반환한다. Scene의 성공 후 출력 확정 계약을 사용하므로 빈/잘못된 데이터·준비 전 Renderer·자원 생성 실패에서는 등록 ID가0으로 유지된다. 생성자/Actor AddComponent/World Spawn과는 별도의 명시적 호출이며 Scene 등록이 Actor의 컴포넌트 소유 목록 등록을 대신하지 않는다.

DestroyRenderState는 ID가0이면 아무것도 하지 않고, ID가 있으면 World의 Scene에서 해당 등록을 제거한 뒤 ID를0으로 만든다. Renderer나 GPU 명령을 사용하지 않아 Renderer Shutdown 뒤에도 해제할 수 있다. Scene Remove 결과는 해제 상태를 초기화하기 위해 버린다. 소멸자도 이 함수를 호출해 남은 등록을 정리한다. World의 Scene이 Actor/Component보다 오래 산다는 기존 순서와 owner 비소유 수명 전제를 사용한다. 반복 해제/명시 해제 후 소멸은 안전하며 CPU 메시/Actor 소유 관계는 바꾸지 않는다.

IsRenderStateCreated는 ID가0이 아닌지만 읽는다. Scene 항목의 외부 제거·Renderer 준비/장치 교체 여부를 검사하는 함수가 아니다. 컴포넌트가 관리하는 항목은 외부에서 임의 제거/교체하지 않는 계약으로 사용한다. Renderer 종료 뒤에도 명시 해제 전까지 조회는 true일 수 있다. 장치 재초기화 시에는 DestroyRenderState 후 새 Renderer로 CreateRenderState를 호출해 재구성한다.

SetMeshData/GetMeshData/GetLocalToWorld 구현은 그대로다. SetMeshData는 CPU 복사만 하고 기존 렌더 상태를 갱신/삭제하거나 dirty 표시하지 않는다. 생성 뒤 CPU 메시·부모/Actor 변환을 바꿔도 Scene의 생성 시 복사본은 유지된다. 빈 CPU setter 역시 자동 해제가 아니다. 이번 단계에서는 명시 해제 후 재생성으로 현재 메시/행렬을 전달할 수 있다. 변경 표시·변환 전달·엔진의 자동 생성/호출은 후속이다. 실패한 Create가 setter로 바뀐 CPU 데이터를 되돌리는 기능은 아니다.

## StaticMeshComponent의 명시적 렌더 변환 전달 — 2026-10-16

StaticMeshComponent.h/.cpp에 [[nodiscard]] bool SendRenderTransform()을 추가했다. IsRenderStateCreated가 false이면 즉시 false로 반환하고 등록을 만들지 않는다. 생성된 상태면 GetOwner().GetWorld().GetScene().SetLocalToWorld(sceneMeshId,GetLocalToWorld())를 호출해 결과를 그대로 반환한다. 호출 때마다 소속 Actor의 기본 Transform과 부모 합성에서 현재 월드 행렬을 얻는다.

이 함수는 Scene의 행렬 복사본만 바꾸며 CPU 메시·GPU 정점/인덱스 버퍼·등록 ID·생성/해제 상태를 바꾸지 않는다. Renderer 인수나 보관 포인터가 없고 직접 GPU 명령을 호출하지 않는다. 실제 상수 버퍼 갱신과 그림은 다음 Scene RenderFrame의 기존 SetLocalToWorld/DrawMesh에서 수행한다. 반환 true는 Scene 행렬 반영 성공이며 Renderer 초기화/현재 장치/GPU 완료를 보증하지 않는다. Renderer Shutdown 뒤에도 World/Scene/등록이 살아 있으면 CPU 행렬 갱신은 가능하다.

변환 데이터의 유한 값/정규화 회전·TRS 합성과 전단 표현 한계는 기존 GetLocalToWorld/FTransform을 따른다. Scene 등록이 외부에서 제거됐다면 갱신 결과는 false지만 컴포넌트가 보관한 ID를 자동으로0으로 만들지는 않는다. 관리 중인 등록을 외부에서 임의 제거하지 않는 기존 계약은 그대로다. 명시 DestroyRenderState로 상태를 정리할 수 있다. 생성 전/해제 뒤 호출은 false이며 자동 재등록하지 않는다.

이번에는 호출 시점과 변경 표시를 추가하지 않았다. Actor/부모 변환이 바뀌면 호출자가 SendRenderTransform을 명시 호출해야 한다. SetMeshData는 여전히 CPU 복사만 하고 이 함수로 메시 내용이 업로드되지 않는다. 엔진 자동 호출·dirty 플래그·메시 변경 전달·Renderer/Scene/World/main 코드는 변경하지 않았다.

## StaticMeshComponent 렌더 상태 변경 표시 — 2026-10-16

`MarkRenderStateDirty()`와 `[[nodiscard]] bool IsRenderStateDirty() const`, 초기 false인 `bRenderStateDirty`를 추가했다. 생성된 상태에서만 표시하며 미생성 상태에서는 무시한다. `SetMeshData()`는 CPU 복사 후 표시 함수를 호출한다. 동일 데이터인지 비교하지 않으며 빈/잘못된 입력도 기존 setter처럼 보관한다. GPU 업로드나 Scene 변경은 하지 않는다.

`CreateRenderState()`가 Scene 등록에 성공한 뒤에만 표시를 false로 초기화한다. 중복 생성 거부와 등록 실패에서는 표시를 지우지 않는다. `DestroyRenderState()`는 미생성/반복 호출까지 표시를 초기화하며 CPU 메시를 보존한다. `SendRenderTransform()`은 행렬만 보내므로 메시 변경 표시를 소비하지 않는다. Renderer 종료도 컴포넌트의 ID/표시를 자동으로 지우지 않는다.

이전 CPU setter만 제공하던 단계에서 변경 기록을 추가한 것이다. 표시가 true여도 기존 GPU 메시가 그대로 사용된다. 실제 변경 처리/지연 큐/자동 호출/공통 가상 수명 함수는 아직 없다. 현재는 명시 해제 후 재생성으로 최신 CPU 메시와 행렬을 반영할 수 있다. 이전 절의 dirty 미구현 설명은 당시 단계의 이력으로 보존한다.

## StaticMeshComponent의 명시적 메시 갱신 — 2026-10-16

`[[nodiscard]] bool UpdateRenderState(FRenderer&)`를 추가했다. 미생성이면 false, 생성돼 있지만 dirty가 아니면 Renderer 준비 검사/업로드 없이 true다. dirty이면 소속 World의 Scene.SetMeshData(sceneMeshId, renderer, meshData)를 호출하며 성공한 경우에만 bRenderStateDirty를 false로 만든다. 실패하면 false를 반환하고 기존 GPU 메시·등록 ID·행렬·dirty를 보존한다. 빈 CPU 데이터는 자동 해제가 아니라 갱신 실패다.

성공은 기존 Scene 등록의 GPU 메시 교체 또는 변경 없는 상태를 뜻한다. CPU 원본은 유지하고 Actor의 현재 변환은 이 함수로 다시 보내지 않는다. 변환은 SendRenderTransform이 별도로 담당한다. setter/MarkRenderStateDirty 이후 이 함수를 명시 호출해야 반영되며 World/main의 자동 호출·프레임 종료 큐는 아직 없다. 이전 절의 메시 변경 처리 미구현 설명은 당시 단계 이력이다.

## World의 프레임 종료 렌더 변경 전달 — 2026-10-16

World.h/.cpp에 `[[nodiscard]] bool SendAllEndOfFrameUpdates(FRenderer&)`를 추가했다. World.h는 FRenderer를 전방 선언하며 World.cpp는 StaticMeshComponent.h를 직접 포함한다. World의 소유 Actor 등록 순서, 각 Actor 컴포넌트 추가 순서로 UStaticMeshComponent와 파생 타입을 모두 조회한다. 호출 안에서 재사용하는 포인터 배열은 소유권이나 수명을 연장하지 않는다.

IsRenderStateCreated가 false이면 건너뛰고 자동 생성하지 않는다. 생성된 대상에는 UpdateRenderState(renderer) 다음 SendRenderTransform()을 호출한다. 어느 쪽이든 실패하면 즉시 false로 중단한다. 해당 메시 갱신이 실패하면 같은 대상의 변환 전달도 하지 않는다. 앞서 성공한 대상/메시 변경은 되돌리지 않으며 뒤 대상은 다음 명시 호출에서 재시도한다. Scene 변환 실패 전에 메시 갱신이 성공한 경우 그 메시/dirty 해제도 유지한다. 전부 처리했거나 대상이 없으면 true다.

Tick 종료 후 그리기 전에 Tick/생성·제거 콜백 밖에서 단일 스레드로 명시 호출하는 계약이다. 기존 Tick과 생성/제거 경로 및 main은 변경하지 않았으며 실제 제품 실행 루프에는 아직 연결하지 않았다. 변환 dirty는 없어 매 호출에 부모 합성 행렬을 전달하고 clean 메시 버퍼는 재생성하지 않는다. 빈 월드/미생성 대상만 있는 월드는 준비 전 Renderer로도 true일 수 있으므로 반환값은 Renderer 준비 보증이 아니다.

## 실행 루프의 World 변경 전달과 Scene 그리기 연결 — 2026-10-16

EngineMain.cpp의 world.Tick(deltaSeconds) 다음 조건을 `!world.SendAllEndOfFrameUpdates(renderer) || !world.GetScene().RenderFrame(renderer, clientWidth, clientHeight)`로 바꿨다. Tick → 렌더 변경 전달 → World Scene 그리기 순서이며 실패하면 기존 exitCode=1/break 경로로 Renderer와 창을 정리한다. 변경 전달 실패 시 단락 평가로 Scene 그리기를 실행하지 않는다. 이미 반영된 일부 갱신은 World의 기존 계약대로 유지한다.

이전 Renderer::RenderFrame이 Renderer 소유 기본 메시를 직접 그리던 경로 → World가 소유한 Scene 등록만 그리는 경로로 전환했다. 현재 main의 World에는 Actor/메시가 없으므로 화면은 기본 사각형 대신 배경색만 지우고 Present한다. 기본 GPU 메시/셰이더/텍스처를 준비하는 기존 Renderer 초기화는 그대로이며 이번 변경이 초기화 자원을 제거하지는 않는다. SampleGame 생성/렌더 상태 생성은 후속 별도 범위다.

최소화 시 클라이언트 크기가0이면 기존 continue로 Tick/변경 전달/그리기를 모두 생략하며 시계 기준은 계속 갱신한다. 양수 크기는 Scene→Renderer BeginFrame 경로로 크기 변경을 전달한다. 초기화·크기 조회 실패, Renderer 종료 뒤 창 종료, 메시 종료코드 보존, 기존 chrono auto 사용과 나머지 main 코드는 변경하지 않았다.

## 단일 실행 프로젝트의 게임 구성 함수 선택과 재검토 기준 — 2026-10-16

사용자 결정: 당장은 별도 Engine/Game 라이브러리·Launch 프로젝트로 나누지 않고, 현재 SampleGame 실행 프로젝트 하나에서 엔진과 게임을 함께 빌드한다. 엔진 소유 main이 게임 쪽에서 구현하는 구성 함수 하나를 호출하는 방법을 선택했다. 이 결정은 모듈·리플렉션을 배제한다는 의미가 아니며 실제 요구가 생기면 도입한다. 이전 3프로젝트/정적 라이브러리·클래스 등록 제안은 현재 채택안이 아니다.

엔진이 공통 선언을 제공하고 게임이 구현한다. 엔진은 구체 게임 헤더/클래스에 의존하지 않으며 게임 함수에는 World만 빌려줘 Actor·컴포넌트·CPU 콘텐츠를 구성하게 한다. main/창/Renderer 준비·프레임 반복·종료는 엔진 책임이다. 같은 실행 파일을 링크할 때 게임 구현이 연결되는 방식이며 런타임 게임 탐색/모듈 로딩이 아니다. 실행 파일마다 게임 구성 구현이 하나 필요하고, 빠지거나 중복되면 링크 오류로 드러난다. 게임 클래스마다 main을 수정하는 방식은 선택하지 않는다.

코드 형태 제안(아직 구현/세부 코드 범위 승인 전): Engine/Source/GameEntry.h에 [[nodiscard]] bool ConfigureGameWorld(UWorld& world)를 선언하고 SampleGame/Source/SampleGame.cpp가 구현한다. EngineMain은 Renderer 준비→World 생성→구성 함수→렌더 상태 준비→Tick/변경 전달/Scene 그리기 순서로 확장한다. 렌더 상태 준비는 아직 없으며 현재 SendAllEndOfFrameUpdates는 미생성 상태를 건너뛰므로 게임 Actor를 생성하는 것만으로는 화면에 나오지 않는다. 함수 호출 연결과 초기 렌더 상태 준비·샘플 Actor는 각각 작은 승인 단위로 진행한다.

실패 계약 제안: 구성 함수 false이면 게임 루프를 시작하지 않고 종료코드1로 정리한다. 부분 생성된 World를 롤백했다고 주장하지 않으며 엔진이 World를 파괴해 정리한다. World/Actor/Component/Scene은 Renderer Shutdown 전에 정리되는 범위를 마련한다. 초기 구성과 프레임 Tick은 구분하고 반복 동작은 Actor::Tick을 사용한다. 메시 생성·업로드 시 Renderer를 게임에 넘기는 방식, 게임별 main, 미사용 GameInstance는 이번 선택에 포함하지 않는다.

## 게임 구성 함수 연결 — 2026-10-16

Engine/Source/GameEntry.h에 [[nodiscard]] bool ConfigureGameWorld(UWorld&)를 선언하고 SampleGame/Source/SampleGame.cpp가 구현한다. 현재 구현은 [[maybe_unused]] World 참조를 받아 true만 반환하므로 제품 World/화면은 계속 빈 Scene/배경이다. 게임별 실행 파일은 이 함수 구현을 정확히 하나 제공한다. 엔진은 GameEntry.h만 include하고 SampleGame 헤더나 구체 클래스는 include하지 않는다. 같은 실행 프로젝트에서 C++ 컴파일/링커로 연결되며 동적 모듈 발견/클래스 등록은 없다.

EngineMain은 창/Renderer 초기화 성공 뒤 지역 World를 만들고 구성 함수를 한 번 호출한다. false이면 exitCode1로 메시지 반복/Tick/그리기를 시작하지 않는다. 부분 생성된 Actor를 트랜잭션 롤백하지 않고 World 파괴로 정리한다. true이면 기존 최소화 검사·Tick→변경 전달→Scene 그리기를 실행한다. World를 내부 블록에 둬 구성 실패/정상 종료/프레임 실패 모두 Actor/Component/Scene을 Renderer Shutdown보다 먼저 파괴한다. 창은 Renderer 뒤에 정리한다. 기존 시계/최소화/프레임 실패·메시 종료코드 처리는 그대로다.

헤더 주석은 게임당 구현1개·빌린 World·초기 콘텐츠 구성·엔진 소유 main/Renderer/반복·false의 부분 정리·미래 모듈/클래스 등록 때 연결부 재검토를 명시한다. 게임 구현 주석은 초기 Actor/컴포넌트 구성 위치와 현재 빈 상태를 설명한다. SampleGame 프로젝트에 게임 cpp/엔진 계약 헤더를, 필터에 해당 항목만 추가했다. 생성기/라이브러리/솔루션/TriWorld는 바꾸지 않았다.

## 초기 렌더 상태 일괄 준비의 책임과 교체 조건 — 2026-10-16

정정/명확화: ConfigureGameWorld는 엔진이 게임의 초기 콘텐츠 구성을 호출하는 연결 계약이다. CreateRenderStates 제안은 구성된 컴포넌트의 렌더 상태를 언제 준비할지에 관한 엔진 내부 수명 선택이다. 자유 함수로 게임을 연결해서 반드시 필요한 API가 아니며, 라이브러리/모듈로 게임을 분리해도 렌더 준비 시점 자체는 정해야 한다. 게임 연결 방식 변경을 이유로 이 함수를 자동 추가/삭제하지 않는다.

필수 작업은 CPU 메시로 GPU 자원을 준비하고 Scene에 등록하는 것이다. 이미 UStaticMeshComponent::CreateRenderState → FScene::AddStaticMesh → FRenderer::CreateMesh 경로가 있다. 아직 빠진 것은 엔진의 호출 시점이다. AActor::AddComponent는 소유 목록에 넣는 역할이고 World/렌더 시스템의 컴포넌트 등록 수명을 제공하지 않는다. 현재 SendAllEndOfFrameUpdates는 미생성 상태를 건너뛴다.

다음 코드 범위 제안/미구현: World.h/.cpp에 [[nodiscard]] bool CreateRenderStates(FRenderer& renderer)를 추가하고 EngineMain.cpp에서 ConfigureGameWorld 성공 직후, 첫 Tick/BeginPlay 이전에 한 번 호출한다. 게임 함수에는 Renderer를 전달하지 않는다. 게임은 이 호출 전까지 초기 Actor/컴포넌트/CPU 메시 구성을 마쳐야 한다. BeginPlay에서 처음 메시를 채우거나 새 컴포넌트를 추가하는 패턴과 실행 중 스폰의 자동 준비는 이 초기 순회의 범위 밖이다.

World는 Actor 등록 순서와 컴포넌트 추가 순서로 모든 UStaticMeshComponent 및 파생 타입을 조회한다. 이미 렌더 상태가 생성된 대상은 건너뛰고 dirty 메시 갱신/변환 전송도 이 함수에서는 하지 않는다. 미생성 대상에는 기존 CreateRenderState를 호출한다. 빈 World/일치 컴포넌트 없음은 true, 빈/잘못된 메시·Renderer 준비 전 등의 생성 실패는 false로 즉시 중단한다. 실패 전 성공한 등록은 유지하고 뒤 대상은 처리하지 않는다. 전체 원자적 롤백이나 자동 재시도가 아니다. 명시 재호출 시 기존 상태를 건너뛰고 나머지를 시도할 수 있다.

main의 조건 제안은 !ConfigureGameWorld(world) || !world.CreateRenderStates(renderer)다. 게임 구성 실패 시 렌더 준비는 호출하지 않는다. 렌더 준비 false도 기존 exitCode1/루프 미진입 경로로 처리하며 World가 부분 Actor/Component/Scene 등록을 Renderer Shutdown 전에 정리한다. Tick/생성·제거 콜백 밖의 안정된 목록에서 단일 스레드로 호출한다. SampleGame의 빈 성공 구현은 유지하므로 이 단계만으로 화면에 새 메시를 추가하지 않는다.

## 초기 렌더 상태 일괄 준비 구현 — 2026-10-17

World.h/.cpp에 [[nodiscard]] bool CreateRenderStates(FRenderer&)를 추가했다. 소유 Actor 등록 순서, 각 Actor 컴포넌트 추가 순서로 모든 UStaticMeshComponent와 파생 타입을 조회한다. 생성된 상태는 건너뛰고 미생성 상태만 기존 CreateRenderState를 호출한다. 기존 ID/행렬/GPU 버퍼/dirty 상태를 갱신하는 함수가 아니다. 함수 안의 임시 포인터 배열은 Actor 간 용량을 재사용하며 컴포넌트를 소유하거나 수명을 연장하지 않는다.

빈 World/메시 컴포넌트 없음은 true다. 생성 실패는 false로 즉시 중단하며 앞서 성공한 등록과 뒤 대상의 미생성 상태를 유지한다. 빈/잘못된 CPU 메시와 준비 전 Renderer는 생성 실패다. 명시 재호출은 이미 생성된 대상을 건너뛰고 나머지를 시도할 수 있다. 모두 생성돼 있으면 Renderer 준비 전에도 true일 수 있어 GPU/장치 준비 보증이 아니다. 전체 원자적 롤백과 자동 재시도는 없다. Tick/생성·제거 콜백 밖 안정된 등록 목록에서 단일 스레드로 호출한다.

EngineMain은 !ConfigureGameWorld(world) || !world.CreateRenderStates(renderer)를 검사한다. 게임 구성 성공 뒤 첫 Tick/BeginPlay 전에 한 번 준비하고, 어느 단계든 실패하면 기존 종료코드1/루프 미진입 경로로 정리한다. 게임 구성 false이면 준비 함수를 호출하지 않는다. 부분 생성된 World/Actor/Component/Scene은 기존 내부 블록에서 Renderer Shutdown 전에 정리한다. 빈 SampleGame 구현은 유지해 실제 제품은 여전히 배경만 그린다.

중요한 한계: 이 초기 호출 뒤 생성한 Actor/메시 컴포넌트는 자동 렌더 등록되지 않는다. BeginPlay/Tick에서 처음 만든 컴포넌트도 마찬가지다. SendAllEndOfFrameUpdates는 기존 계약대로 미생성 상태를 건너뛴다. 별도로 CreateRenderState 또는 World 준비를 명시 호출하면 준비할 수 있지만 엔진의 실행 중 자동 등록 경로는 아직 없다. 게임의 초기 CPU 데이터는 준비 호출 전에 구성해야 한다. BeginPlay에서 처음 데이터를 채우는 기존 빈 메시 컴포넌트는 초기 준비 시 실패할 수 있다.

## SampleGame의 초기 사각형 구성 — 2026-10-17

SampleGame/Source/SampleGame.cpp의 ConfigureGameWorld(UWorld&)가 기본 AActor 하나를 SpawnActor로 생성하고 UStaticMeshComponent 하나를 붙인다. 정점4개는 XY -0.5~0.5, Z0이며 RGBA 정점색/UV와 Triangle List 인덱스6개(0,1,2,0,2,3)를 갖는다. Actor의 기본 Transform은 항등값으로 유지하므로 현재 항등 뷰에서 화면 중앙에 체크 텍스처와 정점색 사각형을 그린다. 깊이/원근 카메라/동적 회전은 이번 샘플에 넣지 않았다.

TUniquePtr<UStaticMeshComponent>로 만들고 SetMeshData로 CPU 데이터를 복사한 뒤 std::move로 TUniquePtr<UActorComponent>에 소유권을 옮겨 actor.AddComponent에 전달한다. 반환 bool은 함수 결과로 그대로 전달한다. 실패 시 local 소유 컴포넌트는 함수 종료에서 정리되고 이미 생성된 Actor는 World 소유로 남아 엔진의 구성 실패 종료에서 정리된다. 이 실패를 강제로 주입한 새 샘플 검사는 없고 기존 AddComponent/게임 구성 실패 검사를 유지했다.

게임 cpp는 Renderer/Scene 호출이나 GPU 객체를 만들지 않는다. 기존 엔진의 ConfigureGameWorld → World.CreateRenderStates → Tick/SendAllEndOfFrameUpdates/Scene.RenderFrame 경로를 사용한다. 새 Actor 파생 클래스·프로젝트 목록·엔진 코드는 추가/수정하지 않았다. 현재 게임 구성은 엔진이 시작할 때 한 번 호출하는 계약이며 반복 호출의 콘텐츠 중복 방지 기능을 추가하지 않았다. 실행 중 새 컴포넌트 자동 등록 미지원은 그대로다.

## SampleActor의 게임 Tick 회전 — 2026-10-17

SampleGame/Source/SampleActor.h/.cpp에 ASampleActor : public AActor를 추가했다. 생성자는 UWorld&를 기반 생성자에 전달한다. Tick(float deltaSeconds)은 기본 UTransformComponent를 조회하고 현재 FRotator를 얻어 roll에45.0f*deltaSeconds를 더한 뒤 SetRotation에 전달한다. 현재 순수 로컬 Z축 회전 예제이며 별도 각도 저장 필드/Renderer/Scene 호출은 없다. 기본 Transform이 항상 있다는 기존 Actor 계약을 사용한다.

SampleGame.cpp의 SpawnActor 타입만 AActor→ASampleActor로 바꿨다. 게임 구성 함수의 CPU 메시/정점색/UV/인덱스·컴포넌트 소유권 전달은 유지한다. 헤더를 직접 포함하고 프로젝트/필터에 SampleActor.h/.cpp만 등록했다. 기존 엔진의 첫 Tick/BeginPlay→가상 Actor Tick→World 변경 전달→Scene 그리기를 그대로 사용한다. 엔진 코드는 수정하지 않았다.

회전속도는 초당45도다. GetRotation은 저장 쿼터니언을 대표 Euler 각도로 반환하므로 roll 숫자가 계속0~360으로 누적되는 상태 변수는 아니다. 검사/설명은45/90도와 기하학적180도를 구분한다. deltaSeconds의 유효성/시간 정책은 기존 엔진을 따르며 게임 샘플에 별도 클램프/고정 시간/프레임 제한을 추가하지 않았다. 시작 시 이미 만들어진 메시의 변환만 변경하며 실행 중 신규 컴포넌트 자동 등록 문제는 해결하지 않는다.

## Renderer의 기본 메시 경로 제거 — 2026-10-17

Renderer.h/.cpp에서 초기 사각형 CPU 데이터·defaultMesh 멤버·정점/인덱스 버퍼 생성과 정리, SetMesh, 인수 없는 DrawMesh, Renderer::RenderFrame을 제거했다. Initialize는 장치·셰이더·입력 레이아웃·변환 상수 버퍼·기존 체크 텍스처/SRV/샘플러·스왑 체인을 준비한다. 그릴 메시 자체는 생성/선택하지 않는다. 이전 X0.25 기본 이동도 제거하고 초기 LocalToWorld/WorldToClip은 항등행렬로 둔다. 재초기화도 같다. 초기화 로그의 Quad 표현을 실제 공통 자원 준비에 맞게 정정했다.

CreateMesh는 CPU 입력을 GPU로 복사해 호출자 소유 FStaticMeshRenderData로 성공 후 교체한다. 기존 비어 있지 않은 정점/인덱스·3의 배수·범위 안 인덱스 조건과 실패 보존을 유지하고 삭제한 SetMesh를 참조하던 헤더 설명에 직접 적었다. GPU 메시 소유는 Scene 또는 호출자이며 Renderer는 DrawMesh(data) 호출 중 빌린다. Renderer Shutdown은 자신의 공통 자원/바인딩을 정리하지만 외부 메시 소유권을 회수하지 않는다. 장치가 바뀌면 기존 외부 메시 Draw는 기존 계약대로 거부한다.

BeginFrame/DrawMesh(data)/EndFrame과 Scene.RenderFrame은 유지한다. 제품 main은 이미 World Scene 경로이며 게임의 회전 샘플/CPU 메시/Actor/Component/World 코드는 바꾸지 않았다. 같은 Renderer에 여러 메시를 만들더라도 암묵적인 현재 메시 선택은 없다. 그리기 없는 프레임은 배경을 지우고 Present한다. 초기화 성공만으로 Scene에 그릴 항목이 생기지 않는다. 이번 정리는 메시만 대상으로 하며 체크 텍스처·셰이더 경로를 범용 재질/에셋 시스템으로 바꾸지는 않았다.
