#include "D3D12Renderer.h"

#include "D3D12Core.h"

#ifdef _DEBUG
#include <comdef.h>
#endif


namespace Hakko
{

using namespace std;


#pragma region Helper

static bool RemoveIdInLUT(
	unordered_map<DataHub::HashedString, uint16_t>& LUT,
	uint16_t id)
{
	for (auto it = LUT.begin(); it != LUT.end(); ++it)
	{
		if (it->second == id)
		{
			LUT.erase(it);
			return true;
		}
	}

	return false;
}

static bool SwapAndRemoveIdInLUT(
	unordered_map<DataHub::HashedString, uint16_t>& LUT, 
	uint16_t id, uint16_t idOrigin)
{
	const bool removed = RemoveIdInLUT(LUT, id);

	bool moved = false;
	for (auto it = LUT.begin(); it != LUT.end(); ++it)
	{
		if (it->second == idOrigin)
		{
			it->second = id;
			moved = true;
			break;
		}
	}

	return removed && moved;
}


static void RemoveCTexRef(vector<vector<pair<CTexID, uint8_t>>>& refList, CTexID cTexId)
{
	for (auto& refs : refList)
	{
		for (size_t i = 0; i < refs.size(); )
		{
			if (refs[i].first == cTexId)
				refs.erase(refs.begin() + i);
			else
				i++;
		}
	}
}

static void SwapCTexRef(vector<vector<pair<CTexID, uint8_t>>>& refList, CTexID a, CTexID b)
{
	for (auto& refs : refList)
	{
		for (auto& ref : refs)
		{
			if (ref.first == a)
				ref.first = b;
			else if (ref.first == b)
				ref.first = a;
		}
	}
}


static void FillTextureSRVDesc(const DX12_Texture& texInfo, D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	desc = {};
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = GetSRVFormat(texInfo.Format);
	desc.Texture2D.MipLevels = texInfo.MipLevels;

	if (texInfo.ArraySize > 1)
	{
		if (texInfo.IsCube)
		{
			if (texInfo.ArraySize > 6)
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			else
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		}
		else
		{
			switch (texInfo.Dimension)
			{
			default:
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;	break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;	break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;			break;
			}
		}
	}
	else
	{
		switch (texInfo.Dimension)
		{
		default:
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;		break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;		break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;		break;
		}
	}

	switch (desc.ViewDimension)
	{
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
		desc.Texture2DArray.ArraySize = texInfo.ArraySize;		break;

	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
		desc.Texture1DArray.ArraySize = texInfo.ArraySize;		break;

	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
		desc.TextureCubeArray.NumCubes = texInfo.ArraySize / 6;	break;
	}
}


static uint8_t ShaderVariantVS(DataHub::VertexType vertexType)
{
	return (uint8_t)vertexType + 1;
}

static uint8_t ShaderVariantPS(DataHub::Action_OutputLayout layout)
{
	return (uint8_t)layout;
}


static const DXGI_FORMAT MESH_INDEX_FORMAT = DXGI_FORMAT_R32_UINT;
static_assert(sizeof(DataHub::VIndex) == 4, "MESH_INDEX_FORMAT 은 허브 VIndex 크기와 같이 바뀌어야 함");


static DXGI_FORMAT ToDXGI(DataHub::TextureReadFormat f)
{
    switch (f) {
    case DataHub::TextureReadFormat::R8_UNorm:             return DXGI_FORMAT_R8_UNORM;
    case DataHub::TextureReadFormat::RG8_UNorm:            return DXGI_FORMAT_R8G8_UNORM;
    case DataHub::TextureReadFormat::RGBA8_UNorm:          return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DataHub::TextureReadFormat::RGBA8_SRGB:           return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case DataHub::TextureReadFormat::BGRA8_UNorm:          return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DataHub::TextureReadFormat::BGRA8_SRGB:           return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case DataHub::TextureReadFormat::R10G10B10A2_UNorm:    return DXGI_FORMAT_R10G10B10A2_UNORM;
    case DataHub::TextureReadFormat::R11G11B10_Float:      return DXGI_FORMAT_R11G11B10_FLOAT;
    case DataHub::TextureReadFormat::R16_Float:            return DXGI_FORMAT_R16_FLOAT;
    case DataHub::TextureReadFormat::RG16_Float:           return DXGI_FORMAT_R16G16_FLOAT;
    case DataHub::TextureReadFormat::RGBA16_Float:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case DataHub::TextureReadFormat::R32_Float:            return DXGI_FORMAT_R32_FLOAT;
    case DataHub::TextureReadFormat::RG32_Float:           return DXGI_FORMAT_R32G32_FLOAT;
    case DataHub::TextureReadFormat::RGBA32_Float:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DataHub::TextureReadFormat::R16_UNorm:            return DXGI_FORMAT_R16_UNORM;
    case DataHub::TextureReadFormat::RG16_UNorm:           return DXGI_FORMAT_R16G16_UNORM;
    case DataHub::TextureReadFormat::RGBA16_UNorm:         return DXGI_FORMAT_R16G16B16A16_UNORM;
    case DataHub::TextureReadFormat::RG8_SNorm:            return DXGI_FORMAT_R8G8_SNORM;
    case DataHub::TextureReadFormat::RGBA8_SNorm:          return DXGI_FORMAT_R8G8B8A8_SNORM;
    case DataHub::TextureReadFormat::RG16_SNorm:           return DXGI_FORMAT_R16G16_SNORM;
    case DataHub::TextureReadFormat::R8_UInt:              return DXGI_FORMAT_R8_UINT;
    case DataHub::TextureReadFormat::R16_UInt:             return DXGI_FORMAT_R16_UINT;
    case DataHub::TextureReadFormat::R32_UInt:             return DXGI_FORMAT_R32_UINT;
    case DataHub::TextureReadFormat::RGB9E5_SharedExp:     return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

    case DataHub::TextureReadFormat::BC1_UNorm:            return DXGI_FORMAT_BC1_UNORM;
    case DataHub::TextureReadFormat::BC1_SRGB:             return DXGI_FORMAT_BC1_UNORM_SRGB;
    case DataHub::TextureReadFormat::BC2_UNorm:            return DXGI_FORMAT_BC2_UNORM;
    case DataHub::TextureReadFormat::BC2_SRGB:             return DXGI_FORMAT_BC2_UNORM_SRGB;
    case DataHub::TextureReadFormat::BC3_UNorm:            return DXGI_FORMAT_BC3_UNORM;
    case DataHub::TextureReadFormat::BC3_SRGB:             return DXGI_FORMAT_BC3_UNORM_SRGB;
    case DataHub::TextureReadFormat::BC4_UNorm:            return DXGI_FORMAT_BC4_UNORM;
    case DataHub::TextureReadFormat::BC4_SNorm:            return DXGI_FORMAT_BC4_SNORM;
    case DataHub::TextureReadFormat::BC5_UNorm:            return DXGI_FORMAT_BC5_UNORM;
    case DataHub::TextureReadFormat::BC5_SNorm:            return DXGI_FORMAT_BC5_SNORM;
    case DataHub::TextureReadFormat::BC6H_UF16:            return DXGI_FORMAT_BC6H_UF16;
    case DataHub::TextureReadFormat::BC6H_SF16:            return DXGI_FORMAT_BC6H_SF16;
    case DataHub::TextureReadFormat::BC7_UNorm:            return DXGI_FORMAT_BC7_UNORM;
    case DataHub::TextureReadFormat::BC7_SRGB:             return DXGI_FORMAT_BC7_UNORM_SRGB;

    case DataHub::TextureReadFormat::D16_UNorm:            return DXGI_FORMAT_D16_UNORM;
    case DataHub::TextureReadFormat::D24_UNorm_S8_UInt:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case DataHub::TextureReadFormat::D32_Float:            return DXGI_FORMAT_D32_FLOAT;
    case DataHub::TextureReadFormat::D32_Float_S8X24_UInt: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    default:                                  				return DXGI_FORMAT_UNKNOWN;
    }
}

static DXGI_FORMAT GetDSFormat(DataHub::TextureReadFormat f)
{
	switch (f)
	{
        case DataHub::TextureReadFormat::D16_UNorm:            return DXGI_FORMAT_R16_TYPELESS;
        case DataHub::TextureReadFormat::D24_UNorm_S8_UInt:    return DXGI_FORMAT_R24G8_TYPELESS;
        case DataHub::TextureReadFormat::D32_Float:            return DXGI_FORMAT_R32_TYPELESS;
        case DataHub::TextureReadFormat::D32_Float_S8X24_UInt: return DXGI_FORMAT_R32G8X24_TYPELESS;
        default:                                  				return DXGI_FORMAT_UNKNOWN;
	}
}

static bool SetLinkedMeshVertex(const DX12_Buffer& buffer, const DX12_BufferView& bufferView, DX12_Mesh& mesh, DX12_MeshView& view)
{
	using namespace DataHub;

	switch (buffer.stride)
	{
	case sizeof(VertexSimple):	mesh.vertexType = VertexType_Simple;	break;
	case sizeof(VertexStatic):	mesh.vertexType = VertexType_Static;	break;
	case sizeof(VertexSkinned):	mesh.vertexType = VertexType_Skinned;	break;
	default:					return false;
	}

	mesh.vertexCount = buffer.count;

	view.VBV.BufferLocation = bufferView.address;
	view.VBV.StrideInBytes = buffer.stride;
	view.VBV.SizeInBytes = buffer.stride * buffer.count;

	return true;
}

#pragma endregion



#pragma region ResourceControl

#pragma region InitResource

bool D3D12Renderer::InitResource()
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	#pragma region Sampler
	
	D3D12_STATIC_SAMPLER_DESC samplers[7] = {};

