#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include <DirectXMath.h>

// 데이터허브 내 자료형 정의 (리소스 타입 / 데이터박스)


namespace Hakko::DataHub
{

    #pragma region Typedef

    typedef uint32_t HashedString;


    typedef uint32_t DataBoxID;
    typedef DataBoxID DtBID;


    typedef uint64_t RenderID;

    enum RenderIDType
    {
        RenderIdType_None,          // ActionEdit / Compute 에서 활용
        RenderIdType_Opaque,
        RenderIdType_Translucent,
        RenderIdType_Shadow,
        RenderIdType_Screen,
    };
    

    #pragma region Vertex

    // 정점 타입 (Simple ⊂ Static ⊂ Skinned)
    enum VertexType
    {
        VertexType_Simple = 0,
        VertexType_Static,
        VertexType_Skinned,
    };


    struct VertexSimple
    {
        DirectX::XMFLOAT3 Pos;
    };

    struct VertexStatic : VertexSimple
    {
        DirectX::XMFLOAT2 UV;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tangent;
        DirectX::XMFLOAT4 Color;
    };

    struct VertexSkinned : VertexStatic
    {
        uint16_t BoneIdx[4];
        float BoneWeight[4];
    };

    typedef uint32_t VIndex;

    #define SKIN_BONE_MAX 512


    struct SubMesh
    {
        uint32_t indexStart;
        uint32_t indexCount;
        uint32_t baseVertex;
    };

    typedef uint16_t SubMeshID;

    
    struct Mesh
    {
        VertexType vertexType;

        std::vector<VertexSimple> verticesSimple;
        std::vector<VertexStatic> verticesStatic;
        std::vector<VertexSkinned> verticesSkinned;

        std::vector<VIndex> indices;
        std::vector<SubMesh> subMeshes;     // 비어있으면 전체 서브메쉬 0
    };
    
    typedef uint16_t MeshID;

    #pragma endregion


    #pragma region Texture

    // 파일의 압축포맷, 확장자
    enum TextureFileFormat
    {
        TextureFileFormat_None,
        TextureFileFormat_DDS,
    };

    // 텍스처 차원 (v0.1-alpha 변경: 색공간 분리, Diffuse / CubeMap 이 sRGB 로 올려주던 동작 제거)
    enum TextureType
    {
        TextureType_2D,
        TextureType_Cube,       // 2D 6장 묶음 (arraySize 는 6의 배수)
        TextureType_1D,
        TextureType_3D,

        TextureType_Default = TextureType_2D,
        TextureType_Diffuse = TextureType_2D,
        TextureType_CubeMap = TextureType_Cube,
        TextureType_LUT = TextureType_1D,
        TextureType_VolumeMap = TextureType_3D,
    };

    // 파일로드 용, 색공간 해석
    enum TextureColorSpace
    {
        TextureColorSpace_Auto,     // 파일에 적힌 대로
        TextureColorSpace_SRGB,     // 색 (알베도 / 스카이박스 / UI) sRGB 변형이 없는 UNORM 은 RGBA8_SRGB 로
        TextureColorSpace_Linear,   // 데이터 (노멀 / ORM / 마스크)
    };

    // 텍스처의 압축포맷
    enum class TextureReadFormat
    {
        Unknown,

        // 컬러 (비압축)
        R8_UNorm,
        RG8_UNorm,
        RGBA8_UNorm,
        RGBA8_SRGB,
        BGRA8_UNorm,
        BGRA8_SRGB,
        R10G10B10A2_UNorm,
        R11G11B10_Float,
        R16_Float,
        RG16_Float,
        RGBA16_Float,
        R32_Float,
        RG32_Float,
        RGBA32_Float,
        R16_UNorm,
        RG16_UNorm,
        RGBA16_UNorm,
        RG8_SNorm,
        RGBA8_SNorm,
        RG16_SNorm,
        R8_UInt,                // 정수 (피킹 id / 마스크), 블렌드 / 필터링 불가
        R16_UInt,
        R32_UInt,
        RGB9E5_SharedExp,       // HDR 읽기 전용 (RT / Compute 불가)

