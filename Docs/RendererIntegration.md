# 다른 렌더러 연결 안내

SeonEngine은 `IRenderer`를 통해 렌더러를 사용한다. 다른 렌더러를 적용하려는 쪽에서 이 인터페이스를 구현하고, 그 객체를 만드는 `CreateRenderer()`를 제공한다. 엔진은 현재 기능에 필요한 데이터와 호출 계약을 제공하고, 외부 형식으로의 변환과 GPU 처리는 구현자가 맡는다.

현재 기본 구현은 `FD3D11Renderer`다. 아래 절차와 예시는 외부 구현자가 따를 안내이며, NoTemplate이나 다른 그래픽스 API가 이미 연결돼 있다는 뜻은 아니다. 개별 함수의 정확한 조건은 링크된 헤더의 계약 주석을 기준으로 한다.

## 1. 렌더러 교체와 RHI의 구분

현재 연결 경계는 다음과 같다.

```text
엔진과 실행부
  └─ IRenderer
       ├─ 기본 FD3D11Renderer
       └─ 적용자가 구현할 어댑터 → 외부 렌더러
```

`IRenderer`는 렌더러 전체를 연결하는 입구다. 어댑터는 이 계약을 구현하면서 엔진의 데이터와 요청을 외부 렌더러의 자원, 장면과 명령으로 바꾼다. 외부 라이브러리의 타입이나 내부 ID를 공통 계약에 노출할 필요는 없다.

RHI는 자체 렌더러 아래에서 그래픽스 API 구현을 교체하는 별도 경계다. 다음은 향후 가능한 구성 예시이며 현재 구현은 아니다.

```text
엔진 → IRenderer → 자체 렌더러 → RHI → D3D11 / D3D12 등
```

외부 렌더러가 없어도 `CreateRenderer()`가 자체 렌더러를 반환하면 된다. 향후 RHI 기반 자체 렌더러를 반환할 수도 있지만, 현재 D3D11 처리와 렌더링 로직을 분리하는 작업까지 생성 함수가 대신하지는 않는다.

## 2. 처음 읽을 파일

| 파일 | 확인할 내용 |
|---|---|
| [RendererFactory.h](../Engine/Source/Renderer/RendererFactory.h) | 생성 함수 선언, 새 미초기화 객체 반환과 단독 소유권 |
| [샘플 RendererFactory.cpp](../Game/Source/RendererFactory.cpp) | 기본 D3D11 선택을 제공하는 정의 |
| [Renderer.h](../Engine/Source/Renderer/Renderer.h) | `IRenderer`의 전체 함수와 성공, 실패, 프레임, 수명 계약 |
| [EngineLoop.cpp](../Engine/Source/Launch/EngineLoop.cpp) | 창 준비, 생성 함수 호출, 초기화, 실행과 종료 순서 |
| [SceneRenderData.h](../Engine/Source/Renderer/SceneRenderData.h) | 시점, 광원, 메시별 변환과 재질 요청 |
| [ImGuiRendererBackend.h](../Engine/Source/Renderer/ImGuiRendererBackend.h) | 에디터 GPU backend와 타겟 이미지 참조 계약 |
| [D3D11Renderer.h](../Engine/Source/D3D11/D3D11Renderer.h)와 [cpp](../Engine/Source/D3D11/D3D11Renderer.cpp) | 공통 계약을 구현한 기존 사례 |

## 3. 연결할 범위를 먼저 정한다

게임 실행인지 에디터 실행인지, 어떤 콘텐츠를 그릴지 먼저 정한다. 그 범위에서 사용하는 메시, 텍스처, 폰트, UI와 렌더 타겟 기능을 나열하고 외부 렌더러의 대응 기능을 확인한다.

미지원 기능도 `IRenderer`의 순수 가상 함수 선언에 맞는 본문은 구현해야 한다. 사용할 수 없는 요청은 계약에 따라 `false`, 미지원 ImGui backend 생성은 `nullptr`로 전달한다. 호출만 성공시킨 뒤 실제 요청을 버리거나 기본 D3D11 구현으로 자동 대체하지 않는다. 단, 빈 UI 목록은 상태 변경 없이 성공한다는 기존 계약을 따른다.