	for (UINT s = 0; s < _countof(samplers); s++)
	{
		samplers[s].MipLODBias = 0;
		samplers[s].MaxAnisotropy = 0;
		samplers[s].MinLOD = 0.0f;
		samplers[s].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[s].ShaderRegister = s;
		samplers[s].RegisterSpace = 0;
		samplers[s].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplers[s].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplers[s].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	samplers[0].Filter = D3D12_FILTER_ANISOTROPIC;
	samplers[0].MaxAnisotropy = 16;
	samplers[0].AddressU = samplers[0].AddressV = samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers[1].AddressU = samplers[1].AddressV = samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	samplers[2].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers[2].AddressU = samplers[2].AddressV = samplers[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	samplers[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers[3].AddressU = samplers[3].AddressV = samplers[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplers[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers[4].AddressU = samplers[4].AddressV = samplers[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplers[5].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplers[5].AddressU = samplers[5].AddressV = samplers[5].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplers[5].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplers[5].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	samplers[6].Filter = D3D12_FILTER_ANISOTROPIC;
	samplers[6].MaxAnisotropy = 16;
	samplers[6].AddressU = samplers[6].AddressV = samplers[6].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	#pragma endregion


	
	#pragma region CreateRootSig

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}


	// --- 칸 다뚫어놓는 디폴트 시그니처 (그래픽스 공용, 인덱스는 DX12_RootParam) ---
	// t space0 : 선행 출력 인스턴트 t0~t8 (RT 8 + DS), 1칸 테이블 x 9 -> 9개 하나씩 바인딩 (STex)
	// t space1 : CTex / STex (언바운드)	t space2 : 구조버퍼 (루트 SRV, 드로우 칸 tid CSBuffer)	t space4 : 레이어 전역 텍스처 (환경맵, 1칸)
	// b space0 : 레이어 고정칸 (b0 카메라, b1 조명, b2 조명행렬, b4 루트상수)
	// b space1 : 레이어 데이터		b space2 : 액션 데이터		b space3 : 물체 칸 b0~b4 (= DRAWREQ_DATABOX_MAX)
	CD3DX12_ROOT_PARAMETER1 defaultRenderRootParams[RootParam_Count];

	for (UINT k = 0; k < DRAWREQ_DATABOX_MAX; k++)
		defaultRenderRootParams[RootParam_ObjectCB0 + k].InitAsConstantBufferView(k, 3U);

	CD3DX12_DESCRIPTOR_RANGE1 loadedTextureRange;
	loadedTextureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX, 0, 1U,
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	defaultRenderRootParams[RootParam_CTexTable].InitAsDescriptorTable(1, &loadedTextureRange, D3D12_SHADER_VISIBILITY_PIXEL);

	defaultRenderRootParams[RootParam_StructuredSB].InitAsShaderResourceView(0, 2U);

	CD3DX12_DESCRIPTOR_RANGE1 instantTextureRanges[INSTANT_SLOT_MAX];
	for (UINT s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		instantTextureRanges[s].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, s, 0U,
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		defaultRenderRootParams[RootParam_InstantSR0 + s].InitAsDescriptorTable(1, &instantTextureRanges[s], D3D12_SHADER_VISIBILITY_PIXEL);
	}

	defaultRenderRootParams[RootParam_ActionCB].InitAsConstantBufferView(0, 2U);
	defaultRenderRootParams[RootParam_LayerDataCB].InitAsConstantBufferView(0, 1U);
	defaultRenderRootParams[RootParam_ConditionCB].InitAsConstantBufferView(0, 0U);
	defaultRenderRootParams[RootParam_LightCB].InitAsConstantBufferView(1, 0U);
	defaultRenderRootParams[RootParam_LightMatrixCB].InitAsConstantBufferView(2, 0U);

	CD3DX12_DESCRIPTOR_RANGE1 layerTextureRange;
	layerTextureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 4U,
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	defaultRenderRootParams[RootParam_LayerTexTable].InitAsDescriptorTable(1, &layerTextureRange, D3D12_SHADER_VISIBILITY_PIXEL);

	defaultRenderRootParams[RootParam_Constants].InitAsConstants(4, 4, 0U, D3D12_SHADER_VISIBILITY_PIXEL);


	D3D12_ROOT_SIGNATURE_FLAGS defaultRenderRootSigFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;



	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC defaultRenderRootSigDesc;
	defaultRenderRootSigDesc.Init_1_1(
		_countof(defaultRenderRootParams),
		defaultRenderRootParams,
		_countof(samplers),
		samplers,
		defaultRenderRootSigFlags
	);


	ComPtr<ID3DBlob> rootSigRaw;
	ComPtr<ID3DBlob> error;


	if (FAILED(D3DX12SerializeVersionedRootSignature(&defaultRenderRootSigDesc, featureData.HighestVersion, &rootSigRaw, &error))
		|| FAILED(device->CreateRootSignature(0, rootSigRaw->GetBufferPointer(), rootSigRaw->GetBufferSize(), IID_PPV_ARGS(&defaultRenderRootSig))))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 그래픽스 루트 시그니처 생성 실패");
		return false;
	}


	// --- 컴퓨트 루트 시그니처 (인덱스는 DX12_RootParamCompute) ---
	// t space0 : 읽기 버퍼 t0~t3 (루트 SRV)		u space0 : 쓰기 버퍼 u0~u3 (루트 UAV)
	// t space1 : 읽기 텍스처 t0~t3 (1칸 테이블)	u space1 : 쓰기 텍스처 u0~u3 (1칸 테이블)
	// t space2 : 선행 출력 인스턴트 t0~t8 (1칸 테이블 x 9)
	// b space0 : 카메라 등 b0	b space1 : 레이어 데이터 b0	b space2 : 액션 데이터 b0	(그래픽스와 같은 자리)
	CD3DX12_ROOT_PARAMETER1 computeRootParams[RootParamCS_Count];

	for (UINT k = 0; k < COMPUTE_BINDING_MAX; k++)
	{
		computeRootParams[RootParamCS_ReadBuffer0 + k].InitAsShaderResourceView(k, 0U);
		computeRootParams[RootParamCS_WriteBuffer0 + k].InitAsUnorderedAccessView(k, 0U);
	}

	CD3DX12_DESCRIPTOR_RANGE1 computeReadTexRanges[COMPUTE_BINDING_MAX];
	CD3DX12_DESCRIPTOR_RANGE1 computeWriteTexRanges[COMPUTE_BINDING_MAX];
	for (UINT k = 0; k < COMPUTE_BINDING_MAX; k++)
	{
		computeReadTexRanges[k].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, k, 1U,
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		computeRootParams[RootParamCS_ReadTex0 + k].InitAsDescriptorTable(1, &computeReadTexRanges[k]);

		computeWriteTexRanges[k].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, k, 1U,
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		computeRootParams[RootParamCS_WriteTex0 + k].InitAsDescriptorTable(1, &computeWriteTexRanges[k]);
	}

	CD3DX12_DESCRIPTOR_RANGE1 computeInstantRanges[INSTANT_SLOT_MAX];
	for (UINT s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		computeInstantRanges[s].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, s, 2U,
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		computeRootParams[RootParamCS_InstantSR0 + s].InitAsDescriptorTable(1, &computeInstantRanges[s]);
	}

	computeRootParams[RootParamCS_ActionCB].InitAsConstantBufferView(0, 2U);
	computeRootParams[RootParamCS_LayerDataCB].InitAsConstantBufferView(0, 1U);
	computeRootParams[RootParamCS_ConditionCB].InitAsConstantBufferView(0, 0U);

	D3D12_STATIC_SAMPLER_DESC computeSamplers[_countof(samplers)];
	for (UINT s = 0; s < _countof(samplers); s++)
	{
		computeSamplers[s] = samplers[s];
		computeSamplers[s].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSigDesc;
	computeRootSigDesc.Init_1_1(
		_countof(computeRootParams),
		computeRootParams,
		_countof(computeSamplers),
		computeSamplers,
		D3D12_ROOT_SIGNATURE_FLAG_NONE
	);

	rootSigRaw.Reset();
	error.Reset();

	if (FAILED(D3DX12SerializeVersionedRootSignature(&computeRootSigDesc, featureData.HighestVersion, &rootSigRaw, &error))
		|| FAILED(device->CreateRootSignature(0, rootSigRaw->GetBufferPointer(), rootSigRaw->GetBufferSize(), IID_PPV_ARGS(&computeRootSig))))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 컴퓨트 루트 시그니처 생성 실패");
		return false;
	}

	#pragma endregion



	#pragma region InitGraphicResource

	if (!InitGraphicResource())
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 기본 리소스 (쿼드 / 예약 텍스처 / 더미 버퍼 / 기본 셰이더) 준비 실패");
		return false;
	}

	PreUploadResource();

	#pragma endregion


	
	#pragma region InitCBArea

	cbArea_Light.clear();
	cbArea_Light.resize(FRAME_COUNT);
	cbArea_LightMatrix.clear();
	cbArea_LightMatrix.resize(FRAME_COUNT);

	for (size_t i = 0; i < FRAME_COUNT; i++)
	{
		cbArea_Condition[i] = std::make_unique<CBAreaAllocator<DataHub::RenderCondition>>(CB_AREA_SIZE_CONDITION);

		if (!cbArea_Light[i].Init((UINT)(RENDER_LIGHT_MAX * sizeof(DataHub::DataBox_RenderLight)), RENDER_LAYER_MAX, 0, 0))
			DebugReport(DataHub::RenderReport_Init, L"[CBArea] 조명 cbArea 생성 실패");
		if (!cbArea_LightMatrix[i].Init((UINT)(RENDER_LIGHT_MAX * sizeof(DataHub::DataBox_RenderLightMatrix)), RENDER_LAYER_MAX, 0, 0))
			DebugReport(DataHub::RenderReport_Init, L"[CBArea] 조명 행렬 cbArea 생성 실패");

		const uint32_t providerCount = DataHub::DataBoxCustomizer::GetProviderCount();

		cbArea_DataBox[i].clear();
		cbArea_DataBox[i].resize(providerCount);

		for (uint32_t tid = 0; tid < providerCount; tid++)
		{
			DataHub::IDtBProvider* provider = DataHub::DataBoxCustomizer::GetProvider(tid);
			if (provider == nullptr)
				continue;

			const bool ok = cbArea_DataBox[i][tid].Init((UINT)provider->sizeofType, CB_AREA_INIT_SIZE_DEFAULT, CB_AREA_SHRINK_FREQ, CB_AREA_SHRINK_THRESHOLD);
			if (!ok)
				DebugReport(DataHub::RenderReport_Init, L"[CBArea] 데이터박스 타입 cbArea 생성 실패 (stride 256 배수 아님 / 할당 실패)");
		}
	}

	dataSlotLUTList.resize(DataHub::DataBoxCustomizer::GetProviderCount());
	dataSlotStampList.resize(DataHub::DataBoxCustomizer::GetProviderCount());

	#pragma endregion


	return true;

}


bool D3D12Renderer::InitShaderCompiler()
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const D3D_SHADER_MODEL candidates[] = {
		D3D_SHADER_MODEL_6_6, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_4,
		D3D_SHADER_MODEL_6_3, D3D_SHADER_MODEL_6_2, D3D_SHADER_MODEL_6_1, D3D_SHADER_MODEL_6_0 };

	bool found = false;
	for (size_t i = 0; i < _countof(candidates); i++)
	{
		D3D12_FEATURE_DATA_SHADER_MODEL data = { candidates[i] };
		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &data, sizeof(data))))
		{
			shaderModel = data.HighestShaderModel;
			found = true;
			break;
		}
	}

	if (!found || shaderModel < D3D_SHADER_MODEL_6_0)
	{
		DebugReport(DataHub::RenderReport_Init, L"[Shader] 셰이더 모델 6.0 미지원 디바이스");
		return false;
	}
	if (shaderModel > D3D_SHADER_MODEL_6_6)
		shaderModel = D3D_SHADER_MODEL_6_6;


	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	if (SUCCEEDED(hr))
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	if (SUCCEEDED(hr))
		hr = dxcUtils->CreateDefaultIncludeHandler(&dxcIncludeHandler);

	if (FAILED(hr))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Shader] DXC 생성 실패 (dxcompiler.dll 확인)");
		return false;
	}

	return true;
}