        // 블록 압축 (파일 텍스처용)
        BC1_UNorm,  BC1_SRGB,
        BC2_UNorm,  BC2_SRGB,
        BC3_UNorm,  BC3_SRGB,
        BC4_UNorm,  BC4_SNorm,
        BC5_UNorm,  BC5_SNorm,
        BC6H_UF16,  BC6H_SF16,
        BC7_UNorm,  BC7_SRGB,

        // 깊이/스텐실
        D16_UNorm,
        D24_UNorm_S8_UInt,
        D32_Float,
        D32_Float_S8X24_UInt,
    };

    struct TextureData {
        TextureReadFormat format;
        TextureType textureType;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arraySize = 1;
    };

    struct SubresourceData {
        size_t offset;
        uint32_t rowPitch;
        uint32_t slicePitch;
    };

    enum TextureUsage
    {
        TextureUsage_ReadOnly,
        TextureUsage_RT,        // 미구현: 아직 검증 전이라 생성 막아놓음
        TextureUsage_DS,        // 미구현
        TextureUsage_Compute,   // 미구현
    };


    struct TextureClearValue
    {
        float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float depth = 1.0f;
        uint8_t stencil = 0;
    };

    struct Texture {
        TextureData data;
        std::vector<SubresourceData> subresources;
        std::vector<std::byte> rawData;
        TextureUsage usage = TextureUsage_ReadOnly;
        TextureClearValue clearValue;
    };

    typedef uint16_t TexID;

    #pragma endregion


    #pragma region Shader

    enum ShaderType
    {
        ShaderType_VS,
        ShaderType_PS,
        
        ShaderType_CS,
    };

	typedef std::pair<std::wstring, ShaderType> ShaderInfo;

    typedef uint32_t ShaderID;

    #pragma endregion


    // 미구현 (컴퓨트 자원)
    #pragma region ComputeBuffer

    struct ComputeBuffer
    {
        uint32_t stride = 0;
        uint32_t count = 0;
        std::vector<uint8_t> data;
    };
    typedef ComputeBuffer CSBuffer;

    typedef uint16_t CSBufferID;


    struct CSMeshLinkData
    {
        MeshID linkMeshId;

        CSBufferID csBufferId;
        std::vector<VIndex> indices;
        std::vector<SubMesh> subMeshes;
    };

    #pragma endregion


    #pragma region Prop

    // (RTB)Action.depthOpt = ForceDepthRead 설정 시 쓰기 강제 꺼짐
    // DS 없는 출력 시 무시
    enum DrawProp_DepthUsage
    {
        DrawProp_DepthUsage_Opaque,         // 깊이 LESS / 쓰기 있음 (기본값)
        DrawProp_DepthUsage_Translucent,    // 깊이 LESS / 쓰기 없음
        DrawProp_DepthUsage_Screen,         // 깊이 테스트 / 쓰기 모두 끔
        DrawProp_DepthUsage_Background,     // 깊이 LESS_EQUAL / 쓰기 없음
        DrawProp_DepthUsage_Equal,          // 깊이 EQUAL / 쓰기 없음
        DrawProp_DepthUsage_Always,         // 깊이 ALWAYS / 쓰기 있음
    };

    enum DrawProp_CullType
    {
        DrawProp_CullType_Back,
        DrawProp_CullType_Front,
        DrawProp_CullType_None,
    };

    // RT 없는 출력 시 무시
    enum DrawProp_BlendType
    {
        DrawProp_BlendType_None,
        DrawProp_BlendType_Alpha,
        DrawProp_BlendType_Addtive,
        DrawProp_BlendType_Multiply,
    };

    enum DrawProp_FillType
    {
        DrawProp_FillType_Solid,
        DrawProp_FillType_Wireframe,
    };

    enum DrawProp_DepthBias
    {
        DrawProp_DepthBias_None,
        DrawProp_DepthBias_ShadowCaster,
        DrawProp_DepthBias_Decal,
    };

    #define DEPTHBIAS_SHADOWCASTER_BIAS 1000
    #define DEPTHBIAS_SHADOWCASTER_SLOPE 1.5f
    #define DEPTHBIAS_SHADOWCASTER_CLAMP 0.0f
    #define DEPTHBIAS_DECAL_BIAS -1000
    #define DEPTHBIAS_DECAL_SLOPE -1.0f
    #define DEPTHBIAS_DECAL_CLAMP 0.0f