현재 게임 경로는 프레임 안에서 장면 캡처, 주 장면, 게임 UI를 순서대로 제출한다. 기본 샘플은 초기화 중 CPU 이미지로 텍스처도 생성하므로, 메시만 지원하는 구현으로 기존 샘플이 바로 실행된다고 가정하면 안 된다.

현재 에디터는 ImGui GPU backend와 뷰포트 렌더 타겟의 생성, 장면 출력, 이미지 읽기를 요구한다. 에디터 지원은 `CreateImGuiRendererBackend()`가 객체를 반환하는 것만으로 끝나지 않는다.

## 4. 어댑터와 구현별 자원을 만든다

### 엔진이 전달하는 입력

| 입력 | 현재 제공하는 값 | 변환 시 확인할 점 |
|---|---|---|
| [FStaticMeshData](../Engine/Source/Renderer/StaticMeshData.h), [FVertex](../Engine/Source/Renderer/Vertex.h) | CPU 정점과 인덱스. 정점에는 위치, 선형 RGBA 색상, 법선, UV가 있음 | 외부 정점 배치로 변환한다. 탄젠트 등 없는 속성이 필요하면 생성하거나 해당 기능의 요구를 별도로 조율한다. 색상 같은 기존 속성을 무시한 채 동등 지원으로 설명하지 않는다 |
| [FImage](../Engine/Source/Image/Image.h) | 위쪽 행부터 저장한 패딩 없는 RGBA8, 비선행곱 알파, 크기와 픽셀 배열 | 색상 공간 메타데이터와 파일 경로는 없다. CPU 이미지 업로드 요청을 파일 로딩 요청으로 취급하지 않는다 |
| `FRenderView` | 월드에서 뷰, 뷰에서 클립 행렬, 월드 시점 위치와 직교 여부 | 행벡터 기준의 행렬 순서와 외부 셰이더의 저장, 전치, 좌표 규칙을 대조한다 |
| `FMeshDrawItem` | 빌린 형상, 로컬에서 월드 행렬, 색상 텍스처, 정반사 색상과 광택 지수 | 외부 재질 모델과 값의 의미를 대조한다. 서로 다른 재질 파라미터를 이름만 바꿔 같은 값으로 취급하지 않는다 |
| `FSceneRenderData` | 한 시점의 메시 목록, 빛이 진행하는 방향, 세기가 반영된 방향광과 주변광 색상 | 필요한 패스, 정렬과 GPU 명령으로 바꾼다. 제출받은 Scene이나 Proxy 자체를 외부 월드로 소유 이전하는 계약은 아니다 |
| [FUIDrawElementList](../Engine/Source/UI/UIDrawElementList.h) | 현재는 [이미지 요소](../Engine/Source/UI/UIImageDrawElement.h)의 목록 | 현재 출력 영역 왼쪽 위 기준 픽셀 위치와 크기, 비선행곱 알파, 추가 순서를 따른다 |
| DDS 바이트 | 원본 파일의 헤더와 픽셀/밉 데이터. 단일 2D RGBA/BGRA/BGRX8 및 BC1/2/3 색상 DDS | `CreateTexture2DFromDDS`에서 형식과 자원 종류를 검사한다. 원본 밉과 픽셀을 유지하며 입력은 호출 동안만 빌린다. 배열·큐브·볼륨·Custom 알파 표현은 거부한다 |
| 폰트 바이트 | 구현이 지원하는 컴파일된 폰트 형식의 바이트 | `CreateFontResource`는 모든 렌더러가 같은 폰트 파일을 해석한다는 보장이 아니다. 실제 지원 형식을 정한다 |

메시 생성은 `CreateMesh`로, CPU 이미지 텍스처 생성은 `CreateTexture2D`로, 원본 DDS는 `CreateTexture2DFromDDS`로 받는다. 메시 데이터의 재질 번호와 주소 설정은 형상 생성에서 사용하지 않는다. 텍스처의 X/Y 주소 설정은 자산에서 조회하는 별도 설정이다.

### GPU 표현의 보관

구현에 필요한 자원 보관 타입을 다음 공통 기반에서 파생시킨다.