bool D3D12Renderer::InitGraphicResource()
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const CD3DX12_RANGE readRange(0, 0);

	vector<ComPtr<ID3D12Resource>> uploadBufferList;
	uploadBufferList.reserve((DEFAULT_RES_MESH_COUNT - 1) * 2 + DEFAULT_RES_TEX_COUNT);


	BeginUploadCommands();

	
	#pragma region DefaultMesh

	csRefList_Mesh.assign(DEFAULT_RES_MESH_COUNT, 0);

	meshList.emplace_back();
	meshViewList.emplace_back();
	meshViewList.back().subMeshes.push_back(DX12_SubMesh{ 0, 0, 0 });


	const DataHub::VertexSimple quadVertices[4] = {
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f) },
		{ DirectX::XMFLOAT3( 1.0f,  1.0f, 0.0f) },
		{ DirectX::XMFLOAT3( 1.0f, -1.0f, 0.0f) },
	};

	const DataHub::VIndex quadIndices[6] = { 0, 1, 2, 0, 2, 3 };

	DX12_Mesh quad = {};
	quad.vertexType = DataHub::VertexType_Simple;
	quad.vertexCount = 4;
	quad.indexCount = 6;

	DX12_MeshView quadView = {};

	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(quadVertices));
	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(quadIndices));


	ComPtr<ID3D12Resource> uploadQuadVB;
	ComPtr<ID3D12Resource> uploadQuadIB;

	if (FAILED(device->CreateCommittedResource(&DEFAULT_HEAP_PROPS, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&quad.VB))))
		return false;
	if (FAILED(device->CreateCommittedResource(&UPLOAD_HEAP_PROPS, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadQuadVB))))
		return false;
	if (FAILED(device->CreateCommittedResource(&DEFAULT_HEAP_PROPS, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&quad.IB))))
		return false;
	if (FAILED(device->CreateCommittedResource(&UPLOAD_HEAP_PROPS, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadQuadIB))))
		return false;


	UINT8* pQuadData = nullptr;
	uploadQuadVB->Map(0, &readRange, (void**)&pQuadData);
	memcpy(pQuadData, quadVertices, sizeof(quadVertices));
	uploadQuadVB->Unmap(0, nullptr);

	uploadQuadIB->Map(0, &readRange, (void**)&pQuadData);
	memcpy(pQuadData, quadIndices, sizeof(quadIndices));
	uploadQuadIB->Unmap(0, nullptr);

	renderCMDList_Master->CopyBufferRegion(quad.VB.Get(), 0, uploadQuadVB.Get(), 0, sizeof(quadVertices));
	renderCMDList_Master->CopyBufferRegion(quad.IB.Get(), 0, uploadQuadIB.Get(), 0, sizeof(quadIndices));


	D3D12_RESOURCE_BARRIER quadBarriers[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(quad.VB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(quad.IB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER),
	};

	renderCMDList_Master->ResourceBarrier(2, quadBarriers);
	

	quadView.VBV.BufferLocation = quad.VB->GetGPUVirtualAddress();
	quadView.VBV.StrideInBytes = sizeof(DataHub::VertexSimple);
	quadView.VBV.SizeInBytes = sizeof(quadVertices);

	quadView.IBV.BufferLocation = quad.IB->GetGPUVirtualAddress();
	quadView.IBV.Format = MESH_INDEX_FORMAT;
	quadView.IBV.SizeInBytes = sizeof(quadIndices);

	quadView.subMeshes.push_back(DX12_SubMesh{ 0, 6, 0 });
	

	meshList.emplace_back(std::move(quad));
	meshViewList.emplace_back(std::move(quadView));

	uploadBufferList.push_back(std::move(uploadQuadVB));
	uploadBufferList.push_back(std::move(uploadQuadIB));
		
	#pragma endregion


	#pragma region DefaultTexture

	const uint32_t DefaultErrorRaw = 0xFFFF00FF;
	const uint32_t DefaultTransparentRaw = 0x00000000;
	const uint32_t None_DiffuseRaw = 0xFFFFFFFF;
	const uint32_t None_NormalRaw = 0xFFFF8080;
	const uint32_t None_ORMRaw = 0xFF0080FF;
	const uint32_t None_EmissionRaw = 0xFF000000;

	const int texWidth = sizeof(uint32_t);


	D3D12_SUBRESOURCE_DATA DefaultErrorData = {};
	DefaultErrorData.pData = &DefaultErrorRaw;
	DefaultErrorData.RowPitch = texWidth;
	DefaultErrorData.SlicePitch = DefaultErrorData.RowPitch;

	D3D12_SUBRESOURCE_DATA DefaultTransparentData = {};
	DefaultTransparentData.pData = &DefaultTransparentRaw;
	DefaultTransparentData.RowPitch = texWidth;
	DefaultTransparentData.SlicePitch = DefaultTransparentData.RowPitch;

	D3D12_SUBRESOURCE_DATA None_DiffuseData = {};
	None_DiffuseData.pData = &None_DiffuseRaw;
	None_DiffuseData.RowPitch = texWidth;
	None_DiffuseData.SlicePitch = None_DiffuseData.RowPitch;

	D3D12_SUBRESOURCE_DATA None_NormalData = {};
	None_NormalData.pData = &None_NormalRaw;
	None_NormalData.RowPitch = texWidth;
	None_NormalData.SlicePitch = None_NormalData.RowPitch;

	D3D12_SUBRESOURCE_DATA None_ORMData = {};
	None_ORMData.pData = &None_ORMRaw;
	None_ORMData.RowPitch = texWidth;
	None_ORMData.SlicePitch = None_ORMData.RowPitch;

	D3D12_SUBRESOURCE_DATA None_EmissionData = {};
	None_EmissionData.pData = &None_EmissionRaw;
	None_EmissionData.RowPitch = texWidth;
	None_EmissionData.SlicePitch = None_EmissionData.RowPitch;


	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Width = 1;
	textureDesc.Height = 1;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;



	ComPtr<ID3D12Resource> DefaultErrorTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&DefaultErrorTexture))))
		return false;
		

	const UINT64 uploadTextureSizeDefault = GetRequiredIntermediateSize(DefaultErrorTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTextureDefault;

	auto errUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSizeDefault);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&errUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTextureDefault))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), DefaultErrorTexture.Get(), uploadTextureDefault.Get(), 0, 0, 1, &DefaultErrorData);

	auto defaultResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		DefaultErrorTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	renderCMDList_Master->ResourceBarrier(1, &defaultResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(DefaultErrorTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTextureDefault));



	ComPtr<ID3D12Resource> TransparentTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&TransparentTexture))))
		return false;
		

	const UINT64 uploadTextureSizeTransparent = GetRequiredIntermediateSize(TransparentTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTextureTransparent;

	auto transparentUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSizeTransparent);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&transparentUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTextureTransparent))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), TransparentTexture.Get(), uploadTextureTransparent.Get(), 0, 0, 1, &DefaultTransparentData);

	auto transparentResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		TransparentTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	renderCMDList_Master->ResourceBarrier(1, &transparentResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(TransparentTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTextureTransparent));



	ComPtr<ID3D12Resource> None_DiffuseTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&None_DiffuseTexture))))
		return false;
		

	const UINT64 uploadTextureSize1 = GetRequiredIntermediateSize(None_DiffuseTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTexture1;

	auto diffUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSize1);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&diffUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTexture1))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), None_DiffuseTexture.Get(), uploadTexture1.Get(), 0, 0, 1, &None_DiffuseData);

	auto diffResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		None_DiffuseTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	renderCMDList_Master->ResourceBarrier(1, &diffResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(None_DiffuseTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTexture1));



	ComPtr<ID3D12Resource> None_NormalTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&None_NormalTexture))))
		return false;
		

	const UINT64 uploadTextureSize2 = GetRequiredIntermediateSize(None_NormalTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTexture2;

	auto normUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSize2);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&normUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTexture2))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), None_NormalTexture.Get(), uploadTexture2.Get(), 0, 0, 1, &None_NormalData);

	auto normResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		None_NormalTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	renderCMDList_Master->ResourceBarrier(1, &normResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(None_NormalTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTexture2));


	
	ComPtr<ID3D12Resource> None_ORMTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&None_ORMTexture))))
		return false;
		

	const UINT64 uploadTextureSize3 = GetRequiredIntermediateSize(None_ORMTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTexture3;

	auto ormUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSize3);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&ormUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTexture3))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), None_ORMTexture.Get(), uploadTexture3.Get(), 0, 0, 1, &None_ORMData);

	auto ormResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		None_ORMTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		;

	renderCMDList_Master->ResourceBarrier(1, &ormResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(None_ORMTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTexture3));


	
	ComPtr<ID3D12Resource> None_EmissionTexture;

	if (FAILED(device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&None_EmissionTexture))))
		return false;
		

	const UINT64 uploadTextureSize4 = GetRequiredIntermediateSize(None_EmissionTexture.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadTexture4;

	auto emsUploadResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSize4);

	if (FAILED(device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&emsUploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadTexture4))))
		return false;


	UpdateSubresources(renderCMDList_Master.Get(), None_EmissionTexture.Get(), uploadTexture4.Get(), 0, 0, 1, &None_EmissionData);

	auto emsResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		None_EmissionTexture.Get(), 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		;

	renderCMDList_Master->ResourceBarrier(1, &emsResBar);

	textureList.emplace_back(
		DX12_Texture{
			std::move(None_EmissionTexture),
			1,
			1,
			textureDesc.Format,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            false
			
		}
	);

	uploadBufferList.push_back(std::move(uploadTexture4));


	texRefList_CTex.assign(DEFAULT_RES_TEX_COUNT, {});
	texRefMask_STex.assign(DEFAULT_RES_TEX_COUNT, false);
	singleTextureList.assign(DEFAULT_RES_TEX_COUNT, {});
	frameMarkList_STex.assign(DEFAULT_RES_TEX_COUNT, 0);
	texRefList_WTex.assign(DEFAULT_RES_TEX_COUNT, {});

	#pragma endregion


	#pragma region DefaultCSBuffer

	{
		DataHub::ComputeBuffer dummyDesc;
		dummyDesc.stride = CSBUFFER_DUMMY_STRIDE;
		dummyDesc.count = CSBUFFER_DUMMY_COUNT;

		DX12_Buffer dummy;
		DX12_BufferView dummyView;
		if (!CreateBufferResource(dummyDesc, dummy, dummyView, uploadBufferList))
			return false;

		csBufferList.emplace_back(std::move(dummy));
		csBufferViewList.push_back(dummyView);
	}

	#pragma endregion


	FlushUploadCommands();

	uploadBufferList.clear();


	#pragma region DefaultShader

	shaderList.clear();
	shaderList.resize(DEFAULT_RES_SHADER_COUNT);
	shaderList[DEFAULT_SHADER_COMPOSITE_PS] = std::make_pair(defaultShaderDir + L"/defaultComposite_PS.hlsl", DataHub::ShaderType_PS);
	shaderList[DEFAULT_SHADER_SKYBOX_VS] = std::make_pair(defaultShaderDir + L"/defaultSkyBox_VS.hlsl", DataHub::ShaderType_VS);
	shaderList[DEFAULT_SHADER_SKYBOX_PS] = std::make_pair(defaultShaderDir + L"/defaultSkyBox_PS.hlsl", DataHub::ShaderType_PS);
	shaderList[DEFAULT_SHADER_BG_VS] = std::make_pair(defaultShaderDir + L"/defaultBG_VS.hlsl", DataHub::ShaderType_VS);
	shaderList[DEFAULT_SHADER_BG_PS] = std::make_pair(defaultShaderDir + L"/defaultBG_PS.hlsl", DataHub::ShaderType_PS);
	shaderList[DEFAULT_SHADER_WORLD_VS] = std::make_pair(defaultShaderDir + L"/defaultWorld_VS.hlsl", DataHub::ShaderType_VS);
	shaderList[DEFAULT_SHADER_WORLD_OPQ_PS] = std::make_pair(defaultShaderDir + L"/defaultWorldOPQ_PS.hlsl", DataHub::ShaderType_PS);
	shaderList[DEFAULT_SHADER_WORLD_TP_PS] = std::make_pair(defaultShaderDir + L"/defaultWorldTP_PS.hlsl", DataHub::ShaderType_PS);


	ComPtr<IDxcBlob> defaultBG_VSRaw;
	ComPtr<IDxcBlob> defaultComposite_PSRaw;

	if (!CompileShader(DEFAULT_SHADER_BG_VS, SHADER_VARIANT_NONE, defaultBG_VSRaw))
		return false;
	if (!CompileShader(DEFAULT_SHADER_COMPOSITE_PS, SHADER_VARIANT_NONE, defaultComposite_PSRaw))
		return false;


	if (DataHub::drawPropIdLUT.size() < DEFAULT_RES_PSOBODY_COUNT)
		DebugReport(DataHub::RenderReport_Init, L"[Init] 허브 빌트인 drawProp 0~6 이 없음 (DataHub::Init 순서 확인)");

	{
		using namespace DataHub;

		psoGeneratorList.resize(DEFAULT_RES_PSOBODY_COUNT);

		DrawProp& sky = psoGeneratorList[1].psoBody;
		sky.depthUsage = DrawProp_DepthUsage_Background;	sky.cullType = DrawProp_CullType_None;	sky.blendType = DrawProp_BlendType_None;
		sky.vertexType = VertexType_Simple;					sky.VSId = DEFAULT_SHADER_SKYBOX_VS;	sky.PSId = DEFAULT_SHADER_SKYBOX_PS;

		DrawProp& bg = psoGeneratorList[2].psoBody;
		bg.depthUsage = DrawProp_DepthUsage_Background;		bg.cullType = DrawProp_CullType_None;	bg.blendType = DrawProp_BlendType_None;
		bg.vertexType = VertexType_Simple;					bg.VSId = DEFAULT_SHADER_BG_VS;	bg.PSId = DEFAULT_SHADER_BG_PS;

		DrawProp& opq = psoGeneratorList[3].psoBody;
		opq.depthUsage = DrawProp_DepthUsage_Opaque;		opq.cullType = DrawProp_CullType_Back;	opq.blendType = DrawProp_BlendType_None;
		opq.vertexType = VertexType_Static;					opq.VSId = DEFAULT_SHADER_WORLD_VS;	opq.PSId = DEFAULT_SHADER_WORLD_OPQ_PS;

		DrawProp& tp = psoGeneratorList[4].psoBody;
		tp.depthUsage = DrawProp_DepthUsage_Translucent;	tp.cullType = DrawProp_CullType_Back;	tp.blendType = DrawProp_BlendType_Alpha;
		tp.vertexType = VertexType_Static;					tp.VSId = DEFAULT_SHADER_WORLD_VS;	tp.PSId = DEFAULT_SHADER_WORLD_TP_PS;

		DrawProp& opqSkin = psoGeneratorList[5].psoBody;
		opqSkin = opq;	opqSkin.vertexType = VertexType_Skinned;

		DrawProp& tpSkin = psoGeneratorList[6].psoBody;
		tpSkin = tp;	tpSkin.vertexType = VertexType_Skinned;
	}

	#pragma endregion


	#pragma region DefaultPSO

	D3D12_GRAPHICS_PIPELINE_STATE_DESC compDesc = {};
	compDesc.pRootSignature = defaultRenderRootSig.Get();
	compDesc.VS = CD3DX12_SHADER_BYTECODE(defaultBG_VSRaw->GetBufferPointer(), defaultBG_VSRaw->GetBufferSize());
	compDesc.PS = CD3DX12_SHADER_BYTECODE(defaultComposite_PSRaw->GetBufferPointer(), defaultComposite_PSRaw->GetBufferSize());
	compDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	compDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	compDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	compDesc.DepthStencilState.DepthEnable = FALSE;
	compDesc.DepthStencilState.StencilEnable = FALSE;
	compDesc.SampleMask = UINT_MAX;
	compDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	compDesc.NumRenderTargets = 1;
	compDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	compDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	compDesc.SampleDesc.Count = 1;

	HRESULT hr = device->CreateGraphicsPipelineState(&compDesc, IID_PPV_ARGS(&compositePSO));
	if (FAILED(hr))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 레이어 합성 PSO 생성 실패");
		return false;
	}

	#pragma endregion


	#pragma region DefaultCombinedTexture

	ID3D12Resource* subresList[CTEX_COMBINED_SIZE] = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC* srvDescList[CTEX_COMBINED_SIZE] = {};


	for (size_t i = 0; i < CTEX_COMBINED_SIZE; i++)
	{
		subresList[i] = textureList[0].Tex.Get();
	}
	

	UINT rvStartIndex;
	if (!rvHeap->SetCombinedSRV(subresList, srvDescList, CTEX_COMBINED_SIZE, rvStartIndex))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 기본 CTex SRV 생성 실패");
		return false;
	}


	const CTexID id = combinedTextureList.size();

	DX12_CombinedTextureView CTex = { rvStartIndex };
	combinedTextureList.emplace_back(CTex);
	combinedTextureBodyList.emplace_back();

	#pragma endregion


	return true;

}

void D3D12Renderer::PreUploadResource()
{
	using namespace DataHub;

	vector<ComPtr<ID3D12Resource>> uploadBufferList;

	BeginUploadCommands();

	UploadMesh(uploadBufferList);
	UploadTexture(uploadBufferList);
	UploadCSBuffer(uploadBufferList);
	LinkCSBufferMesh(uploadBufferList);
	GetShaderInfo();

	BuildPSOBody();
	BuildCombinedTexture();

	FlushUploadCommands();


	flag_updateResources &= ~(uint32_t)RTBCMD_Upload;
}

#pragma endregion


#pragma region SketchCall

void D3D12Renderer::InvalidateView()
{
	using namespace DataHub;
	
	for (auto id : ReleaseTextureList)
	{
		if (id >= textureList.size())
		{
			DebugReport(RenderReport_Contract, L"[Release] texId 범위 밖 (업로드 전 해제?), 무시");
			continue;
		}
		if (id < DEFAULT_RES_TEX_COUNT)
		{
			DebugReport(RenderReport_Contract, L"[Release] 예약 texId 해제 요청, 무시");
			continue;
		}

		auto& targetRefs_CTex = texRefList_CTex[id];
		for (auto [cTexId, slot] : targetRefs_CTex)
		{
			combinedTextureBodyList[cTexId].texId[slot] = 0;

			rvHeap->UpdateSRV(
				combinedTextureList[cTexId].RVHeapIndex + slot,
				textureList[0].Tex.Get(),
				nullptr
			);
		}

		if (texRefMask_STex[id])
		{
			rvHeap->UpdateSRV(
				singleTextureList[id].RVHeapIndex,
				textureList[0].Tex.Get(),
				nullptr
			);
			frameMarkList_STex[id] = 0;
		}
	}
}

void D3D12Renderer::CollectManagedResource()
{
	for (size_t i = 0; i < texRefMask_STex.size(); i++)
	{
		if (!texRefMask_STex[i])
			continue;
		
		if ((currentFrameCount % UINT16_MAX) - frameMarkList_STex[i] > STEX_COLLECT_THRESHOLD)
		{
			rvHeap->UnSetSRV(singleTextureList[i].RVHeapIndex);
			singleTextureList[i].RVHeapIndex = 0;
			texRefMask_STex[i] = false;
		}
	}
}


void D3D12Renderer::ScheduledReleaseResource()
{
	#ifdef _DEBUG
	{
		using namespace DataHub;

		const bool releasing = meshReleaseCount == 0 || texReleaseCount == 0 || csBufferReleaseCount == 0
			|| psoReleaseCount == 0 || cTexReleaseCount == 0;
		const bool pending = !uploadMeshList.empty() || !uploadTextureList.empty() || !uploadCSBufferList.empty()
			|| !linkCSBufferAsMeshList.empty() || !uploadDrawPropList.empty() || !uploadTexPropList.empty()
			|| !updateMeshList.empty() || !updateTextureList.empty() || !updateCSBufferList.empty()
			|| !updateDrawPropList.empty() || !updateTexPropList.empty();

		if (releasing && pending)
			DebugReport(RenderReport_Contract, L"[Release] 해제 실행 시점에 허브 대기 리스트가 남아 있음 (Create / Update / Link 는 부른 프레임에 RTB 를 보내야 함)");
	}
	#endif

	if (meshReleaseCount > -1)
	{
		if (meshReleaseCount == 0)
			ReleaseMesh(releaseMeshIdMask);
		meshReleaseCount--;
	}
	if (texReleaseCount > -1)
	{
		if (texReleaseCount == 0)
			ReleaseTexture(releaseTexIdMask);
		texReleaseCount--;
	}
	if (csBufferReleaseCount > -1)
	{
		if (csBufferReleaseCount == 0)
			ReleaseCSBuffer(releaseCSBufferIdMask);
		csBufferReleaseCount--;
	}
	if (psoReleaseCount > -1)
	{
		if (psoReleaseCount == 0)
			ReleasePSOBody(releasePSOIdMask);
		psoReleaseCount--;
	}
	if (cTexReleaseCount > -1)
	{
		if (cTexReleaseCount == 0)
			ReleaseCombinedTexture(releaseCTexIdMask);
		cTexReleaseCount--;
	}
}


