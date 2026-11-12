#pragma once

#include "D3D12Includes.h"
#include "RenderOption.h"
#include "DataHub.h"

#include <memory>

namespace Hakko
{

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#pragma region Typedefs

typedef DataHub::MeshID MeshID;

typedef DataHub::SubMeshID SubMeshID;


typedef DataHub::TexID TexID, STexID;


typedef DataHub::TexPropID CTexID;


typedef DataHub::ShaderID ShaderID;

typedef DataHub::CSBufferID CSBufferID;

typedef DataHub::DrawProp PSOBody;

typedef DataHub::DrawPropID PSOID;

typedef uint8_t OutputSignatureID;


typedef DataHub::RenderID RenderID;

typedef DataHub::ActionID ActionID;


constexpr uint16_t DEFAULT_RES_MESH_COUNT = RESERVED_MESH_COUNT;
constexpr uint16_t DEFAULT_RES_TEX_COUNT = RESERVED_TEXTURE_COUNT;
constexpr uint16_t DEFAULT_RES_CSBUFFER_COUNT = RESERVED_CSBUFFER_COUNT;
constexpr uint32_t DEFAULT_RES_SHADER_COUNT = RESERVED_SHADER_COUNT;
constexpr uint16_t DEFAULT_RES_PSOBODY_COUNT = RESERVED_DRAWPROP_COUNT;
constexpr uint16_t DEFAULT_RES_CTEX_COUNT = RESERVED_TEXPROP_COUNT;

constexpr uint32_t DEFAULT_SHADER_COMPOSITE_PS = 1;
constexpr uint32_t DEFAULT_SHADER_SKYBOX_VS = 2;
constexpr uint32_t DEFAULT_SHADER_SKYBOX_PS = 3;
constexpr uint32_t DEFAULT_SHADER_BG_VS = 4;
constexpr uint32_t DEFAULT_SHADER_BG_PS = 5;
constexpr uint32_t DEFAULT_SHADER_WORLD_VS = RESERVED_SHADER_WORLD_VS_ID;
constexpr uint32_t DEFAULT_SHADER_WORLD_OPQ_PS = RESERVED_SHADER_WORLD_OPQ_PS_ID;
constexpr uint32_t DEFAULT_SHADER_WORLD_TP_PS = RESERVED_SHADER_WORLD_TP_PS_ID;
static_assert(DEFAULT_SHADER_BG_PS + 1 == DEFAULT_SHADER_WORLD_VS, "렌더러 내부 셰이더 칸과 허브 기본 제공 셰이더 칸이 안 이어짐");
static_assert(DEFAULT_SHADER_WORLD_TP_PS + 1 == DEFAULT_RES_SHADER_COUNT, "빌트인 셰이더 수가 허브 예약 (RESERVED_SHADER_COUNT) 과 다름");

constexpr uint16_t DEFAULT_TEX_TRANSPARENT = RESERVED_TEX_TRANSPARENT_ID;


#pragma region RenderResource

#pragma region Vertex

struct DX12_Mesh{
	ComPtr<ID3D12Resource> VB;
	ComPtr<ID3D12Resource> IB;

	DataHub::VertexType vertexType = DataHub::VertexType_Simple;
	UINT vertexCount = 0;
	UINT indexCount = 0;
};

struct DX12_SubMesh{
	UINT indexStart;
	UINT indexCount;
	INT baseVertex;
};

#pragma endregion


#pragma region Buffer

constexpr D3D12_RESOURCE_STATES CSBUFFER_HOME_STATE =
	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

constexpr UINT CSBUFFER_DUMMY_STRIDE = 4;
constexpr UINT CSBUFFER_DUMMY_COUNT = 64;

struct DX12_Buffer{
	ComPtr<ID3D12Resource> Res;
	UINT stride = 0;
	UINT count = 0;
};

#pragma endregion


#pragma region Texture

enum DX12_TextureUsage{
	DX12_TextureUsage_ReadOnly = DataHub::TextureUsage_ReadOnly,
	DX12_TextureUsage_RTV = DataHub::TextureUsage_RT,
	DX12_TextureUsage_DSV = DataHub::TextureUsage_DS,
	DX12_TextureUsage_UAV = DataHub::TextureUsage_Compute,
};


struct DX12_TextureLoaded{
    ComPtr<ID3D12Resource> Tex;
	UINT16 ArraySize;
	UINT16 MipLevels;
	DXGI_FORMAT Format;
	D3D12_RESOURCE_DIMENSION Dimension;
	bool IsCube;
};
typedef DX12_TextureLoaded DX12_Texture;

struct DX12_TextureInstant{
    ComPtr<ID3D12Resource> Tex;

