# Project Settings

## 현재 구성 — 2026-10-15 재시작

현재는 SampleGame/Source/SampleGame.vcxproj 하나로 SampleGame.exe를 만드는 구성이다. 아래의 이전 구성 기록은 현재 프로젝트·모듈·렌더링 기능의 상태를 나타내지 않는다.

| 항목 | 현재 규칙 |
|---|---|
| 솔루션 | 루트 SeonEngine.slnx는 SampleGame/Source/SampleGame.vcxproj 하나를 참조 |
| 프로젝트 | 실행 파일, 콘솔 서브시스템, x64 Debug/Release |
| 도구 | Visual Studio 2026, MSVC v145, Windows SDK 10.0 계열 |
| 소스 | Engine/Source/EngineMain.cpp, Engine/Source/D3D11/D3D11Device.h/.cpp, Engine/Source/Object.h, Engine/Source/Windows/WindowsApplication.h/.cpp·WindowsHWrapper.h, Engine/Source/Actor.h/.cpp·ActorComponent.h/.cpp·TransformComponent.h/.cpp, Containers/Array.h·Templates/UniquePtr.h·Math/Matrix4.h/.cpp·Math/Quat.h/.cpp·Math/Rotator.h/.cpp·Math/Transform.h/.cpp·Math/Vector3.h·CoreTypes.h |
| 진입점 | main → D3D11 장치 준비 → RunWindowsApplication → D3D11 정리. 생성·조회·표시 → 메시지 펌프·대기 → 창·클래스 정리. 정상 종료 코드는 WM_QUIT에서 받고 API 실패는 1 |
| 엔진 include | $(SERootDir)Engine/Source를 등록하고 기존 include 경로를 상속. EngineMain.cpp는 Windows/WindowsApplication.h를 포함 |
| 파일 등록 | 엔진 EngineMain.cpp와 WindowsApplication·Actor·ActorComponent·TransformComponent의 cpp/header 및 WindowsHWrapper.h·Containers/Array.h·Templates/UniquePtr.h·Math/Matrix4.h/.cpp·Math/Quat.h/.cpp·Math/Rotator.h/.cpp·Math/Transform.h/.cpp·Math/Vector3.h·CoreTypes.h를 명시 등록. 엔진 파일은 ../../Engine/Source 기준. filters는 표시 폴더만 관리. 자동 생성 스크립트는 게임 영역만 검색하므로 실행하지 않음 |
| 엔진·게임 경계 | 같은 프로젝트에 빌드하되 게임 → 엔진 방향. 엔진에서 구체 게임 코드를 참조하지 않음 |
| 공통 설정 | Directory.Build.props/targets를 유지. 별도의 SeonEngine.props는 가져오지 않음 |
| Windows·D3D11 링크 | 실행 프로젝트의 AdditionalDependencies에 d3d11.lib·user32.lib, 기존 링크 의존성 상속 |
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

Math/Quat.h의 FQuat는 float x·y·z·w를 보관하며 기본값 (0, 0, 0, 1)은 회전 없음이다. 각 성분은 축별 각도가 아니다. FRotator 전방 선언과 [[nodiscard]] FRotator ToRotator() const가 있으며 Math/Quat.cpp에서 구현한다. 유한하고 정규화된 입력에서 왼손·고정 축 Z→X→Y 각도를 추출해 도 단위로 반환한다. pitch는 [-90,90], yaw·roll은 [-180,180]도 범위의 대표값이다. cosPitch가 1e-6 이하이면 roll=0인 표현을 고른다. 원본은 유지하며 입력 검사·정규화·별도 생성자는 추가하지 않는다. cpp는 Math/Rotator.h를 포함하므로 호출자는 반환 값을 사용할 때 해당 타입 정의를 포함한다. 임의로 지정한 네 성분을 유효한 회전으로 검사·보정하지 않는다. FQuat는 FTransform의 회전 필드로 사용한다. FTransformComponent가 FTransform을 보관하며 Actor 생성자가 기본 컴포넌트를 만들어 소유한다.

Engine/Source/CoreTypes.h는 Windows/MSVC의 기본 정수 타입에 int8·int16·int32·int64 및 uint8·uint16·uint32·uint64 별칭을 직접 연결하고 sizeof의 1·2·4·8바이트 크기를 static_assert로 확인한다. float·double은 그대로 사용한다. 문자 별칭·플랫폼 타입 계층은 추가하지 않으며 기존 main·SDK 타입·수학 데이터의 선언도 이번에 변경하지 않았다.