void D3D12Renderer::ReleaseRetiredResource()
{
	for (auto& view : retiredSTexViewLists[cpuFrameIndex])
		rvHeap->UnSetSRV(view.RVHeapIndex);

	for (auto& view : retiredCTexViewLists[cpuFrameIndex])
		rvHeap->UnSetSRV(view.RVHeapIndex, CTEX_COMBINED_SIZE);

	retiredSTexViewLists[cpuFrameIndex].clear();
	retiredCTexViewLists[cpuFrameIndex].clear();
	retiredMeshLists[cpuFrameIndex].clear();
	retiredTextureLists[cpuFrameIndex].clear();
	retiredCSBufferLists[cpuFrameIndex].clear();
	retiredPSOLists[cpuFrameIndex].clear();
}

#pragma endregion


#pragma region UploadResource

void D3D12Renderer::UploadMesh(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	const size_t newMeshListSize = meshList.size() + uploadMeshList.size();
	meshList.reserve(newMeshListSize);
	meshViewList.reserve(newMeshListSize);
	releaseMeshIdMask.resize(newMeshListSize, false);
	csRefList_Mesh.resize(newMeshListSize, 0);


	if (uploadMeshList.empty())
		return;


	for (auto& upload : uploadMeshList)
	{
		DX12_Mesh mesh;
		DX12_MeshView view;

		if (!CreateMeshResource(upload, mesh, view, uploadBufferList))
		{
			bool linked = false;
			for (const auto& link : linkCSBufferAsMeshList)
				linked = linked || link.linkMeshId == meshList.size();

			if (!linked)
				DebugReport(DataHub::RenderReport_Contract, L"[Mesh] 업로드 항목 무효 (빈 데이터 / 생성 실패) -> 빈 메쉬로 자리만");

			meshList.emplace_back();
			meshViewList.emplace_back();
			meshViewList.back().subMeshes.push_back(DX12_SubMesh{ 0, 0, 0 });
			continue;
		}

		meshList.emplace_back(std::move(mesh));
		meshViewList.emplace_back(std::move(view));
	}


	uploadMeshList.clear();
}

void D3D12Renderer::UploadTexture(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	const size_t newTextureListSize = textureList.size() + uploadTextureList.size();
	textureList.reserve(newTextureListSize);
	texRefMask_STex.resize(newTextureListSize, false);
	singleTextureList.resize(newTextureListSize, {});
	frameMarkList_STex.resize(newTextureListSize, 0);
	texRefList_WTex.resize(newTextureListSize, {});

	releaseTexIdMask.resize(newTextureListSize, false);

	if (uploadTextureList.empty())
		return;


	for (auto& upload : uploadTextureList)
	{
		DX12_Texture texture;

		if (!CreateTextureResource(upload, texture, uploadBufferList))
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Tex] 텍스처 업로드 거부 (에러 텍스처로 자리만)");
			textureList.emplace_back(textureList[0]);
			continue;
		}

		textureList.emplace_back(std::move(texture));
		BuildWritableTexture(textureList.back(), upload, texRefList_WTex[textureList.size() - 1]);
}


	texRefList_CTex.resize(textureList.size(), {});


	uploadTextureList.clear();

}

void D3D12Renderer::UploadCSBuffer(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	if (uploadCSBufferList.empty())
		return;

	const size_t newListSize = csBufferList.size() + uploadCSBufferList.size();
	csBufferList.reserve(newListSize);
	csBufferViewList.reserve(newListSize);
	releaseCSBufferIdMask.resize(newListSize, false);


	for (auto& upload : uploadCSBufferList)
	{
		DX12_Buffer buffer;
		DX12_BufferView view;

		if (!CreateBufferResource(upload, buffer, view, uploadBufferList))
		{
			DebugReport(RenderReport_Contract, L"[CSBuffer] 업로드 항목 무효 (stride / count / data 크기) -> 더미로 자리만");
			csBufferList.push_back(csBufferList[0]);
			csBufferViewList.push_back(csBufferViewList[0]);
			continue;
		}

		csBufferList.emplace_back(std::move(buffer));
		csBufferViewList.push_back(view);
	}


	uploadCSBufferList.clear();
}

void D3D12Renderer::LinkCSBufferMesh(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	if (linkCSBufferAsMeshList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	for (auto& link : linkCSBufferAsMeshList)
	{
		const MeshID meshId = link.linkMeshId;
		const CSBufferID csBufferId = link.csBufferId;

		if (meshId >= meshList.size() || csBufferId == 0 || csBufferId >= csBufferList.size())
		{
			DebugReport(RenderReport_Contract, L"[Link] meshId / csBufferId 범위 밖 (업로드 전 링크?), 빈 메쉬 유지");
			continue;
		}

		DX12_Mesh mesh;
		DX12_MeshView view;

		if (!SetLinkedMeshVertex(csBufferList[csBufferId], csBufferViewList[csBufferId], mesh, view))
		{
			DebugReport(RenderReport_Contract, L"[Link] 컴퓨트 버퍼 stride 가 정점 타입 (Simple / Static / Skinned) 과 안 맞음, 빈 메쉬 유지");
			continue;
		}

		if (!CreateIndexResource(link.indices, link.subMeshes, csBufferList[csBufferId].count, mesh, view, uploadBufferList))
		{
			DebugReport(RenderReport_Contract, L"[Link] 인덱스 비어 있음 / 생성 실패, 빈 메쉬 유지");
			continue;
		}

		retiredMeshLists[retireSlot].push_back(std::move(meshList[meshId]));
		meshList[meshId] = std::move(mesh);
		meshViewList[meshId] = std::move(view);
		meshSubMeshCountList[meshId] = (uint16_t)meshViewList[meshId].subMeshes.size();
		csRefList_Mesh[meshId] = csBufferId;
	}


	linkCSBufferAsMeshList.clear();
}


void D3D12Renderer::GetShaderInfo()
{
	using namespace DataHub;

	shaderList.insert(
		shaderList.end(),
		std::make_move_iterator(loadShaderList.begin()),
		std::make_move_iterator(loadShaderList.end())
	);

	loadShaderList = vector<ShaderInfo>();
}


void D3D12Renderer::BuildPSOBody()
{
	using namespace DataHub;

	const size_t newListSize = psoGeneratorList.size() + uploadDrawPropList.size();

	psoGeneratorList.reserve(newListSize);

	releasePSOIdMask.resize(newListSize, false);


	const ShaderID shaderCount = (ShaderID)shaderList.size();
	bool rejected = false;

	for (const DrawProp& prop : uploadDrawPropList)
	{
		DX12_PSOGenerator generator = {};
		generator.psoBody = prop;

		if (prop.VSId == 0 || prop.PSId == 0 || prop.VSId >= shaderCount || prop.PSId >= shaderCount)
		{
			generator.psoBody.VSId = 0;
			generator.psoBody.PSId = 0;
			rejected = true;
		}

		psoGeneratorList.emplace_back(std::move(generator));
	}

	if (rejected)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[PSO] 셰이더 id 잘못된 drawProp 있음 (빈 drawProp 으로 자리만)");

		renderFrames[cpuFrameIndex].frameInvalidated = true;
	}

	uploadDrawPropList.clear();
}


void D3D12Renderer::BuildPSOVariant(PSOID psoId, OutputSignatureID sigId)
{
	using namespace DataHub;

	if (sigId == OUTPUT_SIGNATURE_INVALID || sigId >= outputSigCount)
		return;

	
	if (psoId >= psoGeneratorList.size())
	{
		DebugReport(DataHub::RenderReport_Invariant, L"[PSO] psoId 범위 밖");
		return;
	}

	DX12_PSOGenerator& generator = psoGeneratorList[psoId];

	if (generator.psoVariants[sigId] != nullptr)
		return;


	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const PSOBody& body = generator.psoBody;
	const DX12_OutputSignatureDesc sig = UnpackOutputSignature(outputSigIds[sigId]);
	const array<bool, RENDER_TARGET_MAX>& blendable = outputSigBlendableMasks[sigId];
	const Action_OutputLayout layout = sig.outputLayout;


	#pragma region PSOBases

	const D3D12_INPUT_ELEMENT_DESC MeshInputElements[] =
	{
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexSimple, Pos),         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexStatic, UV),          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexStatic, Normal),      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexStatic, Tangent),     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexStatic, Color),       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT,  0, (UINT)offsetof(VertexSkinned, BoneIdx),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexSkinned, BoneWeight), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

    const D3D12_RASTERIZER_DESC DefaultRasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    const D3D12_BLEND_DESC DefaultBlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	const D3D12_RENDER_TARGET_BLEND_DESC AlphaBlendDesc =
	{
		1,
		0,
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};
	const D3D12_RENDER_TARGET_BLEND_DESC AdditiveBlendDesc =
	{
		1,
		0,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ONE,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ONE,
		D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};
	const D3D12_RENDER_TARGET_BLEND_DESC MultiplyBlendDesc =
	{
		1,
		0,
		D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_DEST_ALPHA,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

    const D3D12_DEPTH_STENCIL_DESC DefaultDepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	const D3D12_INPUT_LAYOUT_DESC SimpleMeshInputLayout  = { MeshInputElements, 1 };
	const D3D12_INPUT_LAYOUT_DESC StaticMeshInputLayout  = { MeshInputElements, 5 };
	const D3D12_INPUT_LAYOUT_DESC SkinnedMeshInputLayout = { MeshInputElements, 7 };

	#pragma endregion


	#pragma region Shader

	const ShaderID shaderCount = (ShaderID)shaderList.size();
	const bool needPS = (layout != OutputLayout_DepthOnly);

	if (body.VSId == 0 || body.VSId >= shaderCount
		|| (needPS && (body.PSId == 0 || body.PSId >= shaderCount)))
	{
		DebugReport(DataHub::RenderReport_Contract, L"[PSO] 셰이더 id 잘못됨 (변형 생성 안함)");
		return;
	}

	IDxcBlob* VSRaw = GetCompiledShader(body.VSId, ShaderVariantVS(body.vertexType));
	if (VSRaw == nullptr)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[PSO] VS 컴파일 실패 (변형 생성 안함)");
		return;
	}

	IDxcBlob* PSRaw = nullptr;
	if (needPS)
	{
		PSRaw = GetCompiledShader(body.PSId, ShaderVariantPS(layout));
		if (PSRaw == nullptr)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[PSO] PS 컴파일 / 레이아웃 검증 실패 (변형 생성 안함)");
			return;
		}
	}

	#pragma endregion


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	psoDesc.pRootSignature = defaultRenderRootSig.Get();
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc.Count = sig.sampleCount == 0 ? 1 : sig.sampleCount;
	psoDesc.SampleDesc.Quality = 0;

	psoDesc.RasterizerState = DefaultRasterizerState;
	psoDesc.BlendState = DefaultBlendState;
	psoDesc.DepthStencilState = DefaultDepthStencilState;


	psoDesc.VS = CD3DX12_SHADER_BYTECODE(VSRaw->GetBufferPointer(), VSRaw->GetBufferSize());
	if (PSRaw != nullptr)
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(PSRaw->GetBufferPointer(), PSRaw->GetBufferSize());
		

	switch (body.topology)
	{
	default:
	case DrawProp_Topology_TriangleList:
	case DrawProp_Topology_TriangleStrip:
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		break;
	case DrawProp_Topology_LineList:
	case DrawProp_Topology_LineStrip:
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		break;
	case DrawProp_Topology_PointList:
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		break;
	}

	#pragma region Signature

	UINT numRenderTargets = 0;

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		psoDesc.RTVFormats[r] = sig.rtFormats[r];

		if (sig.rtFormats[r] != DXGI_FORMAT_UNKNOWN)
			numRenderTargets = r + 1;
	}

	psoDesc.NumRenderTargets = numRenderTargets;
	psoDesc.DSVFormat = sig.dsFormat;


	psoDesc.BlendState.IndependentBlendEnable = numRenderTargets > 1;

	for (uint8_t r = 0; r < numRenderTargets; r++)
	{
		if (sig.rtFormats[r] == DXGI_FORMAT_UNKNOWN)
			continue;

		DrawProp_BlendType blend;

		switch (sig.rtBlendOpts[r])
		{
		default:
		case Action_Output_BlendOption_ByBlendType:
			blend = body.blendType;
			break;

		case Action_Output_BlendOption_ForceNone:
			blend = DrawProp_BlendType_None;
			break;

		case Action_Output_BlendOption_ForceAlpha:
			blend = DrawProp_BlendType_Alpha;
			break;

		case Action_Output_BlendOption_ForceAddtive:
			blend = DrawProp_BlendType_Addtive;
			break;

		case Action_Output_BlendOption_ForceMultiply:
			blend = DrawProp_BlendType_Multiply;
			break;
		}

		if (blend != DrawProp_BlendType_None && !blendable[r])
		{
			DebugReport(DataHub::RenderReport_Contract, L"[PSO] 블렌드 안되는 포맷 칸에 블렌드 요구 (None 으로 대체)");
			blend = DrawProp_BlendType_None;
		}

		switch (blend)
		{
		default:
		case DrawProp_BlendType_None:
			psoDesc.BlendState.RenderTarget[r] = DefaultBlendState.RenderTarget[0];
			break;

		case DrawProp_BlendType_Alpha:
			psoDesc.BlendState.RenderTarget[r] = AlphaBlendDesc;
			break;

		case DrawProp_BlendType_Addtive:
			psoDesc.BlendState.RenderTarget[r] = AdditiveBlendDesc;
			break;

		case DrawProp_BlendType_Multiply:
			psoDesc.BlendState.RenderTarget[r] = MultiplyBlendDesc;
			break;
		}
	}


	if (sig.dsFormat == DXGI_FORMAT_UNKNOWN)
	{
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
	}
	else
	{
		switch (body.depthUsage)
		{
		default:
		case DrawProp_DepthUsage_Opaque:
			break;

		case DrawProp_DepthUsage_Translucent:
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			break;

		case DrawProp_DepthUsage_Screen:
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			break;

		case DrawProp_DepthUsage_Background:
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			break;

		case DrawProp_DepthUsage_Equal:
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			break;

		case DrawProp_DepthUsage_Always:
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			break;
		}

		if (sig.dsOpt == Action_Output_DepthOption_ForceDepthRead)
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}

	#pragma endregion


	#pragma region Body

	switch (body.cullType)
	{
	default:
	case DrawProp_CullType_Back:
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		break;

	case DrawProp_CullType_Front:
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
		break;

	case DrawProp_CullType_None:
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		break;
	}

	switch (body.fillType)
	{
	default:
	case DrawProp_FillType_Solid:
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		break;

	case DrawProp_FillType_Wireframe:
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		break;
	}

	switch (body.depthBias)
	{
	default:
	case DrawProp_DepthBias_None:
		break;

	case DrawProp_DepthBias_ShadowCaster:
		psoDesc.RasterizerState.DepthBias = DEPTH_BIAS_SHADOWCASTER;
		psoDesc.RasterizerState.SlopeScaledDepthBias = DEPTH_BIAS_SLOPE_SHADOWCASTER;
		psoDesc.RasterizerState.DepthBiasClamp = DEPTH_BIAS_CLAMP_SHADOWCASTER;
		break;

	case DrawProp_DepthBias_Decal:
		psoDesc.RasterizerState.DepthBias = DEPTH_BIAS_DECAL;
		psoDesc.RasterizerState.SlopeScaledDepthBias = DEPTH_BIAS_SLOPE_DECAL;
		psoDesc.RasterizerState.DepthBiasClamp = DEPTH_BIAS_CLAMP_DECAL;
		break;
	}

	if (body.stecilOpt.useStencil)
		DebugReport(DataHub::RenderReport_Contract, L"[PSO] 스텐실 옵션 미구현 (무시)");

	switch (body.vertexType)
	{
	default:
	case VertexType_Static:
		psoDesc.InputLayout = StaticMeshInputLayout;
		break;

	case VertexType_Skinned:
		psoDesc.InputLayout = SkinnedMeshInputLayout;
		break;

	case VertexType_Simple:
		psoDesc.InputLayout = SimpleMeshInputLayout;
		break;
	}

	#pragma endregion


	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&generator.psoVariants[sigId]));

	if (FAILED(hr))
	{
		generator.psoVariants[sigId].Reset();
		DebugReport(DataHub::RenderReport_Contract, L"[PSO] 변형 생성 실패 (해당 드로우 스킵됨)");
	}
}