| 공통 기반 | 엔진 쪽 소유자 | 구현자가 보관할 내용의 예 |
|---|---|---|
| [FStaticMeshRenderData](../Engine/Source/Renderer/StaticMeshRenderData.h) | `UStaticMesh` | 외부 메시 자원, 등록 객체나 내부 식별 정보 |
| [FTextureRenderData](../Engine/Source/Renderer/TextureRenderData.h) | `UTexture`와 그 파생 자산 | 실제 크기와 일치하는 외부 텍스처 자원, 읽기와 선택적 출력 표현 |
| [FFontRenderData](../Engine/Source/Renderer/FontRenderData.h) | `UFont` | 구현이 지원하는 폰트 자원 |

이 기반 타입들은 외부 GPU 자원이나 외부 ID의 관리 방법을 대신 구현하지 않는다. 생성한 구현과 장치의 자원만 사용하도록 검사하고, 외부 ID가 삭제 과정에서 바뀌는 구조라면 어댑터 내부에서 그 변화를 처리한다.

생성 함수들은 성공한 뒤에만 기존 출력 데이터를 교체하고 실패하면 이전 출력을 유지해야 한다. 자산의 `SetRenderData`는 소유권을 넘기는 입구이며 GPU 업로드나 구현 호환성 검사, GPU 완료 대기를 대신하지 않는다. 실제 자원 보존과 지연 해제는 구현자가 설계한다.

### 함수별 구현 책임

| 함수 | 구현할 책임 |
|---|---|
| `Initialize`, `Shutdown` | 빌린 창과 양수 실제 크기로 준비. 중복 초기화 거부. 빈 상태, 부분 실패, 반복 종료 정리 |
| `CreateMesh` | CPU 삼각형 목록을 받아 구현별 형상 생성. 반환 후 입력 배열을 사용하지 않음 |
| `CreateTexture2D` | RGBA8 CPU 이미지로 전체 밉 텍스처 준비. 성공할 때만 출력 데이터와 크기 교체 |
| `CreateTexture2DFromDDS` | 원본 DDS 바이트로 단일 2D GPU 텍스처 준비. 새 밉 생성·픽셀 변환 없이 원본 밉 유지. 크기와 Straight/Premultiplied 표현 저장. 미지정 표현은 Straight. 실패 시 기존 출력 유지 |
| `CreateFontResource` | 지원하는 바이트 형식으로 생성. 입력은 호출 동안만 빌림. 헤더의 실패와 예외 계약 준수 |
| `CreateRenderTarget2D` | 활성 프레임 밖에서 지정 크기의 RGBA8 UNORM 색상 타겟 준비 |
| `BeginFrame`, `EndFrame` | 창 출력 준비와 지움, 프레임 상태 관리, 마지막 표시 요청 |
| `BeginRenderTarget`, `EndRenderTarget` | 활성 프레임 안에서 타겟 전환과 지움, 창 출력과 기존 행렬 복원. 중첩 거부 |
| `DrawScene`, `DrawUI` | 현재 출력에 장면과 UI 요청 제출. 입력 복사, 순서와 자원 수명 확보 |
| `CreateImGuiRendererBackend` | 같은 구현과 장치에 맞는 미초기화 GPU backend 반환. 미지원이면 `nullptr` |

각 함수의 실패 시 상태 보존 조건은 [IRenderer 주석](../Engine/Source/Renderer/Renderer.h)을 함께 따른다. `true`는 GPU 완료나 화면 표시 성공 전체를 보장하지 않는다.

## 5. 프레임 순서와 자원 수명을 지킨다

Loop가 창을 준비한 다음 생성 함수를 호출하고, 받은 객체에 `Initialize`를 호출한다. Renderer 초기화가 성공한 뒤 Engine, World와 콘텐츠, 에디터 UI를 준비한다.

게임의 기본 렌더 순서는 다음과 같다.

```text
BeginFrame
  → 장면 캡처가 있으면 타겟별 BeginRenderTarget / DrawScene / EndRenderTarget
  → 창의 DrawScene
  → 게임 DrawUI
EndFrame
```

