#include "D3D12Renderer.h"


namespace Hakko
{

#pragma region Helper

inline static UINT ConvertReleaseListToMask(std::vector<uint16_t>& list, std::vector<bool>& mask, uint16_t maskSize)
{
	mask.resize(maskSize, false);

	UINT skipped = 0;
	for (auto id : list)
	{
		if (id >= maskSize)
		{
			skipped++;
			continue;
		}
		mask[id] = true;
	}
	return skipped;
}


inline static bool HasUploadWork()
{
	using namespace Hakko::DataHub;

	const bool upload = (flag_updateResources & RTBCMD_Upload_Resource) != 0
		&& (!uploadMeshList.empty() || !uploadTextureList.empty() || !uploadCSBufferList.empty() || !linkCSBufferAsMeshList.empty());
	const bool update = (flag_updateResources & RTBCMD_Update_Resource) != 0
		&& (!updateMeshList.empty() || !updateTextureList.empty() || !updateCSBufferList.empty());

	return upload || update;
}


inline bool IsEmpty(const DataHub::Layer_Viewport& v)
{
    return v.width <= 0.0f || v.height <= 0.0f;
}

inline bool IsEmpty(const DataHub::Action_Viewport& v)
{
    return v.width <= 0.0f || v.height <= 0.0f;
}

inline bool IsEmpty(const DataHub::Action_Rect& v)
{
    return v.width <= 0 || v.height <= 0;
}

inline bool IsEmpty(const DataHub::Action_Output& v)
{
    if (v.useDS)
        return false;

    for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
    {
        if (v.useRT[r])
            return false;
    }
    return true;
}

inline D3D12_VIEWPORT ToD3D12Viewport(const DataHub::Layer_Viewport& v)
{
    const bool badDepth = !(v.maxZ > v.minZ);
    return D3D12_VIEWPORT{ v.originX, v.originY, v.width, v.height, badDepth ? 0.0f : v.minZ, badDepth ? 1.0f : v.maxZ };
}

inline D3D12_RECT ToD3D12Rect(const DataHub::Layer_Viewport& v)
{
    return D3D12_RECT{
        static_cast<LONG>(v.originX),
        static_cast<LONG>(v.originY),
        static_cast<LONG>(v.originX + v.width),
        static_cast<LONG>(v.originY + v.height)
    };
}

inline D3D12_VIEWPORT ToD3D12Viewport(const DataHub::Action_Viewport& v)
{
    const bool badDepth = !(v.maxZ > v.minZ);
    return D3D12_VIEWPORT{ v.originX, v.originY, v.width, v.height, badDepth ? 0.0f : v.minZ, badDepth ? 1.0f : v.maxZ };
}

inline D3D12_RECT ToD3D12Rect(const DataHub::Action_Viewport& v)
{
    return D3D12_RECT{
        static_cast<LONG>(v.originX),
        static_cast<LONG>(v.originY),
        static_cast<LONG>(v.originX + v.width),
        static_cast<LONG>(v.originY + v.height)
    };
}

inline D3D12_RECT ToD3D12Rect(const DataHub::Action_Rect& v)
{
    return D3D12_RECT{
        static_cast<LONG>(v.originX),
        static_cast<LONG>(v.originY),
        static_cast<LONG>(v.originX + v.width),
        static_cast<LONG>(v.originY + v.height)
    };
}


#pragma region Action

inline static void ResolveActionViewport(const DataHub::RTB_Action& src, const DataHub::Layer_Viewport& layerVp,
	D3D12_VIEWPORT& outVp, D3D12_RECT& outRect)
{
	const bool vpEmpty = IsEmpty(src.viewport);

	outVp = vpEmpty ? ToD3D12Viewport(layerVp) : ToD3D12Viewport(src.viewport);

	if (!IsEmpty(src.rect))
		outRect = ToD3D12Rect(src.rect);
	else
		outRect = vpEmpty ? ToD3D12Rect(layerVp) : ToD3D12Rect(src.viewport);
}

inline static void FillRenderActionBase(DX12_RenderAction& action, DX12_RenderActionType type, const DataHub::RTB_Action& src,
	const DataHub::Layer_Viewport& layerVp)
{

	action.actionType = type;
	action.builtinType = (DataHub::BuiltinActionType)0;
	action.renderIdType = DataHub::RenderIdType_None;
	action.frontNodeIdx.fill(0);

	ResolveActionViewport(src, layerVp, action.viewport, action.scissorRect);

	action.outputSigId = OUTPUT_SIGNATURE_INVALID;

	action.slotIds.fill(0);
	action.slotFlags = 0;
	action.srInstantIdx.fill(TEX_INSTANT_NONE);
	action.copySrcInstantIdx.fill(TEX_INSTANT_NONE);
	action.rtLoadOpts.fill(DataHub::Action_Output_LoadOption_Default);
	action.dsLoadOpt = DataHub::Action_Output_LoadOption_Default;

	for (uint8_t i = 0; i < RENDER_TARGET_MAX; i++)
	{
		if (src.output.useRT[i])
		{
			SetOutTex(action, i, src.output.RTTex[i]);
			action.rtLoadOpts[i] = src.output.loadOpt[i];
			if (src.output.RTTex[i] == 0)
				SetOutInstant(action, i, TEX_INSTANT_WANT);
		}
	}
	if (src.output.useDS)
	{
		SetOutTex(action, INSTANT_SLOT_DS, src.output.DSTex);
		action.dsLoadOpt = src.output.dsLoadOpt;
		if (src.output.DSTex == 0)
			SetOutInstant(action, INSTANT_SLOT_DS, TEX_INSTANT_WANT);
	}

	action.dsReadOnly = src.output.depthOpt == DataHub::Action_Output_DepthOption_ForceDepthRead;

	action.outputLayout = src.output.outputLayout;
	{
		uint8_t liveMask = 0;
		for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
		{
			if (src.output.useRT[r])
				liveMask |= (uint8_t)(1 << r);
		}

		const uint8_t needMask = DataHub::OutputLayoutTargetMask(action.outputLayout);
		bool ok = (needMask & liveMask) == needMask;

		if (action.outputLayout == DataHub::OutputLayout_DepthOnly && !src.output.useDS)
			ok = false;

		if (!ok)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Read] 출력 레이아웃이 요구하는 칸이 안 켜져 있음 (None 으로 강등)");
			action.outputLayout = DataHub::OutputLayout_None;
		}
	}

	action.actionDataTid = src.actionDataBoxTypeId;
	if (action.actionDataTid != 0 && DataHub::DataBoxCustomizer::GetProvider(action.actionDataTid) == nullptr)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[Read] 액션 데이터 tid 에 프로바이더 없음 (무시됨)");
		action.actionDataTid = 0;
	}
	action.actionRenderData = {};

	action.dataTids.fill(0);
}