void D3D12Renderer::BuildCombinedTexture()
{
	using namespace DataHub;

	const size_t newListSize = combinedTextureList.size() + uploadTexPropList.size();

	combinedTextureList.reserve(newListSize);
	combinedTextureBodyList.reserve(newListSize);

	releaseCTexIdMask.resize(newListSize, false);


	const DX12_CombinedTextureView defaultView = combinedTextureList[0];
	bool heapFull = false;

	for (const TextureProp& prop : uploadTexPropList)
	{
		const CTexID id = (CTexID)combinedTextureList.size();

		UINT rvStartIndex;
		if (!BuildCombinedView(prop, rvStartIndex))
		{
			combinedTextureList.emplace_back(defaultView);
			combinedTextureBodyList.emplace_back();
			heapFull = true;
			continue;
		}

		combinedTextureList.emplace_back(DX12_CombinedTextureView{ rvStartIndex });
		combinedTextureBodyList.emplace_back(prop);

		for (size_t i = 0; i < CTEX_COMBINED_SIZE; i++)
		{
			if (prop.texId[i] >= DEFAULT_RES_TEX_COUNT && prop.texId[i] < texRefList_CTex.size())
				texRefList_CTex[prop.texId[i]].emplace_back(id, (uint8_t)i);
		}
	}

	if (heapFull)
		DebugReport(DataHub::RenderReport_Limit, L"[CTex] 디스크립터 힙 꽉참 (기본 texProp 뷰로 자리만)");

	uploadTexPropList.clear();
}

#pragma endregion


#pragma region UpdateResources