    // 미구현
    struct DrawProp_StencilOption
    {
        bool useStencil;
    };

    enum DrawProp_Topology
    {
        DrawProp_Topology_TriangleList,
        DrawProp_Topology_TriangleStrip,
        DrawProp_Topology_LineList,
        DrawProp_Topology_LineStrip,
        DrawProp_Topology_PointList,
    };

    struct DrawProp
    {
        DrawProp_DepthUsage depthUsage;
        DrawProp_CullType cullType;
        DrawProp_BlendType blendType;
        DrawProp_FillType fillType;
        DrawProp_DepthBias depthBias;
        DrawProp_StencilOption stecilOpt;
        DrawProp_Topology topology;

        VertexType vertexType = VertexType_Static;   // 기본값 Static
        ShaderID VSId;
        ShaderID PSId;
    };

    typedef uint16_t DrawPropID;


    // customizable (< 256)
    #define TEXPROP_SLOT_COUNT 8

    struct TextureProp
    {
        TexID texId[TEXPROP_SLOT_COUNT] = {0};
        uint16_t texMipLevels[TEXPROP_SLOT_COUNT] = {0};
    };
    typedef TextureProp TexProp;

    typedef uint16_t TexPropID;

    #pragma endregion

    #pragma endregion



    #pragma region Report

    enum HubReportKind
    {
        HubReport_Contract,
        HubReport_Limit,
        HubReport_Load,
        HubReport_Internal,
    };

    struct HubReport
    {
        HubReportKind kind;
        const wchar_t* msg;
    };

    constexpr size_t HUB_REPORT_MAX = 256;
    extern std::vector<HubReport> HubReportList;

    namespace Internal
    {
        void PushHubReport(HubReportKind kind, const wchar_t* msg);
    }


    enum RenderReportKind
    {
        RenderReport_Invariant,
        RenderReport_Contract,
        RenderReport_Limit,
        RenderReport_Init,
    };

    struct RenderReport
    {
        RenderReportKind kind;
        const wchar_t* msg;
    };

    extern std::vector<RenderReport> RenderReportList;

    #pragma endregion

    

    #pragma region DataBox

    struct DataBox
    {
    protected:
        DataBox() = default;
    };
    typedef DataBox DtB;

    enum DataBoxType
    {
        DataBoxType_None = 0,
        DataBoxType_CSBuffer = 1,               // 드로우 데이터 칸에 컴퓨트 버퍼 (id = CSBufferID, 구조버퍼로 바인딩), 프로바이더 없음, 액션당 한 칸
        DataBoxType_DataBox_RenderCondition = 2,
        DataBoxType_DataBox_RenderLight = 3,
        DataBoxType_DataBox_RenderLightMatrix = 4,
        DataBoxType_DataBox_RenderShadowView = 5,
        DataBoxType_DataBox_Render3DGeneral = 6,
        DataBoxType_DataBox_Render3DOptional = 7,
        DataBoxType_DataBox_Render2DScreen = 8,

        DataBoxType_CustomizableBegin = 9,

        DataBoxType_DtB_Condition = DataBoxType_DataBox_RenderCondition,
        DataBoxType_DtB_Light = DataBoxType_DataBox_RenderLight,
        DataBoxType_DtB_LightMatrix = DataBoxType_DataBox_RenderLightMatrix,
        DataBoxType_DtB_3DGeneral = DataBoxType_DataBox_Render3DGeneral,
        DataBoxType_DtB_3DOptional = DataBoxType_DataBox_Render3DOptional,
        DataBoxType_DtB_2DScreen = DataBoxType_DataBox_Render2DScreen,
        DataBoxType_DtB_ShadowView = DataBoxType_DataBox_RenderShadowView,
    };

    typedef uint32_t DataBoxTID;
    typedef DataBoxTID DtBTID;


    // 렌더러용 커스텀 베이스 (DataBoxCustomizer를 이용하세요)
    struct alignas(256) DataBox_Custom : public DataBox
    {
    protected:
        DataBox_Custom() = default;
    };
    typedef DataBox_Custom DtB_Custom;