inline static void FillRenderAction(DX12_RenderAction& action, const DataHub::RTB_ActionDraw& src, const DataHub::Layer_Viewport& layerVp)
{
	FillRenderActionBase(action, DX12_RenderActionType_Draw, src, layerVp);
	action.renderIdType = src.renderIdType;
	action.dataTids = src.dataBoxTypeIds;

	bool sbTaken = false;

	for (size_t k = 0; k < DRAWREQ_DATABOX_MAX; k++)
	{
		const DataHub::DataBoxTID tid = action.dataTids[k];

		if (tid == 0)
			continue;

		if (tid == DataHub::DataBoxType_CSBuffer)
		{
			if (sbTaken)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Read] 컴퓨트 버퍼 칸은 액션당 하나 (추가 칸 무시됨)");
				action.dataTids[k] = 0;
			}
			sbTaken = true;
			continue;
		}

		if (DataHub::DataBoxCustomizer::GetProvider(tid) == nullptr)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Read] 드로우 데이터 칸 tid 에 프로바이더 없음 (칸 무시됨)");
			action.dataTids[k] = 0;
		}
	}
}

inline static void FillRenderAction(DX12_RenderAction& action, const DataHub::RTB_ActionEdit& src, const DataHub::Layer_Viewport& layerVp)
{
	FillRenderActionBase(action, DX12_RenderActionType_Edit, src, layerVp);
}

inline static void FillRenderAction(DX12_RenderAction& action, const DataHub::RTB_ActionCompute& src, const DataHub::Layer_Viewport& layerVp)
{
	FillRenderActionBase(action, DX12_RenderActionType_Compute, src, layerVp);

	bool outputSet = action.slotFlags != 0;
	for (uint8_t s = 0; s < INSTANT_SLOT_MAX && !outputSet; s++)
		outputSet = action.slotIds[s] != 0;

	if (outputSet)
		DebugReport(DataHub::RenderReport_Contract, L"[Read] 컴퓨트 액션의 output 은 무시됨 (쓰기는 writes 로)");

	action.slotIds.fill(0);
	action.slotFlags = 0;

	for (size_t k = 0; k < COMPUTE_BINDING_MAX; k++)
	{
		const DataHub::ActionCompute_Binding& write = src.writes[k];
		const DataHub::ActionCompute_Binding& read = src.reads[k];

		if (write.type != DataHub::ActionCompute_Binding_Type_None)
			SetCSBinding(action, CS_SLOT_WRITE0 + k, write.resId, write.type == DataHub::ActionCompute_Binding_Type_Texture);

		if (read.type != DataHub::ActionCompute_Binding_Type_None)
			SetCSBinding(action, CS_SLOT_READ0 + k, read.resId, read.type == DataHub::ActionCompute_Binding_Type_Texture);
	}
}

inline static void FillRenderAction(DX12_RenderAction& action, const DataHub::RTB_ActionBuiltin& src, const DataHub::Layer_Viewport& layerVp)
{
	FillRenderActionBase(action, DX12_RenderActionType_Builtin, src, layerVp);
	action.builtinType = src.builtinType;
}


inline static void LinkActionNode(DX12_RenderLayer& layer, uint32_t node, uint32_t slot, uint32_t frontNode)
{
	const DX12_RenderActionNode& front = layer.renderActionNodeList[frontNode];
	DX12_RenderActionNode& dst = layer.renderActionNodeList[node];

	layer.renderActionList[node].frontNodeIdx[slot] = frontNode;
	dst.frontIds[slot] = front.actionId;

	if (dst.nodeDepth <= front.nodeDepth)
		dst.nodeDepth = front.nodeDepth + 1;
}

inline static void LinkActionNodeFronts(DX12_RenderLayer& layer, uint32_t node, const std::array<ActionID, RENDER_ACTION_NODE_MAX>* frontTable, uint32_t* nodeOfId)
{
	const ActionID id = layer.renderActionNodeList[node].actionId;

	for (uint32_t k = 0; k < RENDER_ACTION_NODE_MAX; k++)
	{
		ActionID frontId = frontTable[id][k];

		while (frontId != 0 && nodeOfId[frontId] == 0)
			frontId = frontTable[frontId][0];

		LinkActionNode(layer, node, k, nodeOfId[frontId]);
	}

	nodeOfId[id] = node;
}


#if RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_BUILTIN

static std::array<ActionID, RENDER_ACTION_NODE_MAX> builtinFrontTables[2][DX12_BuiltinActionId_Count];
static bool builtinFrontTablesReady = false;

inline static void BuildBuiltinFrontTables()
{
	for (uint32_t t = 0; t < 2; t++)
	{
		const bool deferred = t == 1;
		std::array<ActionID, RENDER_ACTION_NODE_MAX>* table = builtinFrontTables[t];

		for (uint32_t id = 0; id < DX12_BuiltinActionId_Count; id++)
			table[id].fill(0);

		table[DX12_BuiltinActionId_AO][0] = deferred ? DX12_BuiltinActionId_GBuffer : DX12_BuiltinActionId_EarlyZ;
		table[DX12_BuiltinActionId_AOBlur][0] = DX12_BuiltinActionId_AO;

		table[DX12_BuiltinActionId_OPQ][0] = DX12_BuiltinActionId_EarlyZ;
		table[DX12_BuiltinActionId_OPQ][1] = DX12_BuiltinActionId_AOBlur;
		table[DX12_BuiltinActionId_Lighting][0] = DX12_BuiltinActionId_GBuffer;
		table[DX12_BuiltinActionId_Lighting][1] = DX12_BuiltinActionId_AOBlur;

		table[DX12_BuiltinActionId_Background][0] = deferred ? DX12_BuiltinActionId_Lighting : DX12_BuiltinActionId_OPQ;
		table[DX12_BuiltinActionId_TP][0] = DX12_BuiltinActionId_Background;

		table[DX12_BuiltinActionId_PP0][0] = DX12_BuiltinActionId_TP;
		for (uint32_t i = 1; i < RENDER_BUILTIN_PP_MAX; i++)
			table[DX12_BuiltinActionId_PP0 + i][0] = DX12_BuiltinActionId_PP0 + i - 1;

		table[DX12_BuiltinActionId_SC][0] = DX12_BuiltinActionId_PP0 + RENDER_BUILTIN_PP_MAX - 1;
		table[DX12_BuiltinActionId_Debug][0] = DX12_BuiltinActionId_SC;
	}

	builtinFrontTablesReady = true;
}

#endif

#pragma endregion


#pragma region Barrier

static const D3D12_RESOURCE_STATES WRITE_STATE_MASK =
	D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_UNORDERED_ACCESS |
	D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_COPY_DEST |
	D3D12_RESOURCE_STATE_STREAM_OUT | D3D12_RESOURCE_STATE_RESOLVE_DEST;

