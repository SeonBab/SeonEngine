// 렌더러 defaultRenderRootSig 와 1:1 (D3D12Renderer.h 의 DX12_RootParam / D3D12Renderer_ResCtrl.cpp)
// 모든 월드/에딧 셰이더는 이걸 include 하고 자기 레지스터를 직접 쓰지 않음
#ifndef REGISTERS_HLSLI
#define REGISTERS_HLSLI

// =====================================================================
// b, space0 : 레이어 고정칸
// =====================================================================
cbuffer ConditionCB : register(b0, space0)
{
    matrix g_V;
    matrix g_P;
    matrix g_VP;
    float3 g_EyePos;
    float  g_Time;
    float4 g_LAmb;
    float4 g_LCol;
    float3 g_LDir;      // 빛이 진행하는 방향 (빛 -> 표면). 셰이더에서 뒤집어서 L = -g_LDir 로 씀
    matrix g_LVP[4];    // 전역조명 캐스케이드별 뷰프로젝션 (RENDER_BUILTIN_CSM_MAX, 하나만 쓰면 [0])
    float4 g_LSplit;    // 캐스케이드 i 가 덮는 뷰 깊이의 끝
    float4 g_LTile[4];  // 섀도우 아틀라스 타일 (xy = uv 스케일, zw = uv 오프셋)
};

struct Light
{
    uint   l_Type;      // 0: None, 1: Directional, 2: Point, 3: Spot
    float3 l_Pos;
    float  l_Range;
    float3 l_Col;
    float3 l_Dir;
    float  l_Angle;
};

cbuffer LightCB : register(b1, space0)
{
    Light g_Lights[64];
};

// 조명 행렬: g_Lights[i] 의 뷰프로젝션 g_LightVP[i]
cbuffer LightMatrixCB : register(b2, space0)
{
    matrix g_LightVP[64];
};

cbuffer RootConstantsCB : register(b4, space0)
{
    float4 g_RootConstants;     // 4 DWORD (배경 틴트 등, 렌더러 내부 패스용)
};

// =====================================================================
// b, space1 / space2 : 레이어 데이터 / 액션 데이터 (내용은 사용 측 정의 -> 자리만 매크로로)
// 예) cbuffer MyLayerCB : LAYER_DATA_CB { ... };
// =====================================================================
#define LAYER_DATA_CB   register(b0, space1)
#define ACTION_DATA_CB  register(b0, space2)

// =====================================================================
// b, space3 : 물체 칸 (요청의 dataBoxIds[k] 가 bk 로 옴, 최대 5칸)
// 커스텀 타입은 OBJECT_CB(k) 로 직접 선언. 빌트인 3DGeneral 은 아래 (칸 0), 안쓰면 REGISTERS_NO_BUILTIN_OBJECT
// =====================================================================
#define OBJECT_CB(k)    register(b##k, space3)

#ifndef REGISTERS_NO_BUILTIN_OBJECT
cbuffer ObjectCB : OBJECT_CB(0)        // DataBox_Render3DGeneral
{
    matrix obj_W;
    float4 obj_Col;
    float  obj_Rough;
    float  obj_Metal;
    float2 obj_Pad;
    uint4  obj_Light[2];    // C++ uint32[8] 연속 32B -> ObjLightId(i) 로 읽기
};

// 물체가 받는 조명 칸 i (0~7) -> g_Lights / g_LightVP 인덱스. 언롤된 루프에선 시프트 / 마스크가 상수로 접힘
uint ObjLightId(uint i)
{
    return obj_Light[i >> 2][i & 3];
}
// 칸 1 (DataBox_Render3DOptional) 은 스킨 셰이더에서: cbuffer BoneCB : OBJECT_CB(1) { matrix obj_BoneTr[SKIN_BONE_MAX]; };
#define SKIN_BONE_MAX 512
#endif

// =====================================================================
// t, space0 : 선행 출력 인스턴트 (RT 0~7 = g_SR0~7, DS = g_SR8), 액션 입력 칸 그대로
// =====================================================================
Texture2D g_SR0 : register(t0, space0);
Texture2D g_SR1 : register(t1, space0);
Texture2D g_SR2 : register(t2, space0);
Texture2D g_SR3 : register(t3, space0);
Texture2D g_SR4 : register(t4, space0);
Texture2D g_SR5 : register(t5, space0);
Texture2D g_SR6 : register(t6, space0);
Texture2D g_SR7 : register(t7, space0);
Texture2D g_SR8 : register(t8, space0);

// =====================================================================
// t, space1 : CTex (texProp 슬롯 8칸, g_Tex[0~7])
// =====================================================================
Texture2D g_Tex[] : register(t0, space1);

// =====================================================================
// t, space4 : 레이어 전역 텍스처 (환경맵 큐브, 레이어당 한번 바인딩)
// 레이어에 환경맵이 없으면 기본 2D 텍스처가 묶여 있으니 샘플링은 환경맵 있을 때만
// =====================================================================
TextureCube g_EnvMap : register(t0, space4);

// =====================================================================
// s : 정적 샘플러
// =====================================================================
SamplerState           g_SamplerAnisoWrap   : register(s0);
SamplerState           g_SamplerLinearClamp : register(s1);
SamplerState           g_SamplerPointClamp  : register(s2);
SamplerState           g_SamplerLinearWrap  : register(s3);
SamplerState           g_SamplerPointWrap   : register(s4);
SamplerComparisonState g_SamplerShadow      : register(s5);
SamplerState           g_SamplerAnisoClamp  : register(s6);

#endif