	DX12_TextureUsage usage;
};


struct DX12_InstantLogical{
	uint32_t firstUse;
	uint32_t lastUse;
	uint32_t ownerActionIdx;
	bool readAfterWrite;
	uint16_t physicalIdx;
};

#define TEX_INSTANT_NONE UINT16_MAX
#define TEX_INSTANT_WANT (UINT16_MAX - 1)

#define INSTANT_SLOT_MAX (RENDER_TARGET_MAX + 1)
#define INSTANT_SLOT_DS RENDER_TARGET_MAX

#pragma endregion


#pragma region ResourceRefs

struct DX12_MeshView{
	D3D12_VERTEX_BUFFER_VIEW VBV = {};
	D3D12_INDEX_BUFFER_VIEW IBV = {};		// R16_UINT
	std::vector<DX12_SubMesh> subMeshes;	// 최소 1개
};


#define CTEX_COMBINED_SIZE TEXPROP_SLOT_COUNT

struct DX12_CombinedTextureView{
	UINT RVHeapIndex;
};
typedef DX12_CombinedTextureView DX12_CTexView;

struct DX12_SingleTextureView{
	UINT RVHeapIndex;
};
typedef DX12_SingleTextureView DX12_STexView;

struct DX12_WritableTextureView{
	UINT VHeapIndex = UINT_MAX;		// 최댓값 빈값용으로 새김
};
typedef DX12_WritableTextureView DX12_WTexView;


struct DX12_WritableTextureRef{
	DX12_TextureUsage usage = DX12_TextureUsage_ReadOnly;
	uint16_t idx = 0;
};
typedef DX12_WritableTextureRef DX12_WTexRef;


struct DX12_BufferView{
	D3D12_GPU_VIRTUAL_ADDRESS address = 0;
};

#pragma endregion


#pragma region PSOVariant

#define SHADER_VARIANT_NONE 0
struct DX12_CompiledShader{
	ShaderID shaderId;
	uint8_t variant;
	bool failed;
	ComPtr<IDxcBlob> blob;
};

struct DX12_RenderOutputSignature{
	UINT64 RTFormats;
	UINT64 RTBlendNDSFormat;
};
typedef DX12_RenderOutputSignature DX12_OutputSignature;

struct DX12_OutputSignatureDesc{
	DXGI_FORMAT rtFormats[RENDER_TARGET_MAX];
	DataHub::Action_Output_BlendOption rtBlendOpts[RENDER_TARGET_MAX];
	DXGI_FORMAT dsFormat;
	DataHub::Action_Output_DepthOption dsOpt;
	uint8_t sampleCount;
	DataHub::Action_OutputLayout outputLayout;
};

#define OUTSIG_RTFORMAT_BITS	8
#define OUTSIG_RTBLEND_BITS		3
#define OUTSIG_DSFORMAT_SHIFT	(OUTSIG_RTBLEND_BITS * RENDER_TARGET_MAX)
#define OUTSIG_DSOPT_SHIFT		(OUTSIG_DSFORMAT_SHIFT + 8)
#define OUTSIG_SAMPLE_SHIFT		(OUTSIG_DSOPT_SHIFT + 8)
#define OUTSIG_LAYOUT_SHIFT		(OUTSIG_SAMPLE_SHIFT + 8)


#define OUTPUT_SIGNATURE_MAX 32
#define OUTPUT_SIGNATURE_INVALID 0

// PSOID + OutputSigID -> psoVariant
struct DX12_PSOGenerator{
	PSOBody psoBody;
	ComPtr<ID3D12PipelineState> psoVariants[OUTPUT_SIGNATURE_MAX];
};


struct DX12_ComputePSO{
	ComPtr<ID3D12PipelineState> pso;
	bool failed;
};

#pragma endregion

#pragma endregion


#pragma region RootSignature

enum DX12_RootParam
{
	RootParam_ObjectCB0			= 0,								// b0~b4 space3 : 물체 칸 k = ObjectCB0 + k (DRAWREQ_DATABOX_MAX 개, CBV 2 DWORD 씩)
	RootParam_CTexTable			= RootParam_ObjectCB0 + DRAWREQ_DATABOX_MAX,	// t0.. space1 : CTex 언바운드 테이블 (1)
	RootParam_StructuredSB,											// t0 space2 : 구조버퍼 (루트 SRV 2 DWORD), 드로우 데이터 칸 tid = DataBoxType_CSBuffer (액션당 한 칸)
	RootParam_InstantSR0,											// t0~t8 space0 : 선행 출력 인스턴트, 1칸 테이블 x 9 (칸 s = InstantSR0 + s)
	RootParam_ActionCB			= RootParam_InstantSR0 + INSTANT_SLOT_MAX,	// b0 space2 : 액션 데이터 (2)
	RootParam_LayerDataCB,											// b0 space1 : 레이어 데이터 (2)
	RootParam_ConditionCB,											// b0 space0 : 카메라 등 (2)
	RootParam_LightCB,												// b1 space0 : 조명 64 (2)
	RootParam_LightMatrixCB,										// b2 space0 : 조명 행렬 64 (2)
	RootParam_LayerTexTable,										// t0 space4 : 레이어 전역 텍스처 (환경맵 큐브) 1칸 테이블 (1)
	RootParam_Constants,											// b4 space0 : 루트 상수 4 DWORD (배경 틴트 등)
	RootParam_Count,
};

enum DX12_RootParamCompute
{
	RootParamCS_ReadBuffer0		= 0,												// t0~t3 space0 : 읽기 버퍼 k = ReadBuffer0 + k (COMPUTE_BINDING_MAX 개, 루트 SRV 2 DWORD 씩)
	RootParamCS_WriteBuffer0	= RootParamCS_ReadBuffer0 + COMPUTE_BINDING_MAX,	// u0~u3 space0 : 쓰기 버퍼 (루트 UAV 2 DWORD 씩)
	RootParamCS_ReadTex0		= RootParamCS_WriteBuffer0 + COMPUTE_BINDING_MAX,	// t0~t3 space1 : 읽기 텍스처 1칸 테이블 x 4 (STex)
	RootParamCS_WriteTex0		= RootParamCS_ReadTex0 + COMPUTE_BINDING_MAX,		// u0~u3 space1 : 쓰기 텍스처 1칸 테이블 x 4 (WTex, usage Compute)
	RootParamCS_InstantSR0		= RootParamCS_WriteTex0 + COMPUTE_BINDING_MAX,		// t0~t8 space2 : 선행 출력 인스턴트 1칸 테이블 x 9
	RootParamCS_ActionCB		= RootParamCS_InstantSR0 + INSTANT_SLOT_MAX,		// b0 space2 : 액션 데이터 (2)
	RootParamCS_LayerDataCB,														// b0 space1 : 레이어 데이터 (2)
	RootParamCS_ConditionCB,														// b0 space0 : 카메라 등 (2)
	RootParamCS_Count,
};

#pragma endregion


#pragma region RenderData

struct DX12_RenderData{
	uint32_t CBAreaIndex;
};

#pragma endregion



#pragma region RenderAction

#pragma region RenderRequest

struct DX12_RenderRequestProxy{
	RenderID renderId;
	uint32_t requestIdx;
};

struct DX12_RenderDrawData{
	RenderID renderId;
	MeshID meshId;
	SubMeshID subMeshId;
	DX12_RenderData datas[DRAWREQ_DATABOX_MAX];
};

#pragma endregion


#pragma region Barrier

enum DX12_BarrierTarget{
	DX12_BarrierTarget_Loaded,		// id = texId
	DX12_BarrierTarget_RTInstant,	// id = 인스턴트 인덱스
	DX12_BarrierTarget_DSInstant,
	DX12_BarrierTarget_Buffer,		// id = csBufferId
};


struct DX12_ActionBarrier{
	uint16_t id;
	DX12_BarrierTarget target;
	D3D12_RESOURCE_STATES before;
	D3D12_RESOURCE_STATES after;
};

#pragma endregion


enum DX12_RenderActionType{
	DX12_RenderActionType_Draw,
	DX12_RenderActionType_Edit,
	DX12_RenderActionType_Compute,
	DX12_RenderActionType_Builtin,
};

// TODO 아직 커스텀 모드, 그래프 노드분석은 미구현
struct DX12_RenderActionNode{
	uint32_t nodeDepth;
	ActionID frontIds[RENDER_ACTION_NODE_MAX];
	ActionID actionId;
};

enum DX12_BuiltinActionId : uint32_t{
	DX12_BuiltinActionId_Base = 0,
	DX12_BuiltinActionId_PreCompute0,
	DX12_BuiltinActionId_EarlyZ = DX12_BuiltinActionId_PreCompute0 + RENDER_BUILTIN_PRECOMPUTE_MAX,
	DX12_BuiltinActionId_GBuffer,
	DX12_BuiltinActionId_AO,
	DX12_BuiltinActionId_AOBlur,
	DX12_BuiltinActionId_GlobalSDW0,
	DX12_BuiltinActionId_LocalSDW0 = DX12_BuiltinActionId_GlobalSDW0 + RENDER_BUILTIN_CSM_MAX,
	DX12_BuiltinActionId_OPQ = DX12_BuiltinActionId_LocalSDW0 + RENDER_LIGHT_MAX,
	DX12_BuiltinActionId_Lighting,
	DX12_BuiltinActionId_Background,
	DX12_BuiltinActionId_TP,
	DX12_BuiltinActionId_PP0,
	DX12_BuiltinActionId_SC = DX12_BuiltinActionId_PP0 + RENDER_BUILTIN_PP_MAX,
	DX12_BuiltinActionId_Debug,