inline static D3D12_RESOURCE_STATES GetLoadedHomeState(DX12_TextureUsage usage)
{
	switch (usage)
	{
	case DX12_TextureUsage_DSV:
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	case DX12_TextureUsage_UAV:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	default:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
}

inline static D3D12_RESOURCE_STATES GetInstantBaseState(bool isDS)
{
	if (isDS)
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

inline static D3D12_RESOURCE_STATES GetInstantReadState(DX12_RenderActionType type, bool isDS)
{
	if (type == DX12_RenderActionType_Compute)
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	if (isDS)
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}


inline static bool IsStateCovered(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	if (before == after)
		return true;

	return (before & WRITE_STATE_MASK) == 0 && (before & after) == after;
}


inline static void TransitionInstant(std::vector<DX12_ActionBarrier>& barriers, DX12_BarrierTarget target, uint16_t instIdx,
	D3D12_RESOURCE_STATES& state, D3D12_RESOURCE_STATES after)
{
	if (IsStateCovered(state, after))
		return;

	barriers.push_back(DX12_ActionBarrier{instIdx, target, state, after});
	state = after;
}

inline static void TransitionLoaded(DX12_RenderAction& action, TexID texId, DX12_TextureUsage usage, D3D12_RESOURCE_STATES use)
{
	const D3D12_RESOURCE_STATES home = GetLoadedHomeState(usage);

	if (IsStateCovered(home, use))
		return;

	action.beginBarriers.push_back(DX12_ActionBarrier{texId, DX12_BarrierTarget_Loaded, home, use});
	action.endBarriers.push_back(DX12_ActionBarrier{texId, DX12_BarrierTarget_Loaded, use, home});
}

inline static void TransitionBuffer(DX12_RenderAction& action, CSBufferID csBufferId, D3D12_RESOURCE_STATES use)
{
	if (IsStateCovered(CSBUFFER_HOME_STATE, use))
		return;

	action.beginBarriers.push_back(DX12_ActionBarrier{csBufferId, DX12_BarrierTarget_Buffer, CSBUFFER_HOME_STATE, use});
	action.endBarriers.push_back(DX12_ActionBarrier{csBufferId, DX12_BarrierTarget_Buffer, use, CSBUFFER_HOME_STATE});
}


inline static bool IsFrontAction(const DX12_RenderAction& action, uint32_t idx)
{
	for (uint32_t f = 0; f < RENDER_ACTION_NODE_MAX; f++)
	{
		if (action.frontNodeIdx[f] != 0 && action.frontNodeIdx[f] == idx)
			return true;
	}
	return false;
}

#pragma endregion


#pragma region DrawRequest

inline static void ReadActionDrawRequests(const DataHub::RTB_ActionDraw& src, DX12_RenderLayer& layer, uint32_t node)
{
	const size_t reqCount = src.drawReqList.size();

	auto& requestList = layer.actionRenderRequestList[node];

	requestList.resize(reqCount);

	for (size_t i = 0; i < reqCount; i++)
	{
		requestList[i].renderId = src.drawReqList[i].renderId;
		requestList[i].requestIdx = (uint32_t)i;
	}

	layer.actionSortedRenderDataList[node].resize(reqCount);
}

inline static void ReadActionEditRequest(const DataHub::RTB_ActionEdit& src, DX12_RenderLayer& layer, uint32_t node)
{
	auto& requestList = layer.actionRenderRequestList[node];

	requestList.resize(1);

	requestList[0].renderId = ((RenderID)src.drawProp << 48) | ((RenderID)src.texProp << 32) ;
	requestList[0].requestIdx = 0;

	layer.actionSortedRenderDataList[node].assign(1, {});
}

inline static void ReadActionComputeRequest(const DataHub::RTB_ActionCompute& src, DX12_RenderLayer& layer, uint32_t node)
{
	auto& requestList = layer.actionRenderRequestList[node];

	requestList.resize(1);

	uint32_t dispatch[3] = { src.dispatchX, src.dispatchY, src.dispatchZ };

	for (uint32_t& d : dispatch)
	{
		if (d > D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Read] 컴퓨트 디스패치 수가 한 차원 최댓값 (65535) 초과 (잘라냄)");
			d = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
		}
	}

	ShaderID csId = src.csId;

	if (csId > UINT16_MAX)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[Read] 컴퓨트 셰이더 id 가 16비트 밖 (디스패치 안함)");
		csId = 0;
	}

	requestList[0].renderId = ((RenderID)csId << 48) | ((RenderID)dispatch[0] << 32) | ((RenderID)dispatch[1] << 16) | (RenderID)dispatch[2];
	requestList[0].requestIdx = 0;

	layer.actionSortedRenderDataList[node].assign(1, {});
}

#pragma endregion

#pragma endregion



#pragma region ReadMessage

void D3D12Renderer::ReadTransferBox()
{
	// --- 패스 / 리소스 관련 요청 (프레임 별) ---
	{
		vector<ComPtr<ID3D12Resource>> uploadBufferList;
		const bool gpuWork = HasUploadWork();

		if (gpuWork)
			BeginUploadCommands();

		ReadUploadRequest(uploadBufferList);
		ReadUpdateRequest(uploadBufferList);

		if (gpuWork)
			FlushUploadCommands();
	}

	// --- 그리기 관련 요청 (레이어/패스 별) ---
	ReadLayerOption();

	if (renderFrames[cpuFrameIndex].frameInvalidated)
		return;


	ReadActionNode();
}


void D3D12Renderer::ReadLayerOption()
{
	using namespace DataHub;

	auto& renderFrame = renderFrames[cpuFrameIndex];

	for (size_t i = 0; i < layerCount; i++)
	{
		auto bgTexIdNew = LayerList[i].backGround.bgTexId;
		const auto& color = LayerList[i].backGround.color;

		if (bgTexIdNew >= textureList.size())
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Layer] 배경 texId 범위 밖 (무시됨)");
			bgTexIdNew = 0;
		}
		else if (bgTexIdNew < releaseTexIdMask.size() && releaseTexIdMask[bgTexIdNew])
        {
            DebugReport(DataHub::RenderReport_Contract, L"[Layer] 삭제된 텍스처를 배경으로 지정 (무시됨)");
            bgTexIdNew = 0;
        }
	    else if (bgTexIdNew != 0)
		{
			MarkOrBuildSingleTexture(bgTexIdNew);
            renderFrame.layerList[i].bgIsCube = textureList[bgTexIdNew].IsCube;

			renderFrame.layerList[i].bgColor[0] = color[0];
			renderFrame.layerList[i].bgColor[1] = color[1];
			renderFrame.layerList[i].bgColor[2] = color[2];
			renderFrame.layerList[i].bgColor[3] = color[3];
		}

		renderFrame.layerList[i].bgTexId = bgTexIdNew;


		auto envTexIdNew = LayerList[i].envTexId;

		if (envTexIdNew >= textureList.size())
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Layer] 환경맵 texId 범위 밖 (무시됨)");
			envTexIdNew = 0;
		}
		else if (envTexIdNew < releaseTexIdMask.size() && releaseTexIdMask[envTexIdNew])
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Layer] 삭제된 텍스처를 환경맵으로 지정 (무시됨)");
			envTexIdNew = 0;
		}
		else if (envTexIdNew != 0)
		{
			if (!textureList[envTexIdNew].IsCube)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Layer] 환경맵이 큐브맵이 아님 (무시됨)");
				envTexIdNew = 0;
			}
			else
				MarkOrBuildSingleTexture(envTexIdNew);
		}

		renderFrame.layerList[i].envTexId = envTexIdNew;


		renderFrame.layerList[i].layerDataTid = LayerList[i].layerDataBoxTid;
		if (renderFrame.layerList[i].layerDataTid != 0 && DataBoxCustomizer::GetProvider(renderFrame.layerList[i].layerDataTid) == nullptr)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Read] 레이어 데이터 tid 에 프로바이더 없음 (무시됨)");
			renderFrame.layerList[i].layerDataTid = 0;
		}
	}
	
}