에디터는 UI 프레임 구성 중 패널 크기를 구해 Renderer 프레임 시작 전에 타겟을 준비한다. 이후 캡처와 편집 장면을 타겟에 그리고 창 출력을 복원한 다음, 타겟 이미지를 포함한 ImGui를 창에 제출하고 `EndFrame`으로 표시한다. 접힌 패널은 해당 편집 타겟의 장면과 이미지를 생략한다.

구현이 GPU 제출을 지연하더라도 **타겟 쓰기, UI에서 해당 타겟 읽기, 표시 요청의 순서**를 유지해야 한다. 장면만 마지막으로 미루면서 ImGui를 먼저 즉시 제출하면 이 순서가 깨질 수 있다.

- 초기화, 사용과 정리는 같은 스레드에서 수행한다. 창은 Renderer 정리가 끝날 때까지 살아 있다.
- 자원 생성 입력과 장면, UI 요청의 배열은 호출 동안만 빌린다. 지연 구현은 반환 전에 필요한 CPU 값과 GPU 자원 수명을 확보한다.
- `FMeshDrawItem.mesh`와 UI 이미지의 텍스처 주소는 비소유다. 텍스처 자산의 공유 참조도 내부 렌더 데이터 교체를 막아 주지는 않는다.
- 자산이나 렌더 데이터가 교체, 파괴돼도 이미 제출한 GPU 작업이 사용할 자원이 먼저 해제되지 않도록 구현 내부에서 보존한다. 요청 배열, Proxy나 과거 조회 주소를 그대로 저장하는 것으로 대체하지 않는다.
- 종료 순서는 Engine과 UI 정리, `Renderer.Shutdown()`, Renderer 객체 파괴, 창 종료다. `Shutdown`은 남은 GPU 사용과 지연 자원 정리를 처리한다.
- `Initialize`나 프레임 도중 실패해도 Loop의 공통 종료 경로를 거친다. `Shutdown` 전에 `EndFrame`이 항상 성공했다고 가정하면 안 된다.

에디터를 지원할 때는 `IImGuiRendererBackend`도 구현한다. context와 Win32 연결은 기존 EditorUI가 담당한다. GPU backend는 자기 장치에 맞는 처리와 current context 보존 계약을 따른다. `GetTextureRef`의 반환값은 자원을 소유하지 않으며 프레임을 넘어 캐시하지 않는다. `RenderDrawData`를 지연 처리하려면 다음 ImGui 프레임으로 무효화될 데이터와 GPU 자원도 확보해야 한다.

## 6. 생성 함수와 빌드에 연결한다

적용자가 준비한 어댑터를 반환하는 형태는 다음과 같다. 아래는 **호출 형태 예시**이며 `FMyRendererAdapter`와 해당 헤더는 실제 저장소에 존재하지 않는다. 어댑터의 모든 순수 가상 함수 구현이 먼저 필요하다.

```cpp
#include "Renderer/RendererFactory.h"

#include "Core/Templates/UniquePtr.h"
#include "MyRendererAdapter.h" // 적용자가 제공할 헤더

TUniquePtr<IRenderer> CreateRenderer()
{
    return MakeUnique<FMyRendererAdapter>();
}
```

생성 함수는 호출마다 새 미초기화 객체의 단독 소유권을 반환한다. 생성 함수 안에서 새 창이나 GPU 장치를 준비하지 않는다. 생성 실패의 `nullptr`는 실행 실패로 전달되며 기본 D3D11로 대체하지 않는다. 같은 Loop를 다시 실행하면 생성 함수도 다시 호출된다.

빌드 연결은 다음 순서로 확인한다.