void D3D12Renderer::UpdateMesh(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	if (updateMeshList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	for (auto& update : updateMeshList)
	{
		const MeshID id = update.first;

		if (id >= meshList.size())
		{
			DebugReport(RenderReport_Contract, L"[Mesh] 업데이트 meshId 범위 밖 (업로드 전 업데이트?), 무시");
			continue;
		}

		if (csRefList_Mesh[id] != 0)
		{
			DebugReport(RenderReport_Contract, L"[Mesh] 링크 메쉬 (컴퓨트 버퍼 별칭) 는 업데이트 불가, 무시");
			continue;
		}

		DX12_Mesh mesh;
		DX12_MeshView view;

		if (!CreateMeshResource(update.second, mesh, view, uploadBufferList))
		{
			DebugReport(RenderReport_Contract, L"[Mesh] 업데이트 항목 무효 (빈 데이터 / 생성 실패), 기존 유지");
			continue;
		}

		retiredMeshLists[retireSlot].push_back(std::move(meshList[id]));
		meshList[id] = std::move(mesh);
		meshViewList[id] = std::move(view);
		meshSubMeshCountList[id] = (uint16_t)meshViewList[id].subMeshes.size();
	}


	updateMeshList.clear();
}

void D3D12Renderer::UpdateTexture(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	if (updateTextureList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	for (auto& update : updateTextureList)
	{
		const TexID id = update.first;

		if (id >= textureList.size())
		{
			DebugReport(RenderReport_Contract, L"[Tex] 업데이트 texId 범위 밖 (업로드 전 업데이트?), 무시");
			continue;
		}

		DX12_Texture texture;

		if (!CreateTextureResource(update.second, texture, uploadBufferList))
		{
			DebugReport(RenderReport_Contract, L"[Tex] 텍스처 업데이트 거부, 기존 유지");
			continue;
		}

		retiredTextureLists[retireSlot].push_back(std::move(textureList[id]));
		textureList[id] = std::move(texture);
		BuildWritableTexture(textureList[id], update.second, texRefList_WTex[id]);

		if (texRefMask_STex[id])
		{
			retiredSTexViewLists[retireSlot].push_back(singleTextureList[id]);
			singleTextureList[id] = {};
			texRefMask_STex[id] = false;
		}

		for (auto& ref : texRefList_CTex[id])
		{
			const CTexID cTexId = ref.first;
			UINT rvStartIndex;

			if (!BuildCombinedView(combinedTextureBodyList[cTexId], rvStartIndex))
			{
				DebugReport(RenderReport_Limit, L"[CTex] 디스크립터 힙 꽉참 (업데이트된 텍스처의 조합 뷰 유지)");
				continue;
			}

			retiredCTexViewLists[retireSlot].push_back(combinedTextureList[cTexId]);
			combinedTextureList[cTexId] = { rvStartIndex };
		}
	}


	updateTextureList.clear();
}

void D3D12Renderer::UpdateCSBuffer(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	if (updateCSBufferList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	for (auto& update : updateCSBufferList)
	{
		const CSBufferID id = update.first;

		if (id >= csBufferList.size())
		{
			DebugReport(RenderReport_Contract, L"[CSBuffer] 업데이트 csBufferId 범위 밖 (업로드 전 업데이트?), 무시");
			continue;
		}

		DX12_Buffer buffer;
		DX12_BufferView view;

		if (!CreateBufferResource(update.second, buffer, view, uploadBufferList))
		{
			DebugReport(RenderReport_Contract, L"[CSBuffer] 업데이트 항목 무효 (stride / count / data 크기), 기존 유지");
			continue;
		}

		retiredCSBufferLists[retireSlot].push_back(std::move(csBufferList[id]));
		csBufferList[id] = std::move(buffer);
		csBufferViewList[id] = view;

		for (size_t m = 0; m < csRefList_Mesh.size(); m++)
		{
			if (csRefList_Mesh[m] != id)
				continue;

			if (!SetLinkedMeshVertex(csBufferList[id], view, meshList[m], meshViewList[m]))
			{
				DebugReport(RenderReport_Contract, L"[Link] 업데이트된 컴퓨트 버퍼 stride 가 정점 타입과 안 맞음 (링크 메쉬 -> 빈 메쉬)");
				UnlinkMesh((MeshID)m, retireSlot);
			}
		}
	}


	updateCSBufferList.clear();
}


void D3D12Renderer::UpdatePSOBody()
{
	using namespace DataHub;

	if (updateDrawPropList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	const ShaderID shaderCount = (ShaderID)shaderList.size();

	for (auto& update : updateDrawPropList)
	{
		const DrawPropID id = update.first;
		const DrawProp& prop = update.second;

		if (id >= psoGeneratorList.size())
		{
			DebugReport(RenderReport_Contract, L"[PSO] 업데이트 drawPropId 범위 밖 (빌드 전 업데이트?), 무시");
			continue;
		}

		if (prop.VSId == 0 || prop.PSId == 0 || prop.VSId >= shaderCount || prop.PSId >= shaderCount)
		{
			DebugReport(RenderReport_Contract, L"[PSO] 업데이트 drawProp 셰이더 id 잘못됨, 기존 유지");
			continue;
		}

		retiredPSOLists[retireSlot].push_back(std::move(psoGeneratorList[id]));
		psoGeneratorList[id] = {};
		psoGeneratorList[id].psoBody = prop;
	}

	updateDrawPropList.clear();
}

void D3D12Renderer::UpdateCombinedTexture()
{
	using namespace DataHub;

	if (updateTexPropList.empty())
		return;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	for (auto& update : updateTexPropList)
	{
		const TexPropID id = update.first;
		const TextureProp& prop = update.second;

		if (id >= combinedTextureList.size())
		{
			DebugReport(RenderReport_Contract, L"[CTex] 업데이트 texPropId 범위 밖 (빌드 전 업데이트?), 무시");
			continue;
		}

		UINT rvStartIndex;
		if (!BuildCombinedView(prop, rvStartIndex))
		{
			DebugReport(RenderReport_Limit, L"[CTex] 디스크립터 힙 꽉참 (texProp 업데이트 무시)");
			continue;
		}

		if (combinedTextureList[id].RVHeapIndex != combinedTextureList[0].RVHeapIndex)
			retiredCTexViewLists[retireSlot].push_back(combinedTextureList[id]);
		combinedTextureList[id] = { rvStartIndex };

		RemoveCTexRef(texRefList_CTex, (CTexID)id);
		combinedTextureBodyList[id] = prop;

		for (size_t i = 0; i < CTEX_COMBINED_SIZE; i++)
		{
			if (prop.texId[i] >= DEFAULT_RES_TEX_COUNT && prop.texId[i] < texRefList_CTex.size())
				texRefList_CTex[prop.texId[i]].emplace_back((CTexID)id, (uint8_t)i);
		}
	}

	updateTexPropList.clear();
}

#pragma endregion


#pragma region ReleaseResource

void D3D12Renderer::ReleaseMesh(vector<bool>& releaseResIdMask)
{
	using namespace DataHub;
	

	releaseResIdMask.resize(meshList.size(), false);

	uint16_t lastId = (uint16_t)meshList.size();

	for (uint16_t id = 0; id < releaseResIdMask.size(); id++)
	{
		if (!releaseResIdMask[id])
			continue;


		MeshID idUsable = meshList.size() - 1;

		while (idUsable > id && releaseResIdMask[idUsable])
			idUsable--;

		if (idUsable <= id)
		{
			for (uint16_t k = id; k < releaseResIdMask.size(); k++)
				RemoveIdInLUT(meshIdLUT, k);

			lastId = id;
			break;
		}

		std::swap(meshList[id], meshList[idUsable]);
		std::swap(meshViewList[id], meshViewList[idUsable]);
		std::swap(meshSubMeshCountList[id], meshSubMeshCountList[idUsable]);
		std::swap(csRefList_Mesh[id], csRefList_Mesh[idUsable]);

		SwapAndRemoveIdInLUT(meshIdLUT, id, idUsable);

		releaseResIdMask[id] = false;
		releaseResIdMask[idUsable] = true;

	}

	meshList.resize(lastId);
	meshViewList.resize(lastId);
	meshSubMeshCountList.resize(lastId);
	csRefList_Mesh.resize(lastId);

	releaseResIdMask = {};
	releaseResIdMask.assign(meshList.size(), false);
}

void D3D12Renderer::ReleaseTexture(vector<bool>& releaseResIdMask)
{
	using namespace DataHub;


	releaseResIdMask.resize(textureList.size(), false);

	uint16_t lastId = (uint16_t)textureList.size();

	for (uint16_t id = 0; id < releaseResIdMask.size(); id++)
	{
		if (!releaseResIdMask[id])
			continue;


		TexID idUsable = textureList.size() - 1;

		while (idUsable > id && releaseResIdMask[idUsable])
			idUsable--;

		if (idUsable <= id)
		{
			for (uint16_t k = id; k < releaseResIdMask.size(); k++)
				RemoveIdInLUT(textureIdLUT, k);

			lastId = id;
			break;
		}

		std::swap(textureList[id], textureList[idUsable]);
		std::swap(texRefList_CTex[id], texRefList_CTex[idUsable]);

		for (const auto& ref : texRefList_CTex[id])
			combinedTextureBodyList[ref.first].texId[ref.second] = id;

		const bool usableSTex = texRefMask_STex[idUsable];
		texRefMask_STex[idUsable] = texRefMask_STex[id];
		texRefMask_STex[id] = usableSTex;

		std::swap(singleTextureList[id], singleTextureList[idUsable]);
		std::swap(frameMarkList_STex[id], frameMarkList_STex[idUsable]);
		std::swap(texRefList_WTex[id], texRefList_WTex[idUsable]);
		

		SwapAndRemoveIdInLUT(textureIdLUT, id, idUsable);

		releaseResIdMask[id] = false;
		releaseResIdMask[idUsable] = true;
		
	}

	
	for (size_t k = lastId; k < textureList.size(); k++)
	{
		ReleaseWritableTexture(texRefList_WTex[k]);

		if (texRefMask_STex[k])
			rvHeap->UnSetSRV(singleTextureList[k].RVHeapIndex);
	}

	textureList.resize(lastId);
	texRefList_CTex.resize(lastId);
	texRefMask_STex.resize(lastId);
	singleTextureList.resize(lastId);
	frameMarkList_STex.resize(lastId);
	texRefList_WTex.resize(lastId);

	releaseResIdMask = {};
	releaseResIdMask.assign(textureList.size(), false);
}

void D3D12Renderer::ReleaseCSBuffer(vector<bool>& releaseResIdMask)
{
	using namespace DataHub;

	const UINT retireSlot = (cpuFrameIndex + FRAME_COUNT - 1) % FRAME_COUNT;


	releaseResIdMask.resize(csBufferList.size(), false);

	uint16_t lastId = (uint16_t)csBufferList.size();

	for (uint16_t id = 0; id < releaseResIdMask.size(); id++)
	{
		if (!releaseResIdMask[id])
			continue;


		uint16_t idUsable = csBufferList.size() - 1;

		while (idUsable > id && releaseResIdMask[idUsable])
			idUsable--;

		if (idUsable <= id)
		{
			for (uint16_t k = id; k < releaseResIdMask.size(); k++)
				RemoveIdInLUT(csBufferIdLUT, k);

			lastId = id;
			break;
		}

		std::swap(csBufferList[id], csBufferList[idUsable]);
		std::swap(csBufferViewList[id], csBufferViewList[idUsable]);

		for (auto& ref : csRefList_Mesh)
		{
			if (ref == id)
				ref = idUsable;
			else if (ref == idUsable)
				ref = id;
		}

		SwapAndRemoveIdInLUT(csBufferIdLUT, id, idUsable);

		releaseResIdMask[id] = false;
		releaseResIdMask[idUsable] = true;
	}


	for (size_t k = lastId; k < csBufferList.size(); k++)
	{
		for (size_t m = 0; m < csRefList_Mesh.size(); m++)
		{
			if (csRefList_Mesh[m] != k)
				continue;

			DebugReport(RenderReport_Contract, L"[Link] 컴퓨트 버퍼 해제로 링크 메쉬 끊김 (빈 메쉬로)");
			UnlinkMesh((MeshID)m, retireSlot);
		}
	}

	csBufferList.resize(lastId);
	csBufferViewList.resize(lastId);

	releaseResIdMask = {};
	releaseResIdMask.assign(csBufferList.size(), false);
}


void D3D12Renderer::ReleasePSOBody(vector<bool>& releaseResIdMask)
{
	using namespace DataHub;


	releaseResIdMask.resize(psoGeneratorList.size(), false);

	uint16_t lastId = (uint16_t)psoGeneratorList.size();

	for (uint16_t id = 0; id < releaseResIdMask.size(); id++)
	{
		if (!releaseResIdMask[id])
			continue;


		PSOID idUsable = psoGeneratorList.size() - 1;

		while (idUsable > id && releaseResIdMask[idUsable])
			idUsable--;

		if (idUsable <= id)
		{
			for (uint16_t k = id; k < releaseResIdMask.size(); k++)
				RemoveIdInLUT(drawPropIdLUT, k);

			lastId = id;
			break;
		}

		std::swap(psoGeneratorList[id], psoGeneratorList[idUsable]);

		SwapAndRemoveIdInLUT(drawPropIdLUT, id, idUsable);

		releaseResIdMask[id] = false;
		releaseResIdMask[idUsable] = true;

	}


	psoGeneratorList.resize(lastId);

	releaseResIdMask = {};
	releaseResIdMask.assign(psoGeneratorList.size(), false);
}

void D3D12Renderer::ReleaseCombinedTexture(vector<bool>& releaseResIdMask)
{
	using namespace DataHub;


	releaseResIdMask.resize(combinedTextureList.size(), false);

	uint16_t lastId = (uint16_t)combinedTextureList.size();

	for (uint16_t id = 0; id < releaseResIdMask.size(); id++)
	{
		if (!releaseResIdMask[id])
			continue;


		CTexID idUsable = combinedTextureList.size() - 1;

		while (idUsable > id && releaseResIdMask[idUsable])
			idUsable--;

		if (idUsable <= id)
		{
			for (uint16_t k = id; k < releaseResIdMask.size(); k++)
				RemoveIdInLUT(texPropIdLUT, k);

			lastId = id;
			break;
		}


		std::swap(combinedTextureList[id], combinedTextureList[idUsable]);
		std::swap(combinedTextureBodyList[id], combinedTextureBodyList[idUsable]);

		SwapAndRemoveIdInLUT(texPropIdLUT, id, idUsable);
		SwapCTexRef(texRefList_CTex, id, idUsable);

		releaseResIdMask[id] = false;
		releaseResIdMask[idUsable] = true;

	}

	for (size_t k = lastId; k < combinedTextureList.size(); k++)
	{
		if (combinedTextureList[k].RVHeapIndex != combinedTextureList[0].RVHeapIndex)
			rvHeap->UnSetSRV(combinedTextureList[k].RVHeapIndex, CTEX_COMBINED_SIZE);
		RemoveCTexRef(texRefList_CTex, (CTexID)k);
	}

	combinedTextureList.resize(lastId);
	combinedTextureBodyList.resize(lastId);
	
	releaseResIdMask = {};
	releaseResIdMask.assign(combinedTextureList.size(), false);
}

#pragma endregion


#pragma region Instant

void D3D12Renderer::ResizeTextureInstant()
{
	auto& rtInstantList = rtTextureInstantLists[cpuFrameIndex];
	auto& dsInstantList = dsTextureInstantLists[cpuFrameIndex];
	const uint16_t rtNeed = renderFrames[cpuFrameIndex].rtInstantNeedCount;
	const uint16_t dsNeed = renderFrames[cpuFrameIndex].dsInstantNeedCount;

	if (rtInstantList.size() >= rtNeed && dsInstantList.size() >= dsNeed)
		return;	

		
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();
	

	uint16_t rtExpandNeed;
	uint16_t dsExpandNeed;

	if (rtInstantList.size() > rtNeed)
		rtExpandNeed = 0;
	else
		rtExpandNeed = rtNeed - rtInstantList.size();

	if (dsInstantList.size() > dsNeed)
		dsExpandNeed = 0;
	else
		dsExpandNeed = dsNeed - dsInstantList.size();
		

	rtInstantList.reserve(rtNeed);
	rtSTexInstantLists[cpuFrameIndex].reserve(rtNeed);
	rtvInstantLists[cpuFrameIndex].reserve(rtNeed);

	dsInstantList.reserve(dsNeed);
	dsSTexInstantLists[cpuFrameIndex].reserve(dsNeed);
	dsvInstantLists[cpuFrameIndex].reserve(dsNeed);
	dsvReadOnlyInstantLists[cpuFrameIndex].reserve(dsNeed);


	#pragma region InstantBase
	
	D3D12_CLEAR_VALUE rtoptClear = {};
	rtoptClear.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtoptClear.Color[0] = rtoptClear.Color[1] = rtoptClear.Color[2] = rtoptClear.Color[3] = 0.0f;

	D3D12_RESOURCE_DESC rtTextureDesc = {};
	rtTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	rtTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtTextureDesc.SampleDesc.Count = 1;
	rtTextureDesc.SampleDesc.Quality = 0;
	rtTextureDesc.MipLevels = 1;
	rtTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtTextureDesc.DepthOrArraySize = 1;
	rtTextureDesc.Width = bbWidth;
	rtTextureDesc.Height = bbHeight;

	D3D12_SHADER_RESOURCE_VIEW_DESC rtSViewdesc = {};
	rtSViewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	rtSViewdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtSViewdesc.Texture2D.MipLevels = 1;
	rtSViewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_RENDER_TARGET_VIEW_DESC rtvdesc = {};
	rtvdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvdesc.Texture2D.MipSlice = 0;
	
	
	D3D12_CLEAR_VALUE dsoptClear = {};
	dsoptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsoptClear.DepthStencil = D3D12_DEPTH_STENCIL_VALUE{1.0f, 0};

	D3D12_RESOURCE_DESC dsTextureDesc = {};
	dsTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsTextureDesc.SampleDesc.Count = 1;
	dsTextureDesc.SampleDesc.Quality = 0;
	dsTextureDesc.MipLevels = 1;
	dsTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsTextureDesc.DepthOrArraySize = 1;
	dsTextureDesc.Width = bbWidth;
	dsTextureDesc.Height = bbHeight;

	D3D12_SHADER_RESOURCE_VIEW_DESC dsSViewdesc = {};
	dsSViewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	dsSViewdesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	dsSViewdesc.Texture2D.MipLevels = 1;
	dsSViewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
	dsvdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvdesc.Texture2D.MipSlice = 0;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvReaddesc = {};
	dsvReaddesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvReaddesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvReaddesc.Texture2D.MipSlice = 0;
	dsvReaddesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

	#pragma endregion

	for (uint16_t i = 0; i < rtExpandNeed; i++)
	{
		DX12_TextureInstant rtInstant = {};

		rtInstant.usage = DX12_TextureUsage_RTV;

		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&rtTextureDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&rtoptClear,
			IID_PPV_ARGS(&rtInstant.Tex));

		if (FAILED(hr))
		{
			DebugReport(DataHub::RenderReport_Limit, L"[Instant] RT 인스턴트 생성 실패 (메모리 부족?), 이번 프레임 무효");
			renderFrames[cpuFrameIndex].frameInvalidated = true;
			return;
		}

		UINT rtSViewIdx = 0, rtvIdx = 0;
		const bool rtSViewOk = rvHeap->SetSRV(rtInstant.Tex.Get(), &rtSViewdesc, rtSViewIdx);
		const bool rtvOk = rtSViewOk && rtvHeap->SetRTV(rtInstant.Tex.Get(), &rtvdesc, rtvIdx);

		if (!rtvOk)
		{
			if (rtSViewOk)
				rvHeap->UnSetSRV(rtSViewIdx);

			DebugReport(DataHub::RenderReport_Limit, L"[Instant] RT 인스턴트 뷰 자리 부족 (디스크립터 힙 한도), 이번 프레임 무효");
			renderFrames[cpuFrameIndex].frameInvalidated = true;
			return;
		}

		rtInstantList.emplace_back(std::move(rtInstant));

		rtSTexInstantLists[cpuFrameIndex].emplace_back(rtSViewIdx);
		rtvInstantLists[cpuFrameIndex].emplace_back(rtvIdx);
	}


	for (uint16_t i = 0; i < dsExpandNeed; i++)
	{
		DX12_TextureInstant dsInstant = {};

		dsInstant.usage = DX12_TextureUsage_DSV;

		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&dsTextureDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ,
			&dsoptClear,
			IID_PPV_ARGS(&dsInstant.Tex));

		if (FAILED(hr))
		{
			DebugReport(DataHub::RenderReport_Limit, L"[Instant] DS 인스턴트 생성 실패 (메모리 부족?), 이번 프레임 무효");
			renderFrames[cpuFrameIndex].frameInvalidated = true;
			return;
		}

		UINT dsSViewIdx = 0, dsvIdx = 0, dsvReadIdx = 0;
		const bool dsSViewOk = rvHeap->SetSRV(dsInstant.Tex.Get(), &dsSViewdesc, dsSViewIdx);
		const bool dsvOk = dsSViewOk && dsvHeap->SetDSV(dsInstant.Tex.Get(), &dsvdesc, dsvIdx);
		const bool dsvReadOk = dsvOk && dsvHeap->SetDSV(dsInstant.Tex.Get(), &dsvReaddesc, dsvReadIdx);

		if (!dsvReadOk)
		{
			if (dsSViewOk)
				rvHeap->UnSetSRV(dsSViewIdx);
			if (dsvOk)
				dsvHeap->UnSetDSV(dsvIdx);

			DebugReport(DataHub::RenderReport_Limit, L"[Instant] DS 인스턴트 뷰 자리 부족 (디스크립터 힙 한도), 이번 프레임 무효");
			renderFrames[cpuFrameIndex].frameInvalidated = true;
			return;
		}

		dsInstantList.emplace_back(std::move(dsInstant));

		dsSTexInstantLists[cpuFrameIndex].emplace_back(dsSViewIdx);
		dsvInstantLists[cpuFrameIndex].emplace_back(dsvIdx);
		dsvReadOnlyInstantLists[cpuFrameIndex].emplace_back(dsvReadIdx);
	}
}

#pragma endregion


#pragma region Helper

void D3D12Renderer::BeginUploadCommands()
{
	uploadCMDAllocator->Reset();
	renderCMDList_Master->Reset(uploadCMDAllocator.Get(), nullptr);
}

void D3D12Renderer::FlushUploadCommands()
{
	renderCMDList_Master->Close();

	ID3D12CommandList* cmdLists[] = {renderCMDList_Master.Get()};
	renderCMDQueue->ExecuteCommandLists(1, cmdLists);

	SignalFence();
	WaitGPUProcess();
}


void D3D12Renderer::MarkOrBuildSingleTexture(TexID texId)
{
	if (!texRefMask_STex[texId])
	{
		DX12_Texture& texInfo = textureList[texId];
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		UINT rvIndex;

		FillTextureSRVDesc(texInfo, desc);

		rvHeap->SetSRV(
			texInfo.Tex.Get(),
			&desc,
			rvIndex
		);

		singleTextureList[texId] = {rvIndex};
		texRefMask_STex[texId] = true;
	}

	frameMarkList_STex[texId] = static_cast<UINT16>(currentFrameCount % UINT16_MAX);
}

void D3D12Renderer::BuildWritableTexture(const DX12_Texture& texture, const DataHub::Texture& src, DX12_WTexRef& ref)
{
	const DX12_TextureUsage newUsage = static_cast<DX12_TextureUsage>(src.usage);

	if (newUsage == DX12_TextureUsage_ReadOnly)
	{
		ReleaseWritableTexture(ref);
		return;
	}

	if (texture.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[WTex] 2D 가 아닌 텍스처의 쓰기 뷰 미지원 (쓰기 뷰 없이)");
		ReleaseWritableTexture(ref);
		return;
	}

	const bool inPlace = (ref.usage == newUsage);
	if (!inPlace)
		ReleaseWritableTexture(ref);

	UINT vIndex = inPlace ? writableTextureList[ref.idx].VHeapIndex : 0;
	bool allocated = true;

	D3D12_CLEAR_VALUE clear = {};
	D3D12_RESOURCE_STATES home;
	D3D12_RESOURCE_STATES write;

	if (newUsage == DX12_TextureUsage_RTV)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texture.Format;

		clear.Format = rtvDesc.Format;
		memcpy(clear.Color, src.clearValue.color, sizeof(clear.Color));
		home = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		write = D3D12_RESOURCE_STATE_RENDER_TARGET;

		if (texture.ArraySize > 1)
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = texture.ArraySize;
		}
		else
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		if (inPlace)
			rtvHeap->UpdateRTV(vIndex, texture.Tex.Get(), &rtvDesc);
		else
			allocated = rtvHeap->SetRTV(texture.Tex.Get(), &rtvDesc, vIndex);
	}
	else if (newUsage == DX12_TextureUsage_UAV)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texture.Format;

		home = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		write = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		if (texture.ArraySize > 1)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = texture.ArraySize;
		}
		else
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		if (inPlace)
			rvHeap->UpdateUAV(vIndex, texture.Tex.Get(), &uavDesc);
		else
			allocated = rvHeap->SetUAV(texture.Tex.Get(), &uavDesc, vIndex);
	}
	else
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescs[2] = {};
		dsvDescs[0].Format = GetDSVFormat(texture.Format);

		clear.Format = dsvDescs[0].Format;
		clear.DepthStencil = D3D12_DEPTH_STENCIL_VALUE{ src.clearValue.depth, src.clearValue.stencil };
		home = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ;
		write = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		if (texture.ArraySize > 1)
		{
			dsvDescs[0].ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDescs[0].Texture2DArray.ArraySize = texture.ArraySize;
		}
		else
			dsvDescs[0].ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		dsvDescs[1] = dsvDescs[0];
		dsvDescs[1].Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

		if (inPlace)
		{
			dsvHeap->UpdateDSV(vIndex, texture.Tex.Get(), &dsvDescs[0]);
			dsvHeap->UpdateDSV(vIndex + 1, texture.Tex.Get(), &dsvDescs[1]);
		}
		else
		{
			ID3D12Resource* resList[2] = { texture.Tex.Get(), texture.Tex.Get() };
			D3D12_DEPTH_STENCIL_VIEW_DESC* descList[2] = { &dsvDescs[0], &dsvDescs[1] };
			allocated = dsvHeap->SetCombinedDSV(resList, descList, 2, vIndex);
		}
	}

	if (!allocated)
	{
		DebugReport(DataHub::RenderReport_Limit, L"[WTex] RTV / DSV / RV 힙 꽉참 (쓰기 뷰 없이)");
		return;
	}

	if (src.subresources.empty() && newUsage != DX12_TextureUsage_UAV)
	{
		const CD3DX12_RESOURCE_BARRIER toWrite = CD3DX12_RESOURCE_BARRIER::Transition(texture.Tex.Get(), home, write);
		const CD3DX12_RESOURCE_BARRIER toHome = CD3DX12_RESOURCE_BARRIER::Transition(texture.Tex.Get(), write, home);

		renderCMDList_Master->ResourceBarrier(1, &toWrite);

		if (newUsage == DX12_TextureUsage_RTV)
			renderCMDList_Master->ClearRenderTargetView(rtvHeap->GetCPUHandle(vIndex), clear.Color, 0, nullptr);
		else
			renderCMDList_Master->ClearDepthStencilView(dsvHeap->GetCPUHandle(vIndex), GetDSClearFlags(clear.Format), clear.DepthStencil.Depth, clear.DepthStencil.Stencil, 0, nullptr);

		renderCMDList_Master->ResourceBarrier(1, &toHome);
	}

	if (inPlace)
	{
		writableClearList[ref.idx] = clear;
		return;
	}

	uint16_t idx;

	if (!blankedIdxList_WTex.empty())
	{
		idx = blankedIdxList_WTex.back();
		blankedIdxList_WTex.pop_back();
		writableTextureList[idx] = { vIndex };
		writableClearList[idx] = clear;
	}
	else
	{
		idx = (uint16_t)writableTextureList.size();
		writableTextureList.push_back({ vIndex });
		writableClearList.push_back(clear);
	}

	ref = { newUsage, idx };
}