void D3D12Renderer::ReadActionNode()
{
	using namespace DataHub;

	auto& renderFrame = renderFrames[cpuFrameIndex];

	
    #if RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_BUILTIN

	renderFrame.rtInstantNeedCount = 0;
	renderFrame.dsInstantNeedCount = 0;

	const size_t BUILTIN_ACTION_NODE_MAX = DX12_BuiltinActionId_Count;

	if (!builtinFrontTablesReady)
		BuildBuiltinFrontTables();

	for (size_t l = 0; l < layerCount; l++)
	{
		auto& layerDesc = LayerList[l];
		auto& layer = renderFrame.layerList[l];

		Layer_Viewport layerVp = layerDesc.viewport;
		if (IsEmpty(layerVp))
			layerVp = Layer_Viewport{ 0.0f, 0.0f, (float)bbWidth, (float)bbHeight };

		auto& nodeList = layer.renderActionNodeList;
		auto& actionList = layer.renderActionList;
		auto& requestList = layer.actionRenderRequestList;
		auto& sortedList = layer.actionSortedRenderDataList;

		nodeList.resize(BUILTIN_ACTION_NODE_MAX);
		actionList.resize(BUILTIN_ACTION_NODE_MAX);
		requestList.resize(BUILTIN_ACTION_NODE_MAX);
		sortedList.resize(BUILTIN_ACTION_NODE_MAX);

		uint32_t n = 0;


		nodeList[n] = DX12_RenderActionNode{};
		{
			DX12_RenderAction& base = actionList[n];
			base = {};
			base.srInstantIdx.fill(TEX_INSTANT_NONE);
			base.copySrcInstantIdx.fill(TEX_INSTANT_NONE);
			base.outputSigId = OUTPUT_SIGNATURE_INVALID;
		}
		requestList[n].clear();
		sortedList[n].clear();
		n++;


		bool deferred = layerDesc.pipelineType == Layer_PipelineType_Deferred;

		if (deferred)
		{
			DebugReport(RenderReport_Contract, L"[미지원] 디퍼드 파이프라인은 이 버전에서 막혀 있음 (Forward 로 처리)");
			deferred = false;
		}

		if (layerDesc.globalShadowTexId != 0 || layerDesc.localShadowTexId != 0)
			DebugReport(RenderReport_Contract, L"[미지원] 레이어 섀도우맵 칸은 이 버전에서 막혀 있음 (무시됨)");

		bool blockedPreCompute = false;
		bool blockedShadow = false;

		const std::array<ActionID, RENDER_ACTION_NODE_MAX>* frontTable = builtinFrontTables[deferred ? 1 : 0];

		uint32_t nodeOfId[DX12_BuiltinActionId_Count] = {};


		for (uint32_t i = 0; i < RENDER_BUILTIN_PRECOMPUTE_MAX; i++)
		{
			const RTB_ActionCompute& src = layerDesc.actionPreComputes[i];
			if (src.csId == 0)
				continue;

			blockedPreCompute = true;
			continue;

			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_PreCompute0 + i};
			FillRenderAction(actionList[n], src, layerVp);
			ReadActionComputeRequest(src, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		{
			const RTB_ActionDraw& src = deferred ? layerDesc.actionGBuffer : layerDesc.actionEarlyZ;

			if (!IsEmpty(src.output))
			{
				nodeList[n] = DX12_RenderActionNode{1, {0}, deferred ? DX12_BuiltinActionId_GBuffer : DX12_BuiltinActionId_EarlyZ};
				FillRenderAction(actionList[n], src, layerVp);
				ReadActionDrawRequests(src, layer, n);
				LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
				n++;
			}
		}

		if (layerDesc.actionAO.drawProp != 0)
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_AO};
			FillRenderAction(actionList[n], layerDesc.actionAO, layerVp);
			ReadActionEditRequest(layerDesc.actionAO, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		if (layerDesc.actionAOBlur.drawProp != 0)
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_AOBlur};
			FillRenderAction(actionList[n], layerDesc.actionAOBlur, layerVp);
			ReadActionEditRequest(layerDesc.actionAOBlur, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		for (uint32_t i = 0; i < RENDER_BUILTIN_CSM_MAX; i++)
		{
			const RTB_ActionDraw& src = layerDesc.actionGlobalSDWs[i];
			if (IsEmpty(src.output))
				continue;

			blockedShadow = true;
			continue;

			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_GlobalSDW0 + i};
			FillRenderAction(actionList[n], src, layerVp);
			ReadActionDrawRequests(src, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		for (uint32_t i = 0; i < RENDER_LIGHT_MAX; i++)
		{
			const RTB_ActionDraw& src = layerDesc.actionLocalSDWs[i];
			if (IsEmpty(src.output))
				continue;

			blockedShadow = true;
			continue;

			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_LocalSDW0 + i};
			FillRenderAction(actionList[n], src, layerVp);
			ReadActionDrawRequests(src, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		if (deferred)
		{
			const RTB_ActionEdit& src = layerDesc.actionLighting;

			if (src.drawProp != 0)
			{
				nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_Lighting};
				FillRenderAction(actionList[n], src, layerVp);
				ReadActionEditRequest(src, layer, n);
				LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
				n++;
			}
		}
		else
		{
			const RTB_ActionDraw& src = layerDesc.actionOPQ;

			if (!IsEmpty(src.output))
			{
				nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_OPQ};
				FillRenderAction(actionList[n], src, layerVp);
				ReadActionDrawRequests(src, layer, n);
				LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
				n++;
			}
		}

		if (!IsEmpty(layerDesc.actionBackground.output))
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_Background};
			FillRenderAction(actionList[n], layerDesc.actionBackground, layerVp);
			requestList[n].clear();
			sortedList[n].clear();
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		if (!IsEmpty(layerDesc.actionTP.output))
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_TP};
			FillRenderAction(actionList[n], layerDesc.actionTP, layerVp);
			ReadActionDrawRequests(layerDesc.actionTP, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		for (uint32_t i = 0; i < RENDER_BUILTIN_PP_MAX; i++)
		{
			const RTB_ActionEdit& src = layerDesc.actionPPs[i];
			if (src.drawProp == 0)
				continue;

			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_PP0 + i};
			FillRenderAction(actionList[n], src, layerVp);
			ReadActionEditRequest(src, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		if (!IsEmpty(layerDesc.actionSC.output))
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_SC};
			FillRenderAction(actionList[n], layerDesc.actionSC, layerVp);
			ReadActionDrawRequests(layerDesc.actionSC, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		if (!IsEmpty(layerDesc.actionDebug.output))
		{
			nodeList[n] = DX12_RenderActionNode{1, {0}, DX12_BuiltinActionId_Debug};
			FillRenderAction(actionList[n], layerDesc.actionDebug, layerVp);
			ReadActionDrawRequests(layerDesc.actionDebug, layer, n);
			LinkActionNodeFronts(layer, n, frontTable, nodeOfId);
			n++;
		}

		nodeList.resize(n);
		actionList.resize(n);
		requestList.resize(n);
		sortedList.resize(n);

		if (blockedPreCompute)
			DebugReport(RenderReport_Contract, L"[미지원] PreCompute 액션은 이 버전에서 막혀 있음 (노드 건너뜀)");
		if (blockedShadow)
			DebugReport(RenderReport_Contract, L"[미지원] 섀도우 액션은 이 버전에서 막혀 있음 (노드 건너뜀)");


		BuildActionGraph(layer, layerDesc);


		layer.rtInstantOffset = renderFrame.rtInstantNeedCount;
		layer.dsInstantOffset = renderFrame.dsInstantNeedCount;

		renderFrame.rtInstantNeedCount += layer.rtInstantCount;
		renderFrame.dsInstantNeedCount += layer.dsInstantCount;
	}

	
	ResizeTextureInstant();
	
	
    #elif RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_CUSTOM

	#endif
}

void D3D12Renderer::ReadUploadRequest(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;


	if (flag_updateResources & RTBCMD_Upload_Resource)
	{
		UploadMesh(uploadBufferList);
		UploadTexture(uploadBufferList);
		UploadCSBuffer(uploadBufferList);
		LinkCSBufferMesh(uploadBufferList);
		GetShaderInfo();
	}

	if (flag_updateResources & RTBCMD_Upload_Prop)
	{
		BuildPSOBody();
		BuildCombinedTexture();
	}

	flag_updateResources &= ~(uint32_t)RTBCMD_Upload;
}

void D3D12Renderer::ReadUpdateRequest(vector<ComPtr<ID3D12Resource>>& uploadBufferList)
{
	using namespace DataHub;


	if (flag_updateResources & RTBCMD_Update_Resource)
	{
		UpdateMesh(uploadBufferList);
		UpdateTexture(uploadBufferList);
		UpdateCSBuffer(uploadBufferList);
	}

	if (flag_updateResources & RTBCMD_Update_Prop)
	{
		UpdatePSOBody();
		UpdateCombinedTexture();
	}

	flag_updateResources &= ~(uint32_t)RTBCMD_Update;
}

void D3D12Renderer::ReadReleaseRequest()
{
	using namespace DataHub;

	if (flag_updateResources & RTBCMD_Release_Resource)
	{
		if (!ReleaseMeshList.empty())
		{
			if (ConvertReleaseListToMask(ReleaseMeshList, releaseMeshIdMask, (uint16_t)meshList.size()) > 0)
				DebugReport(RenderReport_Contract, L"[Release] meshId 범위 밖 (업로드 전 해제?), 무시");
			meshReleaseCount = FRAME_COUNT;
			ReleaseMeshList.clear();
		}

		if (!ReleaseTextureList.empty())
		{
			if (ConvertReleaseListToMask(ReleaseTextureList, releaseTexIdMask, (uint16_t)textureList.size()) > 0)
				DebugReport(RenderReport_Contract, L"[Release] texId 범위 밖 (업로드 전 해제?), 무시");
			texReleaseCount = FRAME_COUNT;
			ReleaseTextureList.clear();
		}

		if (!ReleaseCSBufferList.empty())
		{
			if (ConvertReleaseListToMask(ReleaseCSBufferList, releaseCSBufferIdMask, (uint16_t)csBufferList.size()) > 0)
				DebugReport(RenderReport_Contract, L"[Release] csBufferId 범위 밖 (업로드 전 해제?), 무시");
			csBufferReleaseCount = FRAME_COUNT;
			ReleaseCSBufferList.clear();
		}
	}

	if (flag_updateResources & RTBCMD_Release_Prop)
	{
		if (!ReleaseDrawPropList.empty())
		{
			if (ConvertReleaseListToMask(ReleaseDrawPropList, releasePSOIdMask, (uint16_t)psoGeneratorList.size()) > 0)
				DebugReport(RenderReport_Contract, L"[Release] drawPropId 범위 밖 (빌드 전 해제?), 무시");
			psoReleaseCount = FRAME_COUNT;
			ReleaseDrawPropList.clear();
		}

		if (!ReleaseTexPropList.empty())
		{
			if (ConvertReleaseListToMask(ReleaseTexPropList, releaseCTexIdMask, (uint16_t)combinedTextureList.size()) > 0)
				DebugReport(RenderReport_Contract, L"[Release] texPropId 범위 밖 (빌드 전 해제?), 무시");
			cTexReleaseCount = FRAME_COUNT;
			ReleaseTexPropList.clear();
		}
	}

	flag_updateResources &= ~(uint32_t)RTBCMD_Release;
}


#pragma region Helper

DX12_OutputSignature D3D12Renderer::MakeOutputSignature(const DX12_RenderAction& action, const DataHub::Action_Output& output)
{
	using namespace DataHub;

	DX12_OutputSignatureDesc desc = {};

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		const TexID texId = OutTexId(action, r);

		if (texId != 0)
		{
			if (texId < textureList.size())
				format = textureList[texId].Format;
		}
		else if (IsOutInstant(action, r))
		{
			format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		}

		desc.rtFormats[r] = format;

		if (format != DXGI_FORMAT_UNKNOWN)
			desc.rtBlendOpts[r] = output.blendOpt[r];
		else
			desc.rtBlendOpts[r] = Action_Output_BlendOption_ByBlendType;
	}

	DXGI_FORMAT dsFormat = DXGI_FORMAT_UNKNOWN;
	const TexID dsTexId = OutTexId(action, INSTANT_SLOT_DS);

	if (dsTexId != 0)
	{
		if (dsTexId < textureList.size())
			dsFormat = GetDSVFormat(textureList[dsTexId].Format);
	}
	else if (IsOutInstant(action, INSTANT_SLOT_DS))
	{
		dsFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	}

	desc.dsFormat = dsFormat;

	if (dsFormat != DXGI_FORMAT_UNKNOWN)
		desc.dsOpt = output.depthOpt;
	else
		desc.dsOpt = Action_Output_DepthOption_ByDepthUsage;

	desc.sampleCount = 1;

	desc.outputLayout = action.outputLayout;

	return PackOutputSignature(desc);
}