    #pragma region RenderData

    
    // 특수화된 데이터 (커스터마이즈 불가)
    #pragma region ConditionData

    #define RENDER_BUILTIN_CSM_MAX 4

    struct alignas(256) DataBox_RenderCondition : DataBox
    {
        DirectX::XMMATRIX camera_View;
        DirectX::XMMATRIX camera_Proj;
        DirectX::XMMATRIX camera_ViewProj;
        DirectX::XMFLOAT3 camera_EyePos;
        float global_TotalTime;
        DirectX::XMFLOAT4 global_LightAmbient;
        DirectX::XMFLOAT4 global_LightColor;
        DirectX::XMFLOAT3 global_LightDirection;
        DirectX::XMMATRIX global_LightViewProj[RENDER_BUILTIN_CSM_MAX];     // 전역조명 캐스케이드별 뷰프로젝션 (하나만 쓰면 [0])
        DirectX::XMFLOAT4 global_LightCascadeSplit;                         // 캐스케이드 i 가 덮는 뷰 깊이의 끝 (안 쓰는 칸 0)
        DirectX::XMFLOAT4 global_LightCascadeTile[RENDER_BUILTIN_CSM_MAX];  // 섀도우 아틀라스 타일 (xy = uv 스케일, zw = uv 오프셋)
    };
    typedef DataBox_RenderCondition RenderCondition, DtB_Condition;


    struct DataBox_RenderLight : DataBox
    {
        uint32_t type;
        DirectX::XMFLOAT3 position;
        
        float range;
        DirectX::XMFLOAT3 color;
        
        DirectX::XMFLOAT3 direction;
        float spotAngle;
    };
    typedef DataBox_RenderLight DtB_Light;

    extern std::vector<DataBox_RenderLight> RenderLightList;
    extern std::vector<DataBoxID> blankedLightIdList;


    struct DataBox_RenderLightMatrix : DataBox
    {
        DirectX::XMMATRIX lightViewProj;
    };
    typedef DataBox_RenderLightMatrix DtB_LightMatrix;
    
    extern std::vector<DataBox_RenderLightMatrix> RenderLightMatrixList;
    extern std::vector<DataBoxID> blankedLightMatrixIdList;

    #pragma endregion


    // 빌트인 파이프라인이랑 쓰기 좋은 기본제공 데이터타입
    #pragma region BuiltinData

    struct alignas(256) DataBox_Render3DGeneral : DataBox_Custom
    {
        DirectX::XMMATRIX worldMat;
        DirectX::XMFLOAT4 blendColor;
        float roughness;
        float metalic;
        float _[2];
        uint32_t affectLights[8];
    };
    typedef DataBox_Render3DGeneral DtB_3DGeneral;

    struct alignas(256) DataBox_Render3DOptional : DataBox_Custom
    {
        DirectX::XMMATRIX boneTransforms[SKIN_BONE_MAX];
    };
    typedef DataBox_Render3DOptional DtB_3DOptional;


    // 스크린 쿼드 사용, 좌표는 뷰포트 비율 0~1, 원점 좌상단
    // 미구현: 렌더러 스크린 드로우 경로 셰이더가 없음
    struct alignas(256) DataBox_Render2DScreen : DataBox_Custom
    {
        DirectX::XMFLOAT4 rect;
        DirectX::XMFLOAT4 uvRect;
        DirectX::XMFLOAT2 pivot;        // 회전 중심 (rect 내부 비율)
        float rotation;                 // 라디안, 시계 방향
        float _;
        DirectX::XMFLOAT4 blendColor;
    };
    typedef DataBox_Render2DScreen DtB_2DScreen;


    // 미구현 (기본 섀도우 셰이더 / 드로우프롭 전)
    // 섀도우 패스가 그리는 용도의 조명행렬, 읽는 쪽 (OPQ 등) 은 레이어 전역 컨디션 / 조명행렬 데이터로
    struct alignas(256) DataBox_RenderShadowView : DataBox_Custom
    {
        DirectX::XMMATRIX viewProj;
    };
    typedef DataBox_RenderShadowView DtB_ShadowView;

    #pragma endregion

    #pragma endregion

    #pragma endregion

}