void D3D12Renderer::UnlinkMesh(MeshID meshId, UINT retireSlot)
{
	retiredMeshLists[retireSlot].push_back(std::move(meshList[meshId]));

	meshList[meshId] = {};
	meshViewList[meshId] = {};
	meshViewList[meshId].subMeshes.push_back(DX12_SubMesh{ 0, 0, 0 });
	DataHub::meshSubMeshCountList[meshId] = 1;
	csRefList_Mesh[meshId] = 0;
}

void D3D12Renderer::ReleaseWritableTexture(DX12_WTexRef& ref)
{
	if (ref.usage == DX12_TextureUsage_ReadOnly)
		return;

	const UINT vIndex = writableTextureList[ref.idx].VHeapIndex;

	if (ref.usage == DX12_TextureUsage_DSV)
		dsvHeap->UnSetDSV(vIndex, 2);
	else if (ref.usage == DX12_TextureUsage_UAV)
		rvHeap->UnSetSRV(vIndex);
	else
		rtvHeap->UnSetRTV(vIndex);

	writableTextureList[ref.idx] = {};
	blankedIdxList_WTex.push_back(ref.idx);
	ref = {};
}


bool D3D12Renderer::CreateMeshResource(const DataHub::Mesh& src, DX12_Mesh& mesh, DX12_MeshView& view, vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const CD3DX12_RANGE readRange(0, 0);
	UINT8* pDataBegin;


	const void* vertexData = nullptr;
	UINT vertexCount = 0;
	UINT stride = 0;

	switch (src.vertexType)
	{
	case VertexType_Simple:
		vertexData = src.verticesSimple.data();
		vertexCount = (UINT)src.verticesSimple.size();
		stride = sizeof(VertexSimple);
		break;

	case VertexType_Static:
		vertexData = src.verticesStatic.data();
		vertexCount = (UINT)src.verticesStatic.size();
		stride = sizeof(VertexStatic);
		break;

	case VertexType_Skinned:
		vertexData = src.verticesSkinned.data();
		vertexCount = (UINT)src.verticesSkinned.size();
		stride = sizeof(VertexSkinned);
		break;

	default:
		break;
	}

	if (vertexData == nullptr || vertexCount == 0 || src.indices.empty())
		return false;


	mesh = {};
	view = {};
	mesh.vertexType = src.vertexType;
	mesh.vertexCount = vertexCount;

	const UINT64 sizeofVertices = (UINT64)vertexCount * stride;


	ComPtr<ID3D12Resource> uploadVertexBuffer;
	auto vtxResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeofVertices);

	HRESULT hr = device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&vtxResDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mesh.VB)
	);

	if (SUCCEEDED(hr))
	{
		hr = device->CreateCommittedResource(
			&UPLOAD_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&vtxResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadVertexBuffer)
		);
	}

	if (FAILED(hr))
	{
		DebugReport(RenderReport_Contract, L"[Mesh] 버텍스 버퍼 생성 실패");
		return false;
	}

	uploadVertexBuffer->Map(0, &readRange, (void**)(&pDataBegin));
	memcpy(pDataBegin, vertexData, sizeofVertices);
	uploadVertexBuffer->Unmap(0, nullptr);

	view.VBV.BufferLocation = mesh.VB->GetGPUVirtualAddress();
	view.VBV.StrideInBytes = stride;
	view.VBV.SizeInBytes = (UINT)sizeofVertices;

	renderCMDList_Master->CopyBufferRegion(mesh.VB.Get(), 0, uploadVertexBuffer.Get(), 0, sizeofVertices);

	auto vtxResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		mesh.VB.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	);
	renderCMDList_Master->ResourceBarrier(1, &vtxResBar);


	uploadBufferList.push_back(std::move(uploadVertexBuffer));


	if (!CreateIndexResource(src.indices, src.subMeshes, vertexCount, mesh, view, uploadBufferList))
	{
		uploadBufferList.push_back(mesh.VB);
		return false;
	}

	return true;
}

bool D3D12Renderer::CreateIndexResource(const std::vector<DataHub::VIndex>& indices, const std::vector<DataHub::SubMesh>& subMeshes, UINT vertexCount, DX12_Mesh& mesh, DX12_MeshView& view, vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const CD3DX12_RANGE readRange(0, 0);
	UINT8* pDataBegin;

	const UINT indexCount = (UINT)indices.size();

	if (indexCount == 0)
		return false;

	mesh.indexCount = indexCount;

	const UINT64 sizeofIndices = (UINT64)indexCount * sizeof(VIndex);


	ComPtr<ID3D12Resource> uploadIndexBuffer;
	auto idxResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeofIndices);

	HRESULT hr = device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&idxResDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mesh.IB)
	);

	if (SUCCEEDED(hr))
	{
		hr = device->CreateCommittedResource(
			&UPLOAD_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&idxResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadIndexBuffer)
		);
	}

	if (FAILED(hr))
	{
		DebugReport(RenderReport_Contract, L"[Mesh] 인덱스 버퍼 생성 실패");
		return false;
	}

	uploadIndexBuffer->Map(0, &readRange, (void**)(&pDataBegin));
	memcpy(pDataBegin, indices.data(), sizeofIndices);
	uploadIndexBuffer->Unmap(0, nullptr);

	view.IBV.BufferLocation = mesh.IB->GetGPUVirtualAddress();
	view.IBV.Format = MESH_INDEX_FORMAT;
	view.IBV.SizeInBytes = (UINT)sizeofIndices;

	renderCMDList_Master->CopyBufferRegion(mesh.IB.Get(), 0, uploadIndexBuffer.Get(), 0, sizeofIndices);

	auto idxResBar = CD3DX12_RESOURCE_BARRIER::Transition(
		mesh.IB.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER
	);
	renderCMDList_Master->ResourceBarrier(1, &idxResBar);


	view.subMeshes.clear();

	if (subMeshes.empty())
	{
		view.subMeshes.push_back(DX12_SubMesh{ 0, indexCount, 0 });
	}
	else
	{
		for (size_t s = 0; s < subMeshes.size(); s++)
		{
			const SubMesh& subSrc = subMeshes[s];
			DX12_SubMesh sub = { subSrc.indexStart, subSrc.indexCount, (INT)subSrc.baseVertex };

			if (sub.indexStart >= indexCount || subSrc.baseVertex >= vertexCount)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Mesh] 서브메쉬 시작이 인덱스 / 정점 범위 밖 (빈 서브메쉬로)");
				sub.indexStart = 0;
				sub.indexCount = 0;
			}
			else if (sub.indexStart + sub.indexCount > indexCount)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Mesh] 서브메쉬 끝이 인덱스 범위 밖 (잘라냄)");
				sub.indexCount = indexCount - sub.indexStart;
			}

			view.subMeshes.push_back(sub);
		}
	}


	uploadBufferList.push_back(std::move(uploadIndexBuffer));

	return true;
}

bool D3D12Renderer::CreateBufferResource(const DataHub::ComputeBuffer& src, DX12_Buffer& buffer, DX12_BufferView& view, vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	const UINT64 sizeofData = (UINT64)src.stride * src.count;

	if (src.stride == 0 || src.stride % 4 != 0 || src.count == 0)
		return false;

	if (!src.data.empty() && src.data.size() != sizeofData)
		return false;


	buffer = {};
	view = {};
	buffer.stride = src.stride;
	buffer.count = src.count;

	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeofData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	ComPtr<ID3D12Resource> uploadBuffer;
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeofData);

	HRESULT hr = device->CreateCommittedResource(
		&DEFAULT_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&buffer.Res)
	);

	if (SUCCEEDED(hr) && !src.data.empty())
	{
		hr = device->CreateCommittedResource(
			&UPLOAD_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&uploadDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)
		);
	}

	if (FAILED(hr))
	{
		DebugReport(DataHub::RenderReport_Contract, L"[CSBuffer] 버퍼 생성 실패");
		return false;
	}

	view.address = buffer.Res->GetGPUVirtualAddress();


	if (src.data.empty())
	{
		auto resBar = CD3DX12_RESOURCE_BARRIER::Transition(
			buffer.Res.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			CSBUFFER_HOME_STATE
		);
		renderCMDList_Master->ResourceBarrier(1, &resBar);

		return true;
	}


	const CD3DX12_RANGE readRange(0, 0);
	UINT8* pDataBegin;

	uploadBuffer->Map(0, &readRange, (void**)(&pDataBegin));
	memcpy(pDataBegin, src.data.data(), sizeofData);
	uploadBuffer->Unmap(0, nullptr);

	renderCMDList_Master->CopyBufferRegion(buffer.Res.Get(), 0, uploadBuffer.Get(), 0, sizeofData);

	auto resBar = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Res.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		CSBUFFER_HOME_STATE
	);
	renderCMDList_Master->ResourceBarrier(1, &resBar);


	uploadBufferList.push_back(std::move(uploadBuffer));

	return true;
}