const DataHub::RTB_Action& D3D12Renderer::GetActionSource(const DataHub::RTB_Layer& layerDesc, ActionID id)
{
#if RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_BUILTIN
	switch (id)
	{
	case DX12_BuiltinActionId_EarlyZ:		return layerDesc.actionEarlyZ;
	case DX12_BuiltinActionId_GBuffer:		return layerDesc.actionGBuffer;
	case DX12_BuiltinActionId_AO:			return layerDesc.actionAO;
	case DX12_BuiltinActionId_AOBlur:		return layerDesc.actionAOBlur;
	case DX12_BuiltinActionId_OPQ:			return layerDesc.actionOPQ;
	case DX12_BuiltinActionId_Lighting:		return layerDesc.actionLighting;
	case DX12_BuiltinActionId_Background:	return layerDesc.actionBackground;
	case DX12_BuiltinActionId_TP:			return layerDesc.actionTP;
	case DX12_BuiltinActionId_SC:			return layerDesc.actionSC;
	default:								break;
	}

	if (id >= DX12_BuiltinActionId_PreCompute0 && id < DX12_BuiltinActionId_EarlyZ)
		return layerDesc.actionPreComputes[id - DX12_BuiltinActionId_PreCompute0];
	if (id >= DX12_BuiltinActionId_GlobalSDW0 && id < DX12_BuiltinActionId_LocalSDW0)
		return layerDesc.actionGlobalSDWs[id - DX12_BuiltinActionId_GlobalSDW0];
	if (id >= DX12_BuiltinActionId_LocalSDW0 && id < DX12_BuiltinActionId_OPQ)
		return layerDesc.actionLocalSDWs[id - DX12_BuiltinActionId_LocalSDW0];
	if (id >= DX12_BuiltinActionId_PP0 && id < DX12_BuiltinActionId_SC)
		return layerDesc.actionPPs[id - DX12_BuiltinActionId_PP0];

	return layerDesc.actionDebug;
#else
	const auto& v = layerDesc.actionList[id];
	if (const auto* draw = std::get_if<DataHub::RTB_ActionDraw>(&v))
		return *draw;
	if (const auto* edit = std::get_if<DataHub::RTB_ActionEdit>(&v))
		return *edit;
	if (const auto* builtin = std::get_if<DataHub::RTB_ActionBuiltin>(&v))
		return *builtin;
	return std::get<DataHub::RTB_ActionCompute>(v);
#endif
}