	DX12_BuiltinActionId_Count,
};


#define CS_SLOT_WRITE0 0
#define CS_SLOT_READ0 COMPUTE_BINDING_MAX

struct DX12_RenderAction
{
	DX12_RenderActionType actionType;
	DataHub::BuiltinActionType builtinType;
	DataHub::RenderIDType renderIdType;
	std::array<uint32_t, RENDER_ACTION_NODE_MAX> frontNodeIdx;
	
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	OutputSignatureID outputSigId;
	DataHub::Action_OutputLayout outputLayout;

	std::array<uint16_t, INSTANT_SLOT_MAX> slotIds;
	uint16_t slotFlags;
	std::array<uint16_t, INSTANT_SLOT_MAX> srInstantIdx;
	std::array<uint16_t, INSTANT_SLOT_MAX> copySrcInstantIdx;

	bool dsReadOnly;
	std::array<DataHub::Action_Output_LoadOption, RENDER_TARGET_MAX> rtLoadOpts;
	DataHub::Action_Output_LoadOption dsLoadOpt;

	std::vector<DX12_ActionBarrier> copyBarriers;
	std::vector<DX12_ActionBarrier> beginBarriers;
	std::vector<DX12_ActionBarrier> endBarriers;

	DataHub::DataBoxTID actionDataTid;
	DX12_RenderData actionRenderData;