Math/Transform.h의 FTransform은 FVector3 position{}, FQuat rotation{}, FVector3 scale{1.0f, 1.0f, 1.0f}를 보관한다. 기본값은 위치 0·회전 없음·크기 1배다. Math/Transform.cpp의 [[nodiscard]] FMatrix4 ToMatrixWithScale() const는 유한한 위치·크기와 정규화된 유한 회전을 전제로 원점 기준 크기 → 회전 → 이동 행렬을 반환하고 원본은 유지한다. cpp에서 XMMatrixAffineTransformation·XMStoreFloat4x4를 사용하고 결과 16개 성분을 FMatrix4로 복사한다. 입력 검사·정규화는 없다. 헤더는 Math/Matrix4.h를 직접 포함하고 cpp를 프로젝트·기존 Math 필터에 등록했다. 기본 FTransform의 행렬은 항등이며 FMatrix4 자체의 영행렬 기본값은 유지한다. 부모 관계·NoScale·역변환·렌더러 연결은 없으며 좌표축·회전 순서는 아래 2026-10-16 결정대로 사용하며 위치 단위는 미정이고 FRotator 각도 입력은 도 단위다. FTransformComponent의 데이터 보관에 사용하며 Actor 생성자가 기본 컴포넌트를 만들어 소유한다.

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

Math/Matrix4.h/.cpp의 FMatrix4는 float m[4][4]를 행 우선으로 보관한다. m[행][열]의 인덱스는 0부터 시작하며 기본값은 모두 0인 영행렬이다. SetIdentity()는 전체를 FMatrix4{}로 초기화한 뒤 네 대각 성분을 1로 설정한다. 이전 자체 엔진과 같은 초기화 방식으로 2026-10-16에 정정했다. header/cpp를 실행 프로젝트와 기존 Math 필터에 등록했다. [[nodiscard]] FMatrix4 operator*(const FMatrix4& other) const는 두 원본을 유지하고 곱을 값으로 반환한다. cpp에서만 DirectXMath의 XMLoadFloat4x4·XMMatrixMultiply·XMStoreFloat4x4를 사용한다. 별도로 채택한 행 벡터 규칙에서 A*B는 A 다음 B를 적용한다. 공개 헤더의 배열·영행렬 기본값과 기존 수학 타입은 유지한다. 역행렬·GPU 전달은 없으며 FTransform의 행렬 생성은 ToMatrixWithScale에서 담당한다. 셰이더 배치는 별도다.

## WindowsApplication 수명 분리 — 2026-10-16 정정

WindowsApplication.h/.cpp에 FWindowsApplication을 두고 instance·window·bClassRegistered를 보관한다. Initialize는 준비·생성·표시와 실패 시 부분 정리 시도, PumpMessages는 스레드 큐 처리, Shutdown은 남은 자원 정리와 성공 여부 반환이다. RunWindowsApplication은 지역 객체로 이들을 조합하고 WaitMessage를 반복 쪽에 유지한다. 복사/이동은 불가하고 Shutdown은 명시적으로 호출한다. 기본 소멸자의 자동 정리는 없다. 빈 Shutdown 반복은 허용하지만 전체 재시작·다중 창/스레드 계약은 없다. SDK 핸들 멤버를 위해 WindowsApplication.h도 WindowsHWrapper.h를 포함한다. 이전 cpp-only·함수 지역 핸들 설명은 이 절로 정정한다. 공개 실행 함수·프로젝트·옵션·기존 창 스타일과 크기는 유지한다.

## D3D11 장치·즉시 컨텍스트 — 2026-10-16

Engine/Source/D3D11/D3D11Device.h/.cpp의 FD3D11Device를 프로젝트에 등록하고 Engine/Source/D3D11 필터를 추가했다. AdditionalDependencies에 d3d11.lib를 추가하며 기존 user32.lib·상속은 유지한다. 헤더는 WindowsHWrapper.h·d3d11.h·wrl/client.h를 사용한다. Initialize는 기본 어댑터의 하드웨어 feature level 11_0을 flags 0으로 요청한다. 생성 실패 HRESULT는 D3D11 카테고리 오류 로그와 false, main은 1로 전달한다. 현재 장치 준비 중복도 false로 거부하며 기존 자원은 보존한다. Shutdown은 context/device ComPtr를 순서대로 Reset하며 소멸자 자동 참조 해제도 가능하다. 복사/이동은 제공하지 않는다.

EngineMain.cpp가 장치를 준비한 뒤 기존 Windows 실행을 호출하고 결과 반환 전에 정리한다. 스왑 체인·창 출력·프레임 갱신·셰이더·렌더링은 아직 없다. SDK의 WRL ComPtr를 직접 사용하며 별도 엔진 COM 래퍼는 만들지 않는다. Debug 구성도 flags 0이므로 D3D 디버그 레이어가 켜졌다는 뜻이 아니다. WARP 자동 전환·어댑터 선택은 없다. 공통 props/targets·의존성 manifest·Windows 소스는 변경하지 않는다.

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
- **대상**: 저장소 안의 모든 `.vcxproj`(`Binaries`, `Intermediate`, `ThirdParty` 제외). `.vcxproj`가 있는 폴더 아래의 `.h` / `.cpp`를 모은다. 필터 이름은 `.vcxproj` 기준 상대 폴더 경로다.
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