void D3D12Renderer::BuildActionGraph(DX12_RenderLayer& layer, const DataHub::RTB_Layer& layerDesc)
{
	using namespace DataHub;

	auto& actionList = layer.renderActionList;
	const uint32_t actionCount = (uint32_t)actionList.size();

	if (layer.renderActionNodeList.size() != actionCount)
		DebugReport(DataHub::RenderReport_Invariant, L"[Graph] 노드 리스트랑 액션 리스트 갯수 다름");

	static vector<DX12_InstantLogical> rtInstantLogicalList;
	static vector<DX12_InstantLogical> dsInstantLogicalList;
	static vector<uint16_t> instantFreeList;
	static vector<D3D12_RESOURCE_STATES> rtInstantSimStateList;
	static vector<D3D12_RESOURCE_STATES> dsInstantSimStateList;

	rtInstantLogicalList.clear();
	dsInstantLogicalList.clear();


	#pragma region AssignInstant

	for (uint32_t i = 1; i < actionCount; i++)
	{
		DX12_RenderAction& action = actionList[i];

		uint16_t outClaim[INSTANT_SLOT_MAX];
		uint16_t srClaim[INSTANT_SLOT_MAX];
		uint16_t copySrc[INSTANT_SLOT_MAX];
		uint16_t inheritClaim[INSTANT_SLOT_MAX];
		bool srInherited[INSTANT_SLOT_MAX];

		for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
		{
			outClaim[s] = TEX_INSTANT_NONE;
			srClaim[s] = TEX_INSTANT_NONE;
			copySrc[s] = TEX_INSTANT_NONE;
			inheritClaim[s] = TEX_INSTANT_NONE;
			srInherited[s] = false;
		}


		for (uint32_t f = 0; f < RENDER_ACTION_NODE_MAX; f++)
		{
			const uint32_t frontIdx = action.frontNodeIdx[f];

			if (frontIdx == 0)
				continue;

			if (frontIdx >= i)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Graph] 선행 노드가 정렬상 뒤에 있음 / 자기 자신 (무시)");
				continue;
			}

			const DX12_RenderAction& front = actionList[frontIdx];
			const Action_Input& input = GetActionSource(layerDesc, layer.renderActionNodeList[i].actionId).inputs[f];

			for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
			{
				const bool isDS = s == INSTANT_SLOT_DS;
				auto& logicalList = isDS ? dsInstantLogicalList : rtInstantLogicalList;

				const Action_InputType type = isDS ? input.DSInputType : input.RTInputType[s];

				if ((type & Action_InputType_ClearSR) == 0 && front.srInstantIdx[s] != TEX_INSTANT_NONE && inheritClaim[s] == TEX_INSTANT_NONE)
					inheritClaim[s] = front.srInstantIdx[s];

				const TexID frontTexId = OutTexId(front, s);
				const uint16_t logicalIdx = OutInstantIdx(front, s);

				if (frontTexId != 0 || logicalIdx == TEX_INSTANT_NONE)
					continue;

				if (logicalIdx >= logicalList.size())
				{
					DebugReport(DataHub::RenderReport_Invariant, L"[Graph] 선행 출력 인스턴트 인덱스 범위 밖");
					continue;
				}

				const bool wantCopy = (type & Action_InputType_Copy) != 0;
				const bool wantRead = wantCopy || (type & Action_InputType_Convert) != 0;
				const bool detach = (type & Action_InputType_Detach) != 0;
				const bool wantOut = !detach && (!wantRead || wantCopy || (isDS && action.dsReadOnly));

				if (wantRead)
				{
					if (logicalList[logicalIdx].ownerActionIdx != frontIdx)
						DebugReport(DataHub::RenderReport_Contract, L"[Graph] 선행 출력에 다른 액션이 먼저 이어 그림 (형제 노드 순서에 따라 결과 달라짐)");

					if (srClaim[s] == TEX_INSTANT_NONE)
					{
						if (wantCopy)
						{
							srClaim[s] = (uint16_t)logicalList.size();
							copySrc[s] = logicalIdx;
							logicalList.push_back(DX12_InstantLogical{i, i, i, false, TEX_INSTANT_NONE});
						}
						else
						{
							srClaim[s] = logicalIdx;
						}
					}
					else if (srClaim[s] != logicalIdx && copySrc[s] != logicalIdx)
					{
						DebugReport(DataHub::RenderReport_Contract, L"[Graph] 여러 선행 출력이 같은 SR 칸으로 들어옴 (앞쪽 선행 노드 우선)");
					}
				}

				if (wantOut)
				{
					if (outClaim[s] == TEX_INSTANT_NONE)
						outClaim[s] = logicalIdx;
					else if (outClaim[s] != logicalIdx)
						DebugReport(DataHub::RenderReport_Contract, L"[Graph] 여러 선행 출력이 같은 출력 칸으로 들어옴 (앞쪽 선행 노드 우선)");
				}
			}
		}

		for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
		{
			if (srClaim[s] == TEX_INSTANT_NONE && inheritClaim[s] != TEX_INSTANT_NONE)
			{
				srClaim[s] = inheritClaim[s];
				srInherited[s] = true;
			}
		}


		for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
		{
			const bool isDS = s == INSTANT_SLOT_DS;
			const bool readOnly = isDS && action.dsReadOnly;
			auto& logicalList = isDS ? dsInstantLogicalList : rtInstantLogicalList;

			const TexID texId = OutTexId(action, s);
			const uint16_t outIdx = OutInstantIdx(action, s);

			if (texId != 0)
			{
				if (outClaim[s] != TEX_INSTANT_NONE)
					DebugReport(DataHub::RenderReport_Contract, L"[Graph] 로디드 출력이라 선행 인스턴트 이어 그리기 무시");
				continue;
			}

			if (outIdx != TEX_INSTANT_WANT)
				continue;

			if (outClaim[s] != TEX_INSTANT_NONE)
			{
				const uint16_t logicalIdx = outClaim[s];
				const DX12_InstantLogical& logical = logicalList[logicalIdx];

				if (srClaim[s] == logicalIdx && !readOnly)
				{
					if (!srInherited[s])
						DebugReport(DataHub::RenderReport_Contract, L"[Graph] 같은 인스턴트를 읽으면서 씀 (읽기 제외, 읽기 유지하려면 Copy 로)");
					srClaim[s] = TEX_INSTANT_NONE;
					srInherited[s] = false;
				}

				if (!IsFrontAction(action, logical.ownerActionIdx))
					DebugReport(DataHub::RenderReport_Contract, L"[Graph] 선행 노드 출력이 아닌 인스턴트에 이어 그림 (다른 액션이 이미 이어받음)");
				else if (logical.readAfterWrite && !readOnly)
					DebugReport(DataHub::RenderReport_Contract, L"[Graph] 다른 액션이 읽은 인스턴트에 이어 그림 (형제 노드 순서에 따라 결과 달라짐)");

				SetOutInstant(action, s, logicalIdx);
			}
			else
			{
				if (readOnly)
					DebugReport(DataHub::RenderReport_Contract, L"[Graph] 읽기전용 DS 인데 이어받을 DS 없음");

				SetOutInstant(action, s, (uint16_t)logicalList.size());
				logicalList.push_back(DX12_InstantLogical{i, i, i, false, TEX_INSTANT_NONE});
			}
		}


		for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
		{
			const bool isDS = s == INSTANT_SLOT_DS;
			auto& logicalList = isDS ? dsInstantLogicalList : rtInstantLogicalList;

			action.srInstantIdx[s] = srClaim[s];
			action.copySrcInstantIdx[s] = copySrc[s];

			if (srClaim[s] != TEX_INSTANT_NONE)
			{
				if (OutInstantIdx(action, s) != srClaim[s])
					logicalList[srClaim[s]].readAfterWrite = true;
				logicalList[srClaim[s]].lastUse = i;
			}

			if (copySrc[s] != TEX_INSTANT_NONE)
			{
				logicalList[copySrc[s]].readAfterWrite = true;
				logicalList[copySrc[s]].lastUse = i;
			}

			const uint16_t outIdx = OutInstantIdx(action, s);

			if (outIdx != TEX_INSTANT_NONE)
			{
				logicalList[outIdx].lastUse = i;
				logicalList[outIdx].ownerActionIdx = i;
				if (!(isDS && action.dsReadOnly))
					logicalList[outIdx].readAfterWrite = false;
			}
		}
	}

	#pragma endregion


	#pragma region AllocateInstant

	for (uint8_t k = 0; k < 2; k++)
	{
		auto& logicalList = (k == 0) ? rtInstantLogicalList : dsInstantLogicalList;
		const uint32_t logicalCount = (uint32_t)logicalList.size();

		uint16_t physicalCount = 0;
		uint32_t cursor = 0;
		instantFreeList.clear();

		for (uint32_t i = 1; i < actionCount; i++)
		{
			while (cursor < logicalCount && logicalList[cursor].firstUse == i)
			{
				if (!instantFreeList.empty())
				{
					logicalList[cursor].physicalIdx = instantFreeList.back();
					instantFreeList.pop_back();
				}
				else
				{
					logicalList[cursor].physicalIdx = physicalCount++;
				}

				cursor++;
			}

			for (uint32_t l = 0; l < cursor; l++)
			{
				if (logicalList[l].lastUse == i)
					instantFreeList.push_back(logicalList[l].physicalIdx);
			}
		}

		if (k == 0)
			layer.rtInstantCount = physicalCount;
		else
			layer.dsInstantCount = physicalCount;
	}

	for (uint32_t i = 1; i < actionCount; i++)
	{
		DX12_RenderAction& action = actionList[i];

		for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
		{
			if (IsOutInstant(action, r))
				action.slotIds[r] = rtInstantLogicalList[action.slotIds[r]].physicalIdx;

			if (action.srInstantIdx[r] != TEX_INSTANT_NONE)
				action.srInstantIdx[r] = rtInstantLogicalList[action.srInstantIdx[r]].physicalIdx;

			if (action.copySrcInstantIdx[r] != TEX_INSTANT_NONE)
				action.copySrcInstantIdx[r] = rtInstantLogicalList[action.copySrcInstantIdx[r]].physicalIdx;
		}

		if (IsOutInstant(action, INSTANT_SLOT_DS))
			action.slotIds[INSTANT_SLOT_DS] = dsInstantLogicalList[action.slotIds[INSTANT_SLOT_DS]].physicalIdx;

		if (action.srInstantIdx[INSTANT_SLOT_DS] != TEX_INSTANT_NONE)
			action.srInstantIdx[INSTANT_SLOT_DS] = dsInstantLogicalList[action.srInstantIdx[INSTANT_SLOT_DS]].physicalIdx;

		if (action.copySrcInstantIdx[INSTANT_SLOT_DS] != TEX_INSTANT_NONE)
			action.copySrcInstantIdx[INSTANT_SLOT_DS] = dsInstantLogicalList[action.copySrcInstantIdx[INSTANT_SLOT_DS]].physicalIdx;
	}

	#pragma endregion


	#pragma region Barrier

	rtInstantSimStateList.assign(layer.rtInstantCount, GetInstantBaseState(false));
	dsInstantSimStateList.assign(layer.dsInstantCount, GetInstantBaseState(true));

	for (uint32_t i = 1; i < actionCount; i++)
	{
		BuildActionBarriers(actionList[i], rtInstantSimStateList, dsInstantSimStateList);

		if (actionList[i].actionType == DX12_RenderActionType_Compute)
		{
			actionList[i].outputSigId = OUTPUT_SIGNATURE_INVALID;
		}
		else
		{
			const Action_Output& output = GetActionSource(layerDesc, layer.renderActionNodeList[i].actionId).output;
			actionList[i].outputSigId = GetOutputSignatureId(MakeOutputSignature(actionList[i], output));
		}
	}

	layer.finalBarriers.clear();

	for (uint16_t p = 0; p < layer.rtInstantCount; p++)
	{
		if (rtInstantSimStateList[p] != GetInstantBaseState(false))
			layer.finalBarriers.push_back(DX12_ActionBarrier{p, DX12_BarrierTarget_RTInstant, rtInstantSimStateList[p], GetInstantBaseState(false)});
	}

	for (uint16_t p = 0; p < layer.dsInstantCount; p++)
	{
		if (dsInstantSimStateList[p] != GetInstantBaseState(true))
			layer.finalBarriers.push_back(DX12_ActionBarrier{p, DX12_BarrierTarget_DSInstant, dsInstantSimStateList[p], GetInstantBaseState(true)});
	}

	#pragma endregion
}