	std::array<DataHub::DataBoxTID, DRAWREQ_DATABOX_MAX> dataTids;
};

#pragma endregion


#pragma region RenderLayer

struct DX12_RenderLayer
{
	TexID bgTexId;
	bool bgIsCube;
	float bgColor[4];
	TexID envTexId;

	DX12_RenderData conditionRenderData;
	DX12_RenderData lightRenderData;
	DX12_RenderData lightMatrixRenderData;

	DataHub::DataBoxTID layerDataTid;
	DX12_RenderData layerRenderData;

	std::vector<DX12_RenderActionNode> renderActionNodeList;
	std::vector<DX12_RenderAction> renderActionList;

	std::vector<DX12_ActionBarrier> finalBarriers;

	uint16_t rtInstantCount;
	uint16_t dsInstantCount;

	uint16_t rtInstantOffset;
	uint16_t dsInstantOffset;

	std::vector<std::vector<DX12_RenderRequestProxy>> actionRenderRequestList;
	std::vector<std::vector<DX12_RenderDrawData>> actionSortedRenderDataList;
};

#pragma endregion


#pragma region RenderFrame

struct DX12_RenderFrame
{
	std::vector<DX12_RenderLayer> layerList;
	
	uint16_t rtInstantNeedCount;
	uint16_t dsInstantNeedCount;

	bool frameInvalidated;
};
typedef DX12_RenderFrame DX12_RenderCanvas;

#pragma endregion

#pragma endregion


#pragma region Helper

inline static void DebugReport(DataHub::RenderReportKind kind, const wchar_t* msg)
{
#if RENDER_DEBUG_OUTPUT_MODE == RENDER_DEBUG_OUTPUT_ENABLE
	OutputDebugStringW(msg);
	OutputDebugStringW(L"\n");
#endif
#if RENDER_REPORT_SEND_MODE == RENDER_REPORT_SEND_ENABLE
	DataHub::RenderReportList.push_back({ kind, msg });
#endif
#ifdef _DEBUG
	if (kind == DataHub::RenderReport_Invariant)
		__debugbreak();
#endif
}


inline static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT resFormat)
{
	switch (resFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS:		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case DXGI_FORMAT_R32_TYPELESS:			return DXGI_FORMAT_D32_FLOAT;
	case DXGI_FORMAT_R32G8X24_TYPELESS:		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case DXGI_FORMAT_R16_TYPELESS:			return DXGI_FORMAT_D16_UNORM;
	default:								return resFormat;
	}
}

inline static DXGI_FORMAT GetSRVFormat(DXGI_FORMAT resFormat)
{
	switch (resFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS:		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_R32_TYPELESS:			return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R32G8X24_TYPELESS:		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case DXGI_FORMAT_R16_TYPELESS:			return DXGI_FORMAT_R16_UNORM;
	default:								return resFormat;
	}
}