1. 현재 [샘플 생성 함수](../Game/Source/RendererFactory.cpp)를 자기 구현을 반환하도록 교체하거나, 샘플 정의를 빌드 대상에서 제거하고 자기 정의 하나만 포함한다. 다른 파일에 두 번째 `CreateRenderer()`를 추가하는 것만으로는 연결되지 않는다.
2. 어댑터와 외부 렌더러의 소스 또는 라이브러리, include, 링크 의존성과 셰이더 등 실행 자산을 등록한다. 기존 외부 앱의 `main`, 창 생성, 게임 서비스와 실행 루프를 함께 가져와 엔진 실행 책임과 중복시키지 않는다.
3. 현재 생성기는 `Engine/Source`, `Engine/Shaders`, `Game/Source`를 수집한다. 이 구성에서는 실행 구성 코드를 `Game/Source`에 둘 수 있다. 파일 추가나 삭제 후 프로젝트를 다시 생성하고 등록 결과를 확인한다.
4. 샘플 파일이 수집 범위에 남아 있다면 프로젝트 목록에서 잠깐 지워도 다음 생성 때 다시 포함된다. 생성 함수 본문을 교체하는 방법이 가장 직접적이다. 파일별 조건이나 `ExcludedFromBuild`를 자동 목록에 추가하는 방식은 현재 생성기가 지원하지 않는다.
5. 외부 라이브러리의 CRT, 도구 집합, Debug와 Release 설정을 현재 실행 프로젝트와 대조하고 두 구성의 최종 링크까지 확인한다. 세부 명령은 [BuildSetup](BuildSetup.md)을 따른다.

현재는 단일 실행 프로젝트가 엔진과 게임 소스를 함께 빌드한다. 별도 Engine 라이브러리, DLL 로딩, 실행 중 교체나 `-renderer` 선택 옵션은 제공하지 않는다. 별도 프로젝트에서 재사용하려면 그 빌드 구성도 설계해야 한다.

## 7. 연결 완료를 판단하는 기준

처음부터 기존 에디터와 샘플 전체를 목표로 잡기보다 적용할 콘텐츠의 최소 기능을 정하고 순서대로 확인한다. 아래 항목은 새 구현의 **검증 기준**이며 이미 그 구현이 검증됐다는 기록은 아니다.

| 단계 | 확인할 내용 |
|---|---|
| 빌드와 생성 연결 | 생성 함수 정의가 하나인지, 기본 샘플 대신 자기 구현이 생성되는지, Debug와 Release 컴파일 및 최종 링크 |
| 초기화와 빈 프레임 | 엔진 창을 빌려 초기화, 크기 변경, 빈 출력의 지움과 표시 요청, 정상 종료 |
| 메시와 시점 | CPU 메시 하나의 생성과 표시, 변환과 투영, 입력 배열 해제 후 사용, 여러 프레임의 데이터 갱신 |
| 텍스처와 타겟 | CPU 이미지, 밉, 주소 설정, 미준비 텍스처의 흰색 대체, 생성 실패 시 이전 출력 보존, 타겟 출력 후 읽기 |
| UI와 에디터 | 게임 이미지의 순서와 알파 합성, 필요한 폰트 형식, ImGui backend, 뷰포트 타겟 쓰기 후 이미지 제출 |
| 실패와 수명 | null 생성, 부분 초기화 실패, 프레임 실패, 미지원 요청, 구현과 장치가 다른 자원 거부, 반복 정리, 같은 Loop 재실행, GPU 완료 뒤 해제 |

컴파일과 링크, 자동 GPU 검사, 실제 실행과 정상 종료, 화면과 입력 관찰은 각각 구분해 기록한다. DX11에 위임하는 검사 구현의 성공만으로 다른 그래픽스 API의 기능 호환성이 확인되는 것은 아니다.

## 8. 엔진 확장이 필요한 경우

현재 데이터로 표현할 수 있는 요청이라면 어댑터 내부 변환과 구현을 우선한다. 외부 렌더러가 쓰는 내부 전역 데이터나 ID를 엔진 전체의 자료형으로 바꾸지 않는다.

현재 공통 계약에 없는 기능이 실제 콘텐츠에 필요할 때는 필요한 입력의 의미, 호출 시점, 소유와 수명, 지원하지 않는 구현의 동작을 정리해 엔진 확장을 검토한다. 예를 들어 외부 렌더러가 요구하는 탄젠트와 엔진 콘텐츠 자체에 새 속성이 필요한 상황은 구분해야 한다. 어댑터에서 계산할지, 공통 형상을 확장할지 실제 요구에 맞춰 판단한다.

새 공통 기능을 추가할 때는 기본 D3D11 구현과 기존 호출부에 미치는 영향도 함께 확인한다. NoTemplate 같은 특정 렌더러의 구조만을 이유로 공통 계약이나 RHI를 먼저 확장하지 않는다.