void D3D12Renderer::BuildActionBarriers(DX12_RenderAction& action, vector<D3D12_RESOURCE_STATES>& rtInstantSimStateList, vector<D3D12_RESOURCE_STATES>& dsInstantSimStateList)
{
	const bool isCompute = action.actionType == DX12_RenderActionType_Compute;

	action.copyBarriers.clear();
	action.beginBarriers.clear();
	action.endBarriers.clear();


	#pragma region Copy

	for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		const uint16_t srcIdx = action.copySrcInstantIdx[s];

		if (srcIdx == TEX_INSTANT_NONE)
			continue;

		const uint16_t dstIdx = action.srInstantIdx[s];
		const bool isDS = s == INSTANT_SLOT_DS;
		auto& simStates = isDS ? dsInstantSimStateList : rtInstantSimStateList;
		const DX12_BarrierTarget target = isDS ? DX12_BarrierTarget_DSInstant : DX12_BarrierTarget_RTInstant;

		if (dstIdx == TEX_INSTANT_NONE || srcIdx >= simStates.size() || dstIdx >= simStates.size())
		{
			DebugReport(DataHub::RenderReport_Invariant, L"[Barrier] 복제 인스턴트 인덱스 이상 (복제 취소)");
			action.copySrcInstantIdx[s] = TEX_INSTANT_NONE;
			action.srInstantIdx[s] = TEX_INSTANT_NONE;
			continue;
		}

		TransitionInstant(action.copyBarriers, target, srcIdx, simStates[srcIdx], D3D12_RESOURCE_STATE_COPY_SOURCE);
		TransitionInstant(action.copyBarriers, target, dstIdx, simStates[dstIdx], D3D12_RESOURCE_STATE_COPY_DEST);
	}

	#pragma endregion


	#pragma region InputSR

	for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		const uint16_t instIdx = action.srInstantIdx[s];

		if (instIdx == TEX_INSTANT_NONE)
			continue;

		const bool isDS = s == INSTANT_SLOT_DS;
		auto& simStates = isDS ? dsInstantSimStateList : rtInstantSimStateList;

		if (instIdx >= simStates.size())
		{
			DebugReport(DataHub::RenderReport_Invariant, L"[Barrier] 읽기 인스턴트 인덱스 범위 밖");
			continue;
		}

		TransitionInstant(action.beginBarriers, isDS ? DX12_BarrierTarget_DSInstant : DX12_BarrierTarget_RTInstant,
			instIdx, simStates[instIdx], GetInstantReadState(action.actionType, isDS));
	}

	#pragma endregion


	#pragma region OutputRT

	for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
	{
		const TexID texId = OutTexId(action, r);
		const uint16_t instIdx = OutInstantIdx(action, r);

		if (texId != 0)
		{
			const DX12_TextureUsage needUsage = DX12_TextureUsage_RTV;
			bool valid = true;

			if (texId >= textureList.size() || (texId < releaseTexIdMask.size() && releaseTexIdMask[texId]))
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 출력 RT 텍스처 없음 / 해제 예정");
				valid = false;
			}
			else if (texRefList_WTex[texId].usage != needUsage)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 출력 RT 텍스처 usage 불일치");
				valid = false;
			}
			else
			{
				for (uint8_t i = 0; i < r; i++)
				{
					if (OutTexId(action, i) == texId)
					{
						DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 같은 텍스처를 RT 두 슬롯에 지정");
						valid = false;
						break;
					}
				}
			}

			if (!valid)
			{
				SetOutTex(action, r, 0);
				continue;
			}

			TransitionLoaded(action, texId, texRefList_WTex[texId].usage, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		else if (instIdx != TEX_INSTANT_NONE)
		{
			if (instIdx >= rtInstantSimStateList.size())
			{
				DebugReport(DataHub::RenderReport_Invariant, L"[Barrier] 출력 RT 인스턴트 인덱스 범위 밖");
				SetOutInstant(action, r, TEX_INSTANT_NONE);
				continue;
			}

			TransitionInstant(action.beginBarriers, DX12_BarrierTarget_RTInstant, instIdx,
				rtInstantSimStateList[instIdx], D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}

	#pragma endregion


	#pragma region OutputDS

	const TexID dsTexId = OutTexId(action, INSTANT_SLOT_DS);
	const uint16_t dsInstIdx = OutInstantIdx(action, INSTANT_SLOT_DS);
	const D3D12_RESOURCE_STATES dsUse = action.dsReadOnly ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE;

	if (dsTexId != 0)
	{
		bool valid = true;

		if (dsTexId >= textureList.size() || (dsTexId < releaseTexIdMask.size() && releaseTexIdMask[dsTexId]))
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 출력 DS 텍스처 없음 / 해제 예정");
			valid = false;
		}
		else if (texRefList_WTex[dsTexId].usage != DX12_TextureUsage_DSV)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 출력 DS 텍스처 usage 불일치");
			valid = false;
		}

		if (valid)
			TransitionLoaded(action, dsTexId, texRefList_WTex[dsTexId].usage, dsUse);
		else
			SetOutTex(action, INSTANT_SLOT_DS, 0);
	}
	else if (dsInstIdx != TEX_INSTANT_NONE)
	{
		if (dsInstIdx >= dsInstantSimStateList.size())
		{
			DebugReport(DataHub::RenderReport_Invariant, L"[Barrier] 출력 DS 인스턴트 인덱스 범위 밖");
			SetOutInstant(action, INSTANT_SLOT_DS, TEX_INSTANT_NONE);
		}
		else
		{
			TransitionInstant(action.beginBarriers, DX12_BarrierTarget_DSInstant, dsInstIdx,
				dsInstantSimStateList[dsInstIdx], dsUse);
		}
	}

	#pragma endregion


	#pragma region ComputeBinding

	if (isCompute)
	{
		for (uint8_t k = 0; k < COMPUTE_BINDING_MAX; k++)
		{
			const uint8_t s = (uint8_t)(CS_SLOT_WRITE0 + k);
			const uint16_t id = action.slotIds[s];
			const bool isTex = IsCSTexture(action, s);

			if (id == 0)
				continue;

			bool valid = true;

			for (uint8_t i = CS_SLOT_WRITE0; i < s && valid; i++)
				valid = !(action.slotIds[i] == id && IsCSTexture(action, i) == isTex);

			if (!valid)
				DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 쓰기 칸에 같은 자원 중복 (칸 무시)");
			else if (!isTex)
			{
				if (id >= csBufferList.size() || (id < releaseCSBufferIdMask.size() && releaseCSBufferIdMask[id]))
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 쓰기 버퍼 없음 / 해제 예정 (칸 무시)");
					valid = false;
				}
			}
			else
			{
				if (id >= textureList.size() || (id < releaseTexIdMask.size() && releaseTexIdMask[id]))
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 쓰기 텍스처 없음 / 해제 예정 (칸 무시)");
					valid = false;
				}
				else if (texRefList_WTex[id].usage != DX12_TextureUsage_UAV || writableTextureList[texRefList_WTex[id].idx].VHeapIndex == UINT_MAX)
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 쓰기 텍스처 usage 가 Compute 가 아님 (칸 무시)");
					valid = false;
				}
			}

			if (!valid)
			{
				SetCSBinding(action, s, 0, false);
				continue;
			}

			if (!isTex)
				TransitionBuffer(action, id, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			else
				TransitionLoaded(action, id, texRefList_WTex[id].usage, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		for (uint8_t k = 0; k < COMPUTE_BINDING_MAX; k++)
		{
			const uint8_t s = (uint8_t)(CS_SLOT_READ0 + k);
			const uint16_t id = action.slotIds[s];
			const bool isTex = IsCSTexture(action, s);

			bool valid = true;

			for (uint8_t i = CS_SLOT_WRITE0; i < CS_SLOT_WRITE0 + COMPUTE_BINDING_MAX && valid; i++)
				valid = !(action.slotIds[i] != 0 && action.slotIds[i] == id && IsCSTexture(action, i) == isTex);

			if (!valid)
				DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 읽기 칸이 같은 액션의 쓰기 자원 (칸 무시)");
			else if (!isTex)
			{
				if (id >= csBufferList.size() || (id < releaseCSBufferIdMask.size() && releaseCSBufferIdMask[id]))
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 읽기 버퍼 없음 / 해제 예정 (칸 무시)");
					valid = false;
				}
			}
			else
			{
				if (id >= textureList.size() || (id < releaseTexIdMask.size() && releaseTexIdMask[id]))
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Barrier] 컴퓨트 읽기 텍스처 없음 / 해제 예정 (칸 무시)");
					valid = false;
				}
			}

			if (!valid)
			{
				SetCSBinding(action, s, 0, false);
				continue;
			}

			if (isTex)
			{
				MarkOrBuildSingleTexture(id);
				TransitionLoaded(action, id, texRefList_WTex[id].usage, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
		}
	}

	#pragma endregion
}

#pragma endregion

#pragma endregion

}