inline static D3D12_CLEAR_FLAGS GetDSClearFlags(DXGI_FORMAT dsvFormat)
{
	switch (dsvFormat)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:	return D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
	default:								return D3D12_CLEAR_FLAG_DEPTH;
	}
}


inline bool IsOutInstant(const DX12_RenderAction& action, size_t slot)
{
	return action.actionType != DX12_RenderActionType_Compute && ((action.slotFlags >> slot) & 1) != 0;
}

inline uint16_t OutInstantIdx(const DX12_RenderAction& action, size_t slot)
{
	return IsOutInstant(action, slot) ? action.slotIds[slot] : (uint16_t)TEX_INSTANT_NONE;
}

inline TexID OutTexId(const DX12_RenderAction& action, size_t slot)
{
	if (action.actionType == DX12_RenderActionType_Compute || IsOutInstant(action, slot))
		return 0;

	return action.slotIds[slot];
}

inline void SetOutInstant(DX12_RenderAction& action, size_t slot, uint16_t instIdx)
{
	if (instIdx == TEX_INSTANT_NONE)
	{
		action.slotIds[slot] = 0;
		action.slotFlags &= (uint16_t)~(1u << slot);
	}
	else
	{
		action.slotIds[slot] = instIdx;
		action.slotFlags |= (uint16_t)(1u << slot);
	}
}

inline void SetOutTex(DX12_RenderAction& action, size_t slot, TexID texId)
{
	action.slotIds[slot] = texId;
	action.slotFlags &= (uint16_t)~(1u << slot);
}

inline bool IsCSTexture(const DX12_RenderAction& action, size_t slot)
{
	return ((action.slotFlags >> slot) & 1) != 0;
}

inline void SetCSBinding(DX12_RenderAction& action, size_t slot, uint16_t id, bool isTexture)
{
	action.slotIds[slot] = id;
	if (isTexture)
		action.slotFlags |= (uint16_t)(1u << slot);
	else
		action.slotFlags &= (uint16_t)~(1u << slot);
}


inline static DX12_OutputSignature PackOutputSignature(const DX12_OutputSignatureDesc& desc)
{
	DX12_OutputSignature sig = {};

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		sig.RTFormats |= (UINT64)((UINT32)desc.rtFormats[r] & 0xFF) << (r * OUTSIG_RTFORMAT_BITS);
		sig.RTBlendNDSFormat |= (UINT64)((UINT32)desc.rtBlendOpts[r] & 0x7) << (r * OUTSIG_RTBLEND_BITS);
	}

	sig.RTBlendNDSFormat |= (UINT64)((UINT32)desc.dsFormat & 0xFF) << OUTSIG_DSFORMAT_SHIFT;
	sig.RTBlendNDSFormat |= (UINT64)((UINT32)desc.dsOpt & 0xFF) << OUTSIG_DSOPT_SHIFT;
	sig.RTBlendNDSFormat |= (UINT64)desc.sampleCount << OUTSIG_SAMPLE_SHIFT;
	sig.RTBlendNDSFormat |= (UINT64)((UINT32)desc.outputLayout & 0xFF) << OUTSIG_LAYOUT_SHIFT;

	return sig;
}

inline static DX12_OutputSignatureDesc UnpackOutputSignature(const DX12_OutputSignature& sig)
{
	DX12_OutputSignatureDesc desc = {};

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		desc.rtFormats[r] = (DXGI_FORMAT)((sig.RTFormats >> (r * OUTSIG_RTFORMAT_BITS)) & 0xFF);
		desc.rtBlendOpts[r] = (DataHub::Action_Output_BlendOption)((sig.RTBlendNDSFormat >> (r * OUTSIG_RTBLEND_BITS)) & 0x7);
	}

	desc.dsFormat = (DXGI_FORMAT)((sig.RTBlendNDSFormat >> OUTSIG_DSFORMAT_SHIFT) & 0xFF);
	desc.dsOpt = (DataHub::Action_Output_DepthOption)((sig.RTBlendNDSFormat >> OUTSIG_DSOPT_SHIFT) & 0xFF);
	desc.sampleCount = (uint8_t)((sig.RTBlendNDSFormat >> OUTSIG_SAMPLE_SHIFT) & 0xFF);
	desc.outputLayout = (DataHub::Action_OutputLayout)((sig.RTBlendNDSFormat >> OUTSIG_LAYOUT_SHIFT) & 0xFF);

	return desc;
}

#pragma endregion

}