bool D3D12Renderer::CreateTextureResource(const DataHub::Texture& src, DX12_Texture& texture, vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	auto& data = src.data;


	D3D12_CLEAR_VALUE optClear = {};
	D3D12_CLEAR_VALUE* pOptClear = nullptr;
	D3D12_RESOURCE_STATES resState = {};


	D3D12_RESOURCE_DESC textureDesc = {};

	switch (src.usage)
	{
	default:
	case TextureUsage_ReadOnly:
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.Format = ToDXGI(data.format);

		resState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		break;

	case TextureUsage_RT:
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		textureDesc.Format = ToDXGI(data.format);

		optClear.Format = textureDesc.Format;
		memcpy(optClear.Color, src.clearValue.color, sizeof(optClear.Color));
		pOptClear = &optClear;

		resState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		break;

	case TextureUsage_DS:
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		textureDesc.Format = GetDSFormat(data.format);

		optClear.Format = GetDSVFormat(textureDesc.Format);
		optClear.DepthStencil = D3D12_DEPTH_STENCIL_VALUE{ src.clearValue.depth, src.clearValue.stencil };
		pOptClear = &optClear;

		resState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ;
		break;

	case TextureUsage_Compute:
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		textureDesc.Format = ToDXGI(data.format);

		resState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		break;
	}

	if (textureDesc.Format == DXGI_FORMAT_UNKNOWN)
	{
		DebugReport(RenderReport_Contract, L"[Tex] 포맷 UNKNOWN (표에 없는 포맷 / DS 인데 깊이 포맷 아님)");
		return false;
	}

	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	textureDesc.MipLevels = data.mipLevels;
	textureDesc.Width = data.width;
	textureDesc.Height = data.height;


	switch (data.textureType)
	{
	default:
	case TextureType_2D:
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.DepthOrArraySize = data.arraySize;
		break;

	case TextureType_Cube:
		if (data.arraySize < 6 || data.arraySize % 6 != 0)
		{
			DebugReport(RenderReport_Contract, L"[Tex] Cube 타입인데 arraySize 가 6의 배수가 아님");
			return false;
		}

		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.DepthOrArraySize = data.arraySize;
		break;

	case TextureType_1D:
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		textureDesc.DepthOrArraySize = data.arraySize;
		break;

	case TextureType_3D:
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		textureDesc.DepthOrArraySize = data.depth;
		break;
	}


	ComPtr<ID3D12Resource> resource;

	const UINT numSubresources = (UINT)src.subresources.size();
	const UINT expectedSubresources = data.mipLevels * (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : data.arraySize);

	if (numSubresources != 0 && numSubresources != expectedSubresources)
	{
		DebugReport(RenderReport_Contract, L"[Tex] 서브리소스 수가 mipLevels * arraySize 와 다름 (3D 는 mipLevels)");
		return false;
	}

	if (numSubresources == 0)
	{
		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			resState,
			pOptClear,
			IID_PPV_ARGS(&resource));

		if (FAILED(hr))
		{
			DebugReport(RenderReport_Contract, L"[Tex] 텍스처 리소스 생성 실패 (포맷 / 용도 / 차원 / 크기 조합 확인)");
			return false;
		}
	}
	else
	{
		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			pOptClear,
			IID_PPV_ARGS(&resource));

		if (FAILED(hr))
		{
			DebugReport(RenderReport_Contract, L"[Tex] 텍스처 리소스 생성 실패 (포맷 / 용도 / 차원 / 크기 조합 확인)");
			return false;
		}


		const UINT64 uploadTextureSize = GetRequiredIntermediateSize(resource.Get(), 0, numSubresources);
		ComPtr<ID3D12Resource> uploadTexture;

		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureSize);

		hr = device->CreateCommittedResource(
			&UPLOAD_HEAP_PROPS,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadTexture));

		if (FAILED(hr))
		{
			DebugReport(RenderReport_Contract, L"[Tex] 텍스처 업로드 버퍼 생성 실패");
			return false;
		}


		vector<D3D12_SUBRESOURCE_DATA> subResData(numSubresources, {});

		const std::byte* base = src.rawData.data();
		for (size_t i = 0; i < numSubresources; ++i)
		{
			subResData[i].pData = base + src.subresources[i].offset;
			subResData[i].RowPitch = static_cast<LONG_PTR>(src.subresources[i].rowPitch);
			subResData[i].SlicePitch = static_cast<LONG_PTR>(src.subresources[i].slicePitch);
		}


		const UINT64 uploaded = UpdateSubresources(renderCMDList_Master.Get(), resource.Get(), uploadTexture.Get(),
							0,
							0, numSubresources,
							subResData.data());

		if (uploaded == 0)
		{
			DebugReport(RenderReport_Contract, L"[Tex] 텍스처 데이터 업로드 실패 (서브리소스 크기 / pitch 확인)");
			return false;
		}


		auto resBar = CD3DX12_RESOURCE_BARRIER::Transition(
			resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			resState
		);

		renderCMDList_Master->ResourceBarrier(1, &resBar);


		uploadBufferList.push_back(move(uploadTexture));
	}


	texture = DX12_Texture{
		move(resource),
		textureDesc.DepthOrArraySize,
		textureDesc.MipLevels,
		textureDesc.Format,
		textureDesc.Dimension,
		data.textureType == TextureType_Cube
	};

	return true;
}

bool D3D12Renderer::BuildCombinedView(const DataHub::TextureProp& prop, UINT& rvStartIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC descList[CTEX_COMBINED_SIZE] = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC* srvDescList[CTEX_COMBINED_SIZE] = {};
	ID3D12Resource* subresList[CTEX_COMBINED_SIZE] = {};

	for (size_t i = 0; i < CTEX_COMBINED_SIZE; i++)
	{
		const TexID texId = prop.texId[i];

		if (texId >= textureList.size())
		{
			subresList[i] = textureList[0].Tex.Get();
			DebugReport(DataHub::RenderReport_Contract, L"[CTex] texProp 의 texId 범위 밖 (기본 텍스처로 대체)");
			continue;
		}

		if (releaseTexIdMask[texId])
		{
			subresList[i] = textureList[0].Tex.Get();
			DebugReport(DataHub::RenderReport_Contract, L"[CTex] 삭제된 텍스처를 texProp 이 참조 (기본 텍스처로 대체)");
			continue;
		}

		const DX12_Texture& texInfo = textureList[texId];

		subresList[i] = texInfo.Tex.Get();
		srvDescList[i] = &descList[i];

		FillTextureSRVDesc(texInfo, descList[i]);
		if (prop.texMipLevels[i] != 0)
			descList[i].Texture2D.MipLevels = prop.texMipLevels[i];
	}

	return rvHeap->SetCombinedSRV(subresList, srvDescList, CTEX_COMBINED_SIZE, rvStartIndex);
}


bool D3D12Renderer::CompileShader(ShaderID shaderId, uint8_t variant, ComPtr<IDxcBlob>& outBlob)
{
	using namespace DataHub;

	if (shaderId >= shaderList.size() || dxcCompiler == nullptr)
		return false;

	const auto& [filename, type] = shaderList[shaderId];


	ComPtr<IDxcBlobEncoding> source;
	HRESULT hr = dxcUtils->LoadFile(filename.c_str(), nullptr, &source);

	if (FAILED(hr))
	{
		OutputDebugStringW(L"[Shader] 파일 로드 실패: ");
		OutputDebugStringW(filename.c_str());
		OutputDebugStringW(L"\n");
		return false;
	}

	DxcBuffer sourceBuffer = { source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_ACP };


	static const wchar_t* VS_PROFILES[] = { L"vs_6_0", L"vs_6_1", L"vs_6_2", L"vs_6_3", L"vs_6_4", L"vs_6_5", L"vs_6_6" };
	static const wchar_t* PS_PROFILES[] = { L"ps_6_0", L"ps_6_1", L"ps_6_2", L"ps_6_3", L"ps_6_4", L"ps_6_5", L"ps_6_6" };
	static const wchar_t* CS_PROFILES[] = { L"cs_6_0", L"cs_6_1", L"cs_6_2", L"cs_6_3", L"cs_6_4", L"cs_6_5", L"cs_6_6" };

	const size_t smIdx = (size_t)(shaderModel - D3D_SHADER_MODEL_6_0);

	const wchar_t* entry;
	const wchar_t* profile;

	switch (type)
	{
	case ShaderType_VS:	entry = L"VSMain";	profile = VS_PROFILES[smIdx];	break;
	case ShaderType_PS:	entry = L"PSMain";	profile = PS_PROFILES[smIdx];	break;
	case ShaderType_CS:	entry = L"CSMain";	profile = CS_PROFILES[smIdx];	break;
	default:			return false;
	}


	const wchar_t* macro = nullptr;

	if (variant != SHADER_VARIANT_NONE)
	{
		if (type == ShaderType_VS)
		{
			switch ((VertexType)(variant - 1))
			{
			case VertexType_Static:		macro = L"VERTEX_STATIC=1";		break;
			case VertexType_Skinned:	macro = L"VERTEX_SKINNED=1";	break;
			case VertexType_Simple:		macro = L"VERTEX_SIMPLE=1";		break;
			default: break;
			}
		}
		else
		{
			switch ((Action_OutputLayout)variant)
			{
			case OutputLayout_DepthOnly:	macro = L"OUTPUT_DEPTHONLY=1";	break;
			case OutputLayout_Color:		macro = L"OUTPUT_COLOR=1";		break;
			case OutputLayout_ColorId:		macro = L"OUTPUT_COLORID=1";	break;
			case OutputLayout_GBuffer:		macro = L"OUTPUT_GBUFFER=1";	break;
			default: break;
			}
		}
	}


	std::vector<LPCWSTR> args;
	args.push_back(L"-E");	args.push_back(entry);
	args.push_back(L"-T");	args.push_back(profile);
	args.push_back(L"-I");	args.push_back(defaultShaderDir.c_str());

	#ifdef _DEBUG
	args.push_back(L"-Zi");
	args.push_back(L"-Od");
	args.push_back(L"-Qembed_debug");
	#else
	args.push_back(L"-O3");
	#endif

	if (macro != nullptr)
	{
		args.push_back(L"-D");
		args.push_back(macro);
	}


	ComPtr<IDxcResult> result;
	hr = dxcCompiler->Compile(&sourceBuffer, args.data(), (UINT32)args.size(), dxcIncludeHandler.Get(), IID_PPV_ARGS(&result));

	if (FAILED(hr) || result == nullptr)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[Shader] DXC Compile 호출 실패");
		return false;
	}

	#ifdef _DEBUG
	ComPtr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

	if (errors != nullptr && errors->GetStringLength() > 0)
	{
		OutputDebugStringW(L"[Shader] ");
		OutputDebugStringW(filename.c_str());
		if (macro != nullptr) { OutputDebugStringW(L" -D "); OutputDebugStringW(macro); }
		OutputDebugStringW(L"\n");
		OutputDebugStringA(errors->GetStringPointer());
		OutputDebugStringA("\n");
	}
	#endif

	HRESULT status = S_OK;
	result->GetStatus(&status);

	if (FAILED(status))
		return false;

	hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&outBlob), nullptr);

	return SUCCEEDED(hr) && outBlob != nullptr;
}

IDxcBlob* D3D12Renderer::GetCompiledShader(ShaderID shaderId, uint8_t variant)
{
	using namespace DataHub;

	for (size_t i = 0; i < compiledShaderList.size(); i++)
	{
		const DX12_CompiledShader& entry = compiledShaderList[i];

		if (entry.shaderId == shaderId && entry.variant == variant)
			return entry.failed ? nullptr : entry.blob.Get();
	}

	if (shaderId >= shaderList.size())
		return nullptr;


	DX12_CompiledShader entry = {};
	entry.shaderId = shaderId;
	entry.variant = variant;
	entry.failed = !CompileShader(shaderId, variant, entry.blob);


	if (!entry.failed && variant != SHADER_VARIANT_NONE && shaderList[shaderId].second == ShaderType_PS)
	{
		DxcBuffer objectBuffer = { entry.blob->GetBufferPointer(), entry.blob->GetBufferSize(), 0 };
		ComPtr<ID3D12ShaderReflection> reflection;
		HRESULT hr = dxcUtils->CreateReflection(&objectBuffer, IID_PPV_ARGS(&reflection));

		uint8_t writtenMask = 0;

		if (SUCCEEDED(hr))
		{
			D3D12_SHADER_DESC shaderDesc = {};
			reflection->GetDesc(&shaderDesc);

			for (UINT p = 0; p < shaderDesc.OutputParameters; p++)
			{
				D3D12_SIGNATURE_PARAMETER_DESC param = {};
				reflection->GetOutputParameterDesc(p, &param);

				if (param.SystemValueType == D3D_NAME_TARGET && param.Register < RENDER_TARGET_MAX)
					writtenMask |= (uint8_t)(1 << param.Register);
			}
		}

		const uint8_t expectMask = OutputLayoutTargetMask((Action_OutputLayout)variant);

		if (FAILED(hr) || writtenMask != expectMask)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Shader] PS 의 SV_TARGET 집합이 출력 레이아웃과 다름 (변형 무효)");
			entry.failed = true;
		}
	}


	compiledShaderList.push_back(entry);

	return entry.failed ? nullptr : compiledShaderList.back().blob.Get();
}

ID3D12PipelineState* D3D12Renderer::GetComputePSO(ShaderID csId)
{
	using namespace DataHub;

	if (csId == 0 || csId >= shaderList.size() || shaderList[csId].second != ShaderType_CS)
		return nullptr;

	if (computePSOList.size() <= csId)
		computePSOList.resize((size_t)csId + 1);

	DX12_ComputePSO& entry = computePSOList[csId];

	if (entry.pso != nullptr || entry.failed)
		return entry.pso.Get();


	IDxcBlob* blob = GetCompiledShader(csId, SHADER_VARIANT_NONE);

	if (blob == nullptr)
	{
		entry.failed = true;
		DebugReport(RenderReport_Contract, L"[PSO] 컴퓨트 셰이더 컴파일 실패 (해당 디스패치 스킵됨)");
		return nullptr;
	}


	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = computeRootSig.Get();
	psoDesc.CS = { blob->GetBufferPointer(), blob->GetBufferSize() };

	HRESULT hr = D3D12Core::getInstance().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&entry.pso));

	if (FAILED(hr))
	{
		entry.pso.Reset();
		entry.failed = true;
		DebugReport(RenderReport_Contract, L"[PSO] 컴퓨트 PSO 생성 실패 (해당 디스패치 스킵됨)");
	}

	return entry.pso.Get();
}


OutputSignatureID D3D12Renderer::GetOutputSignatureId(DX12_OutputSignature outputSig)
{
	using namespace DataHub;

	for (OutputSignatureID i = 1; i < outputSigCount; i++)
	{
		if (outputSigIds[i].RTFormats == outputSig.RTFormats
			&& outputSigIds[i].RTBlendNDSFormat == outputSig.RTBlendNDSFormat)
			return i;
	}

	if (outputSigCount >= OUTPUT_SIGNATURE_MAX)
	{
		DebugReport(DataHub::RenderReport_Limit, L"[Sig] 출력 시그니처 꽉참 (OUTPUT_SIGNATURE_MAX 늘려야 함, 해당 액션 스킵됨)");
		return OUTPUT_SIGNATURE_INVALID;
	}


	const OutputSignatureID newId = outputSigCount;
	outputSigCount++;

	outputSigIds[newId] = outputSig;

	const DX12_OutputSignatureDesc desc = UnpackOutputSignature(outputSig);
	array<bool, RENDER_TARGET_MAX>& blendable = outputSigBlendableMasks[newId];

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		if (desc.rtFormats[r] == DXGI_FORMAT_UNKNOWN)
		{
			blendable[r] = false;
			continue;
		}

		blendable[r] = IsFormatBlendable(desc.rtFormats[r]);

		if (!blendable[r]
			&& desc.rtBlendOpts[r] != Action_Output_BlendOption_ByBlendType
			&& desc.rtBlendOpts[r] != Action_Output_BlendOption_ForceNone)
			DebugReport(DataHub::RenderReport_Contract, L"[Sig] 블렌드 안되는 포맷 칸에 Force 블렌드 (None 으로 대체됨)");
	}

	return newId;
}


bool D3D12Renderer::IsFormatBlendable(DXGI_FORMAT format)
{

	const uint8_t key = (uint8_t)format;

	if (formatBlendableCache[key] == 0)
	{
		ID3D12Device* device = D3D12Core::getInstance().GetDevice();

		D3D12_FEATURE_DATA_FORMAT_SUPPORT support = {};
		support.Format = format;

		HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support));

		const bool blendable = SUCCEEDED(hr) && (support.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE) != 0;

		formatBlendableCache[key] = blendable ? 2 : 1;
	}

	return formatBlendableCache[key] == 2;
}

#pragma endregion

#pragma endregion

}

