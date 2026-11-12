#include "D3D12Renderer.h"

#include "D3D12Core.h"

#include <bit>


namespace Hakko
{

using namespace std;


#pragma region Helper

static UINT PickLimit(UINT value, UINT fallback, UINT max)
{
	if (value == 0)
		return fallback;
	if (value > max)
		return max;
	return value;
}

static std::wstring GetExecutableDirectory()
{
	std::wstring path(MAX_PATH, L'\0');
	DWORD len = 0;
	for (;;)
	{
		len = GetModuleFileNameW(nullptr, path.data(), (DWORD)path.size());
		if (len == 0)
			return L"";
		if (len < path.size())
			break;
		path.resize(path.size() * 2);
	}
	path.resize(len);

	const size_t slash = path.find_last_of(L"\\/");
	return slash == std::wstring::npos ? std::wstring() : path.substr(0, slash + 1);
}


static D3D_PRIMITIVE_TOPOLOGY ToTopology(DataHub::DrawProp_Topology topology)
{
	switch (topology)
	{
	default:
	case DataHub::DrawProp_Topology_TriangleList:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case DataHub::DrawProp_Topology_TriangleStrip:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case DataHub::DrawProp_Topology_LineList:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case DataHub::DrawProp_Topology_LineStrip:		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case DataHub::DrawProp_Topology_PointList:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	}
}


static DataHub::RenderIdProvider* GetRenderIdProvider(DataHub::RenderIDType type)
{
	using namespace DataHub;

	static RenderIdNone none;
	static RenderIdOpaque opq;
	static RenderIdTranslucent tp;
	static RenderIdShadow sdw;
	static RenderIdScreen sc;

	switch (type)
	{
	default:
	case RenderIdType_None:			return &none;
	case RenderIdType_Opaque:		return &opq;
	case RenderIdType_Translucent:	return &tp;
	case RenderIdType_Shadow:		return &sdw;
	case RenderIdType_Screen:		return &sc;
	}
}

static void RadixSortRenderKeys(std::vector<DX12_RenderRequestProxy>& data, std::vector<DX12_RenderRequestProxy>& scratch, bool ascending)
{

	const size_t n = data.size();
	if (n < 2)
		return;

	if (n <= 32)
	{
		for (size_t i = 1; i < n; i++)
		{
			const DX12_RenderRequestProxy item = data[i];
			size_t j = i;

			if (ascending)
			{
				while (j > 0 && data[j - 1].renderId > item.renderId) { data[j] = data[j - 1]; j--; }
			}
			else
			{
				while (j > 0 && data[j - 1].renderId < item.renderId) { data[j] = data[j - 1]; j--; }
			}

			data[j] = item;
		}
		return;
	}

	scratch.resize(n);
	DX12_RenderRequestProxy* src = data.data();
	DX12_RenderRequestProxy* dst = scratch.data();
	const uint8_t flip = ascending ? 0 : 0xFF;

	for (int shift = 0; shift < 64; shift += 8)
	{
		uint32_t counts[256] = {};

		for (size_t i = 0; i < n; i++)
			counts[(uint8_t)((src[i].renderId >> shift) & 0xFF) ^ flip]++;

		if (counts[(uint8_t)((src[0].renderId >> shift) & 0xFF) ^ flip] == n)
			continue;

		uint32_t offset = 0;
		for (int b = 0; b < 256; b++)
		{
			const uint32_t c = counts[b];
			counts[b] = offset;
			offset += c;
		}

		for (size_t i = 0; i < n; i++)
			dst[counts[(uint8_t)((src[i].renderId >> shift) & 0xFF) ^ flip]++] = src[i];

		std::swap(src, dst);
	}

	if (src != data.data())
		memcpy(data.data(), src, n * sizeof(DX12_RenderRequestProxy));
}

#pragma endregion



#pragma region Constructor

D3D12Renderer::D3D12Renderer(const RenderLimits& limits) :
	requestedLimits(limits),
	RES_COLLECT_FREQ(RenderTuning().resCollectFreq),
	STEX_COLLECT_THRESHOLD((UINT16)RenderTuning().texCollectThreshold),	

	CB_AREA_INIT_SIZE_DEFAULT(PickLimit(limits.cbAreaInitNum, RenderLimits().cbAreaInitNum, UINT_MAX)),
	CB_AREA_SHRINK_FREQ(limits.cbAreaShrinkFreq),
	CB_AREA_SHRINK_THRESHOLD(PickLimit(limits.cbAreaShrinkThreshold, RenderLimits().cbAreaShrinkThreshold, UINT_MAX)),

	RTV_HEAP_NUM_DESCS(PickLimit(limits.rtvDescriptors, RenderLimits().rtvDescriptors, UINT_MAX)),
	DSV_HEAP_NUM_DESCS(PickLimit(limits.dsvDescriptors, RenderLimits().dsvDescriptors, UINT_MAX)),
	RV_HEAP_NUM_DESCS(PickLimit(limits.rvDescriptors, RenderLimits().rvDescriptors, D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1)),
	
	backBufferIndex(0),
	cpuFrameIndex(0),
	currentFrameCount(0),
	layerCount(0),

	vsync(true),
	tearingSupported(false),
	suspended(false),

	DEFAULT_HEAP_PROPS(D3D12_HEAP_TYPE_DEFAULT),
	UPLOAD_HEAP_PROPS(D3D12_HEAP_TYPE_UPLOAD)
{
}

#pragma endregion



#pragma region Lifecycle

bool D3D12Renderer::Init(HWND hwnd, int width, int height)
{
	hWnd = hwnd;
	defaultShaderDir = GetExecutableDirectory() + L"Render/D3D12/shader";

	if (requestedLimits.rvDescriptors > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1)
		DebugReport(DataHub::RenderReport_Init, L"[Init] limits.rvDescriptors 가 셰이더 가시 힙 상한 (1,000,000) 을 넘어 상한으로 클램프");

	if (FRAME_COUNT > 3 || FRAME_COUNT < 2)
		return false;
	


	#pragma region Debug
	#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) 
		debugController->EnableDebugLayer();

	#endif

	HRESULT hr = {};
	#pragma endregion



	#pragma region Core

	IDXGIFactory4* factory = D3D12Core::getInstance().GetFactory();
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();

	#pragma endregion



	#pragma region CmdQueue

	renderCMDQueue = D3D12Core::getInstance().GetDirectQueue();


	bool cmdOk = true;

	for (size_t i = 0; i < FRAME_COUNT; i++)
		cmdOk = cmdOk && SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&renderCMDAllocators[i])));
	cmdOk = cmdOk && SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadCMDAllocator)));

	cmdOk = cmdOk && SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, renderCMDAllocators[0].Get(), nullptr, IID_PPV_ARGS(&renderCMDList_Master)));

	if (!cmdOk)
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 커맨드 할당자 / 리스트 생성 실패");
		return false;
	}

	hr = renderCMDList_Master->Close();

	#pragma endregion



	#pragma region SwapChain

	bbWidth = (UINT)width;
	bbHeight = (UINT)height;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_COUNT;
	swapChainDesc.Width = bbWidth;
	swapChainDesc.Height = bbHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	{
		ComPtr<IDXGIFactory5> factory5;
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5))) &&
			SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			tearingSupported = allowTearing == TRUE;
	}
	swapChainDesc.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> sc;
	hr = factory->CreateSwapChainForHwnd(
		renderCMDQueue,
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&sc
	);

	if (SUCCEEDED(hr))
		hr = sc.As(&swapChain);

	if (FAILED(hr))
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 스왑체인 생성 실패");
		return false;
	}

	factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	#pragma endregion



	#pragma region RT

	rtvHeap = make_unique<RTVHeap>(RTV_HEAP_NUM_DESCS, FRAME_COUNT);

	if (!rtvHeap->IsValid())
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] RTV 힙 생성 실패");
		return false;
	}


	D3D12_RENDER_TARGET_VIEW_DESC nullDesc = {};
	nullDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	nullDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	UINT nullIndex;
	rtvHeap->SetRTV(nullptr, &nullDesc, nullIndex);
	nullRTVHandle = rtvHeap->GetCPUHandle(nullIndex);
	

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT n = 0; n < FRAME_COUNT; n++)
	{
		hr = swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffers[n]));

		if (FAILED(hr))
		{
			DebugReport(DataHub::RenderReport_Init, L"[Init] 백버퍼 가져오기 실패");
			return false;
		}

		UINT rtvIndex;
		rtvHeap->SetRTV(backBuffers[n].Get(), &rtvDesc, rtvIndex);
		bbvCPUHandles[n] = rtvHeap->GetCPUHandle(rtvIndex);
	}

	#pragma endregion



	#pragma region DS

	dsvHeap = make_unique<DSVHeap>(DSV_HEAP_NUM_DESCS);

	if (!dsvHeap->IsValid())
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] DSV 힙 생성 실패");
		return false;
	}

	#pragma endregion



	#pragma region Fence

	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));


	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (FAILED(hr) || fenceEvent == nullptr)
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 펜스 / 펜스 이벤트 생성 실패");
		return false;
	}

	currentFenceValue = 1;

	for (size_t i = 0; i < FRAME_COUNT; i++)
		frameFenceValues[i] = 0;

	#pragma endregion



	#pragma region RVHeap

	rvHeap = std::make_unique<RVHeapAllocator>(RV_HEAP_NUM_DESCS);

	if (!rvHeap->IsValid())
	{
		DebugReport(DataHub::RenderReport_Init, L"[Init] 리소스 뷰 힙 생성 실패 (limits.rvDescriptors 확인)");
		return false;
	}

	#pragma endregion
	


	if (!InitShaderCompiler())
		return false;

	if (!InitResource())
		return false;

	if (!InitCanvasLayer())
		return false;


	return true;
}

void D3D12Renderer::Finalize()
{
	if (fence != nullptr)
	{
		for (size_t i = 0; i < FRAME_COUNT; i++)
		{
			WaitGPUProcess();
			cpuFrameIndex = (cpuFrameIndex + 1) % FRAME_COUNT;
		}
	}

	if (fenceEvent != nullptr)
		CloseHandle(fenceEvent);


	backBufferIndex = 0;
	cpuFrameIndex = 0;
	currentFrameCount = 0;
	layerCount = 0;

	for (size_t i = 0; i < FRAME_COUNT; i++)
	{
		cbArea_Condition[i].reset();
		cbArea_DataBox[i].clear();

		frameFenceValues[i] = 0;
		renderFrames[i] = {};
		renderFrames[i].frameInvalidated = false;

		retiredMeshLists[i].clear();
		retiredTextureLists[i].clear();
		retiredCSBufferLists[i].clear();
		retiredPSOLists[i].clear();
		retiredSTexViewLists[i].clear();
		retiredCTexViewLists[i].clear();
	}

	cbArea_Light.clear();
	cbArea_LightMatrix.clear();
	
	retiredCBAreaList.clear();
	dataSlotLUTList.clear();
	dataSlotStampList.clear();
	

	rvHeap.reset();

}


void D3D12Renderer::OnResize(int width, int height)
{
	if (width == 0 || height == 0)
	{
		suspended = true;
		return;
	}
	suspended = false;

	
	for (UINT i = 0; i < FRAME_COUNT; i++)
	{
		WaitGPUProcess();
		cpuFrameIndex = (cpuFrameIndex + 1) % FRAME_COUNT;
	}


	for (UINT n = 0; n < FRAME_COUNT; n++)
	{
		backBuffers[n].Reset();

		for (size_t i = 0; i < rtTextureInstantLists[n].size(); i++)
		{
			rvHeap->UnSetSRV(rtSTexInstantLists[n][i].RVHeapIndex);
			rtvHeap->UnSetRTV(rtvInstantLists[n][i].VHeapIndex);
		}
		for (size_t i = 0; i < dsTextureInstantLists[n].size(); i++)
		{
			rvHeap->UnSetSRV(dsSTexInstantLists[n][i].RVHeapIndex);
			dsvHeap->UnSetDSV(dsvInstantLists[n][i].VHeapIndex);
			dsvHeap->UnSetDSV(dsvReadOnlyInstantLists[n][i].VHeapIndex);
		}

		rtTextureInstantLists[n].clear();
		dsTextureInstantLists[n].clear();

		rtSTexInstantLists[n].clear();
		dsSTexInstantLists[n].clear();

		rtvInstantLists[n].clear();
		dsvInstantLists[n].clear();
		dsvReadOnlyInstantLists[n].clear();
	}


	bbWidth = (UINT)width;
	bbHeight = (UINT)height;

	swapChain->ResizeBuffers(FRAME_COUNT, bbWidth, bbHeight, DXGI_FORMAT_R8G8B8A8_UNORM, tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);


	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT n = 0; n < FRAME_COUNT; n++)
	{
		swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffers[n]));
		rtvHeap->UpdateRTV(n + 1, backBuffers[n].Get(), &rtvDesc);
	}
}

void D3D12Renderer::SetVSync(bool enable)
{
	vsync = enable;
}

void D3D12Renderer::SetTuning(const RenderTuning& tuning)
{
	RES_COLLECT_FREQ = tuning.resCollectFreq == 0 ? RenderTuning().resCollectFreq : tuning.resCollectFreq;
	STEX_COLLECT_THRESHOLD = tuning.texCollectThreshold == 0 ? (UINT16)RenderTuning().texCollectThreshold : (UINT16)tuning.texCollectThreshold;
}


void D3D12Renderer::Sketch()
{
	#pragma region UpdateFrameIndex

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	cpuFrameIndex = (cpuFrameIndex + 1) % FRAME_COUNT;
	currentFrameCount++;

	#pragma endregion


	auto& renderFrame = renderFrames[cpuFrameIndex];
	renderFrame.frameInvalidated = false;

#if RENDER_REPORT_SEND_MODE == RENDER_REPORT_SEND_ENABLE
	DataHub::RenderReportList.clear();
#endif

	UpdateCanvasLayer();

	ReadTransferBox();

	if (DataHub::flag_updateResources & DataHub::RTBCMD_Release_Resource)
		InvalidateView();

	if (currentFrameCount % RES_COLLECT_FREQ == 0)
		CollectManagedResource();

	if (renderFrame.frameInvalidated)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[Frame] 무효 프레임 (이번 프레임 그리기 요청 전부 버림)");

		WaitGPUProcess();
		ReleaseRetiredResource();
		return;
	}


	SortRenderId();

	GetConditionData();
	
	GetDataFromSortedId();

	WaitGPUProcess();

	UploadRenderData();
	
}

void D3D12Renderer::Render()
{
	using namespace DataHub;

	if (suspended)
		return;


	auto& renderFrame = renderFrames[cpuFrameIndex];
	const UINT frameIdx = cpuFrameIndex;	

	if (renderFrame.frameInvalidated)
	{
		ReadReleaseRequest();
		ScheduledReleaseResource();

		return;
	}


	#pragma region FrameInitialize

	renderCMDAllocators[cpuFrameIndex]->Reset();
	renderCMDList_Master->Reset(renderCMDAllocators[cpuFrameIndex].Get(), nullptr);

	ID3D12GraphicsCommandList* cmd = renderCMDList_Master.Get();


	ID3D12DescriptorHeap* descriptorHeaps[] = { rvHeap->GetDescriptorHeap() };
	cmd->SetDescriptorHeaps(1, descriptorHeaps);
	cmd->SetGraphicsRootSignature(defaultRenderRootSig.Get());
	cmd->SetGraphicsRootShaderResourceView(RootParam_StructuredSB, csBufferViewList[0].address);
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	MarkOrBuildSingleTexture(0);
	const D3D12_GPU_DESCRIPTOR_HANDLE defaultSRHandle = rvHeap->GetGPUHandle(singleTextureList[0].RVHeapIndex);

	MarkOrBuildSingleTexture(DEFAULT_TEX_TRANSPARENT);
	const D3D12_GPU_DESCRIPTOR_HANDLE transparentSRHandle = rvHeap->GetGPUHandle(singleTextureList[DEFAULT_TEX_TRANSPARENT].RVHeapIndex);

	const auto& rtInstants = rtTextureInstantLists[frameIdx];
	const auto& dsInstants = dsTextureInstantLists[frameIdx];

	static vector<bool> instantClearedMask_RT;
	static vector<bool> instantClearedMask_DS;
	instantClearedMask_RT.assign(rtInstants.size(), false);
	instantClearedMask_DS.assign(dsInstants.size(), false);

	static const float instantClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	D3D12_GPU_DESCRIPTOR_HANDLE layerResultHandles[RENDER_LAYER_MAX];
	for (size_t l = 0; l < RENDER_LAYER_MAX; l++)
		layerResultHandles[l] = transparentSRHandle;


	D3D12_GPU_DESCRIPTOR_HANDLE boundSRs[INSTANT_SLOT_MAX];
	for (size_t s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		boundSRs[s] = defaultSRHandle;
		cmd->SetGraphicsRootDescriptorTable((UINT)(RootParam_InstantSR0 + s), defaultSRHandle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE boundRTVs[RENDER_TARGET_MAX] = {};
	D3D_PRIMITIVE_TOPOLOGY boundTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT boundRTVCount = UINT_MAX;
	D3D12_CPU_DESCRIPTOR_HANDLE boundDSV = {};
	D3D12_VIEWPORT boundViewport = {};
	D3D12_RECT boundScissor = {};

	#pragma endregion


	#pragma region Layers

	for (size_t l = 0; l < layerCount; l++)
	{
		auto& layer = renderFrame.layerList[l];


		cmd->SetGraphicsRootConstantBufferView(RootParam_ConditionCB, cbArea_Condition[frameIdx]->GetGPUAddress(layer.conditionRenderData.CBAreaIndex));
		cmd->SetGraphicsRootConstantBufferView(RootParam_LightCB, cbArea_Light[frameIdx].GetGPUAddress(layer.lightRenderData.CBAreaIndex));
		cmd->SetGraphicsRootConstantBufferView(RootParam_LightMatrixCB, cbArea_LightMatrix[frameIdx].GetGPUAddress(layer.lightMatrixRenderData.CBAreaIndex));

		if (layer.envTexId != 0)
			cmd->SetGraphicsRootDescriptorTable(RootParam_LayerTexTable, rvHeap->GetGPUHandle(singleTextureList[layer.envTexId].RVHeapIndex));
		else
			cmd->SetGraphicsRootDescriptorTable(RootParam_LayerTexTable, defaultSRHandle);

		if (layer.layerDataTid != 0)
			cmd->SetGraphicsRootConstantBufferView(RootParam_LayerDataCB, cbArea_DataBox[frameIdx][layer.layerDataTid].GetGPUAddress(layer.layerRenderData.CBAreaIndex));


		#pragma region Actions

		for (size_t i = 1; i < layer.renderActionList.size(); i++)
		{
			DX12_RenderAction& action = layer.renderActionList[i];


			RecordActionBarriers(cmd, layer, action.copyBarriers, frameIdx);

			for (size_t s = 0; s < INSTANT_SLOT_MAX; s++)
			{
				if (action.copySrcInstantIdx[s] == TEX_INSTANT_NONE)
					continue;

				if (s < RENDER_TARGET_MAX)
					cmd->CopyResource(rtInstants[action.srInstantIdx[s] + layer.rtInstantOffset].Tex.Get(), rtInstants[action.copySrcInstantIdx[s] + layer.rtInstantOffset].Tex.Get());
				else
					cmd->CopyResource(dsInstants[action.srInstantIdx[s] + layer.dsInstantOffset].Tex.Get(), dsInstants[action.copySrcInstantIdx[s] + layer.dsInstantOffset].Tex.Get());
			}

			RecordActionBarriers(cmd, layer, action.beginBarriers, frameIdx);


			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[RENDER_TARGET_MAX];
			UINT rtvCount = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

			if (action.outputSigId != OUTPUT_SIGNATURE_INVALID)
			{
				for (uint8_t r = 0; r < RENDER_TARGET_MAX; r++)
				{
					const TexID texId = OutTexId(action, r);
					const Action_Output_LoadOption loadOpt = action.rtLoadOpts[r];

					if (texId == 0 && !IsOutInstant(action, r))
					{
						rtvHandles[r] = nullRTVHandle;
						continue;
					}

					if (texId != 0)
					{
						const uint16_t wIdx = texRefList_WTex[texId].idx;
						rtvHandles[r] = rtvHeap->GetCPUHandle(writableTextureList[wIdx].VHeapIndex);

						if (loadOpt == Action_Output_LoadOption_Clear)
							cmd->ClearRenderTargetView(rtvHandles[r], writableClearList[wIdx].Color, 0, nullptr);
					}
					else
					{
						const size_t idx = (size_t)action.slotIds[r] + layer.rtInstantOffset;
						rtvHandles[r] = rtvHeap->GetCPUHandle(rtvInstantLists[frameIdx][idx].VHeapIndex);

						if (loadOpt == Action_Output_LoadOption_Clear || (loadOpt == Action_Output_LoadOption_Default && !instantClearedMask_RT[idx]))
							cmd->ClearRenderTargetView(rtvHandles[r], instantClearColor, 0, nullptr);
						instantClearedMask_RT[idx] = true;
					}

					rtvCount = r + 1;
				}

				const TexID dsTexId = OutTexId(action, INSTANT_SLOT_DS);

				if (dsTexId != 0)
				{
					const uint16_t wIdx = texRefList_WTex[dsTexId].idx;
					const UINT dsvIndex = writableTextureList[wIdx].VHeapIndex;

					if (!action.dsReadOnly && action.dsLoadOpt == Action_Output_LoadOption_Clear)
					{
						const D3D12_CLEAR_VALUE& clear = writableClearList[wIdx];
						cmd->ClearDepthStencilView(dsvHeap->GetCPUHandle(dsvIndex), GetDSClearFlags(clear.Format), clear.DepthStencil.Depth, clear.DepthStencil.Stencil, 0, nullptr);
					}

					dsvHandle = dsvHeap->GetCPUHandle(action.dsReadOnly ? dsvIndex + 1 : dsvIndex);
				}
				else if (IsOutInstant(action, INSTANT_SLOT_DS))
				{
					const size_t idx = (size_t)action.slotIds[INSTANT_SLOT_DS] + layer.dsInstantOffset;

					if (!action.dsReadOnly)
					{
						if (action.dsLoadOpt == Action_Output_LoadOption_Clear || (action.dsLoadOpt == Action_Output_LoadOption_Default && !instantClearedMask_DS[idx]))
							cmd->ClearDepthStencilView(dsvHeap->GetCPUHandle(dsvInstantLists[frameIdx][idx].VHeapIndex), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
						instantClearedMask_DS[idx] = true;
					}

					dsvHandle = action.dsReadOnly
						? dsvHeap->GetCPUHandle(dsvReadOnlyInstantLists[frameIdx][idx].VHeapIndex)
						: dsvHeap->GetCPUHandle(dsvInstantLists[frameIdx][idx].VHeapIndex);
				}
			}


			bool hasWork = action.outputSigId != OUTPUT_SIGNATURE_INVALID;
			ID3D12PipelineState* builtinPSO = nullptr;

			if (hasWork)
			{
				switch (action.actionType)
				{
				case DX12_RenderActionType_Builtin:
					if (action.builtinType == BuiltinAction_Background && layer.bgTexId != 0)
						builtinPSO = psoGeneratorList[layer.bgIsCube ? SKYBOX_PASS_PSOID : BG_PASS_PSOID].psoVariants[action.outputSigId].Get();
					hasWork = builtinPSO != nullptr;
					break;

				case DX12_RenderActionType_Edit:
					hasWork = !layer.actionRenderRequestList[i].empty() && (layer.actionRenderRequestList[i][0].renderId >> 48) != 0;
					break;

				case DX12_RenderActionType_Draw:
					hasWork = !layer.actionSortedRenderDataList[i].empty();
					break;

				default:
					hasWork = false;
					break;
				}
			}


			if (hasWork)
			{
				const OutputSignatureID sigId = action.outputSigId;


				bool rtChanged = rtvCount != boundRTVCount || dsvHandle.ptr != boundDSV.ptr;
				for (UINT r = 0; r < rtvCount && !rtChanged; r++)
					rtChanged = rtvHandles[r].ptr != boundRTVs[r].ptr;

				if (rtChanged)
				{
					cmd->OMSetRenderTargets(rtvCount, rtvHandles, FALSE, dsvHandle.ptr != 0 ? &dsvHandle : nullptr);
					boundRTVCount = rtvCount;
					boundDSV = dsvHandle;
					for (UINT r = 0; r < rtvCount; r++)
						boundRTVs[r] = rtvHandles[r];
				}

				if (memcmp(&action.viewport, &boundViewport, sizeof(D3D12_VIEWPORT)) != 0)
				{
					cmd->RSSetViewports(1, &action.viewport);
					boundViewport = action.viewport;
				}
				if (memcmp(&action.scissorRect, &boundScissor, sizeof(D3D12_RECT)) != 0)
				{
					cmd->RSSetScissorRects(1, &action.scissorRect);
					boundScissor = action.scissorRect;
				}

				for (size_t s = 0; s < INSTANT_SLOT_MAX; s++)
				{
					D3D12_GPU_DESCRIPTOR_HANDLE handle = defaultSRHandle;

					if (action.srInstantIdx[s] != TEX_INSTANT_NONE)
					{
						if (s < RENDER_TARGET_MAX)
							handle = rvHeap->GetGPUHandle(rtSTexInstantLists[frameIdx][(size_t)action.srInstantIdx[s] + layer.rtInstantOffset].RVHeapIndex);
						else
							handle = rvHeap->GetGPUHandle(dsSTexInstantLists[frameIdx][(size_t)action.srInstantIdx[s] + layer.dsInstantOffset].RVHeapIndex);
					}

					if (handle.ptr != boundSRs[s].ptr)
					{
						cmd->SetGraphicsRootDescriptorTable((UINT)(RootParam_InstantSR0 + s), handle);
						boundSRs[s] = handle;
					}
				}

				if (action.actionDataTid != 0)
					cmd->SetGraphicsRootConstantBufferView(RootParam_ActionCB, cbArea_DataBox[frameIdx][action.actionDataTid].GetGPUAddress(action.actionRenderData.CBAreaIndex));


				switch (action.actionType)
				{
				case DX12_RenderActionType_Builtin:
				{
					if (boundTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
					{
						cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						boundTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					}
					cmd->SetPipelineState(builtinPSO);

					cmd->SetGraphicsRootDescriptorTable(RootParam_CTexTable, rvHeap->GetGPUHandle(singleTextureList[layer.bgTexId].RVHeapIndex));
					cmd->SetGraphicsRoot32BitConstants(RootParam_Constants, 4, layer.bgColor, 0);
					cmd->DrawInstanced(3, 1, 0, 0);
					break;
				}

				case DX12_RenderActionType_Edit:
				{
					const RenderID renderId = layer.actionRenderRequestList[i][0].renderId;
					const PSOID psoId = (PSOID)(renderId >> 48);
					const CTexID cTexId = (CTexID)((renderId >> 32) & 0xFFFF);

					if (boundTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
					{
						cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						boundTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					}
					cmd->SetPipelineState(psoGeneratorList[psoId].psoVariants[sigId].Get());
					cmd->SetGraphicsRootDescriptorTable(RootParam_CTexTable, rvHeap->GetGPUHandle(combinedTextureList[cTexId].RVHeapIndex));

					const DX12_MeshView& quad = meshViewList[1];
					cmd->IASetVertexBuffers(0, 1, &quad.VBV);
					cmd->IASetIndexBuffer(&quad.IBV);
					cmd->DrawIndexedInstanced(quad.subMeshes[0].indexCount, 1, 0, 0, 0);
					break;
				}

				case DX12_RenderActionType_Draw:
				{
					const auto& sortedList = layer.actionSortedRenderDataList[i];

					RenderIdProvider* provider = GetRenderIdProvider(action.renderIdType);

					RenderId_ParseFlag psoFlag, cTexFlag, sTexFlag;
					const uint64_t PSOMASK = provider->GetDrawPropIdMask(psoFlag);
					const uint64_t CTEXMASK = provider->GetTexPropIdMask(cTexFlag);
					const uint64_t STEXMASK = provider->GetTexIdMask(sTexFlag);
					const int PSOSHIFT = PSOMASK ? countr_zero(PSOMASK) : 0;
					const int CTEXSHIFT = CTEXMASK ? countr_zero(CTEXMASK) : 0;
					const int STEXSHIFT = STEXMASK ? countr_zero(STEXMASK) : 0;
					const bool useCTex = (cTexFlag == RenderId_ParseFlag_OK) && CTEXMASK != 0;
					const bool useSTex = (sTexFlag == RenderId_ParseFlag_OK) && STEXMASK != 0;

					UINT slotCount = 0;
					UINT slotK[DRAWREQ_DATABOX_MAX];
					D3D12_GPU_VIRTUAL_ADDRESS slotBase[DRAWREQ_DATABOX_MAX];
					UINT slotStride[DRAWREQ_DATABOX_MAX];
					UINT sbSlotK = UINT_MAX;

					for (UINT k = 0; k < DRAWREQ_DATABOX_MAX; k++)
					{
						const DataBoxTID tid = action.dataTids[k];
						if (tid == 0)
							continue;

						if (tid == DataBoxType_CSBuffer)
						{
							sbSlotK = k;
							continue;
						}

						slotK[slotCount] = k;
						slotBase[slotCount] = cbArea_DataBox[frameIdx][tid].GetGPUAddress(0);
						slotStride[slotCount] = cbArea_DataBox[frameIdx][tid].GetStride();
						slotCount++;
					}

					PSOID prevPsoId = 0;
					CTexID prevCTexId = 0xFFFF;
					STexID prevSTexId = 0xFFFF;
					MeshID prevMeshId = 0xFFFF;
					uint32_t prevSlotIdx[DRAWREQ_DATABOX_MAX];
					for (UINT a = 0; a < DRAWREQ_DATABOX_MAX; a++)
						prevSlotIdx[a] = UINT32_MAX;
					uint32_t prevSBId = UINT32_MAX;

					for (size_t j = 0; j < sortedList.size(); j++)
					{
						const DX12_RenderDrawData& draw = sortedList[j];

						const PSOID psoId = (PSOID)((draw.renderId & PSOMASK) >> PSOSHIFT);
						if (psoId != prevPsoId)
						{
							if (psoId != 0)
							{
								cmd->SetPipelineState(psoGeneratorList[psoId].psoVariants[sigId].Get());

								const D3D_PRIMITIVE_TOPOLOGY topology = ToTopology(psoGeneratorList[psoId].psoBody.topology);
								if (topology != boundTopology)
								{
									cmd->IASetPrimitiveTopology(topology);
									boundTopology = topology;
								}
							}
							prevPsoId = psoId;
						}
						if (psoId == 0)
							continue;

						if (useCTex)
						{
							const CTexID cTexId = (CTexID)((draw.renderId & CTEXMASK) >> CTEXSHIFT);
							if (cTexId != prevCTexId)
							{
								cmd->SetGraphicsRootDescriptorTable(RootParam_CTexTable, rvHeap->GetGPUHandle(combinedTextureList[cTexId].RVHeapIndex));
								prevCTexId = cTexId;
							}
						}
						if (useSTex)
						{
							const STexID sTexId = (STexID)((draw.renderId & STEXMASK) >> STEXSHIFT);
							if (sTexId != prevSTexId)
							{
								cmd->SetGraphicsRootDescriptorTable(RootParam_CTexTable, rvHeap->GetGPUHandle(singleTextureList[sTexId].RVHeapIndex));
								prevSTexId = sTexId;
							}
						}

						const DX12_MeshView& meshView = meshViewList[draw.meshId];
						if (draw.meshId != prevMeshId)
						{
							cmd->IASetVertexBuffers(0, 1, &meshView.VBV);
							cmd->IASetIndexBuffer(&meshView.IBV);
							prevMeshId = draw.meshId;
						}

						for (UINT a = 0; a < slotCount; a++)
						{
							const uint32_t idx = draw.datas[slotK[a]].CBAreaIndex;
							if (idx == prevSlotIdx[a])
								continue;

							cmd->SetGraphicsRootConstantBufferView(RootParam_ObjectCB0 + slotK[a], slotBase[a] + (UINT64)idx * slotStride[a]);
							prevSlotIdx[a] = idx;
						}

						if (sbSlotK != UINT_MAX)
						{
							const uint32_t sbId = draw.datas[sbSlotK].CBAreaIndex;
							if (sbId != prevSBId)
							{
								cmd->SetGraphicsRootShaderResourceView(RootParam_StructuredSB, csBufferViewList[sbId].address);
								prevSBId = sbId;
							}
						}

						const DX12_SubMesh& sub = meshView.subMeshes[draw.subMeshId];
						cmd->DrawIndexedInstanced(sub.indexCount, 1, sub.indexStart, sub.baseVertex, 0);
					}
					break;
				}

				default:
					break;
				}
			}


			if (action.actionType == DX12_RenderActionType_Compute)
				RecordComputeAction(cmd, layer, i, frameIdx, defaultSRHandle);


			RecordActionBarriers(cmd, layer, action.endBarriers, frameIdx);
		}

		#pragma endregion


		RecordActionBarriers(cmd, layer, layer.finalBarriers, frameIdx);


		for (size_t i = layer.renderActionList.size(); i > 1; i--)
		{
			const DX12_RenderAction& action = layer.renderActionList[i - 1];

			if (action.outputSigId == OUTPUT_SIGNATURE_INVALID || action.actionType == DX12_RenderActionType_Compute)
				continue;
			if (!IsOutInstant(action, 0))
				continue;

			const size_t idx = (size_t)action.slotIds[0] + layer.rtInstantOffset;
			if (idx < rtSTexInstantLists[frameIdx].size() && l < RENDER_LAYER_MAX)
				layerResultHandles[l] = rvHeap->GetGPUHandle(rtSTexInstantLists[frameIdx][idx].RVHeapIndex);
			break;
		}
	}

	#pragma endregion


	#pragma region Composite

	auto RTresBar = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffers[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	cmd->ResourceBarrier(1, &RTresBar);

	cmd->OMSetRenderTargets(1, &bbvCPUHandles[backBufferIndex], FALSE, nullptr);

	const D3D12_VIEWPORT fullViewport = { 0.0f, 0.0f, (float)bbWidth, (float)bbHeight, 0.0f, 1.0f };
	const D3D12_RECT fullScissor = { 0, 0, (LONG)bbWidth, (LONG)bbHeight };
	cmd->RSSetViewports(1, &fullViewport);
	cmd->RSSetScissorRects(1, &fullScissor);

	if (layerCount > 0)
	{
		if (boundTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
			cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->SetPipelineState(compositePSO.Get());

		for (size_t s = 0; s < RENDER_TARGET_MAX; s++)
		{
			const D3D12_GPU_DESCRIPTOR_HANDLE handle = s < layerCount ? layerResultHandles[s] : defaultSRHandle;
			if (handle.ptr != boundSRs[s].ptr)
				cmd->SetGraphicsRootDescriptorTable((UINT)(RootParam_InstantSR0 + s), handle);
		}

		const float compositeConstants[4] = { (float)layerCount, 0.0f, 0.0f, 0.0f };
		cmd->SetGraphicsRoot32BitConstants(RootParam_Constants, 4, compositeConstants, 0);

		cmd->DrawInstanced(3, 1, 0, 0);
	}
	else
	{
		static const float bgColor[] = {0.0f, 0.1f, 0.2f, 1.0f};
		cmd->ClearRenderTargetView(bbvCPUHandles[backBufferIndex], bgColor, 0, nullptr);
	}

	#pragma endregion


	#pragma region FramePresent

	auto PTresBar = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffers[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	cmd->ResourceBarrier(1, &PTresBar);

	cmd->Close();

	ID3D12CommandList* cmdLists[] = {renderCMDList_Master.Get()};
	renderCMDQueue->ExecuteCommandLists(1, cmdLists);

	if (vsync)
		swapChain->Present(1, 0);
	else
		swapChain->Present(0, tearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0);

	SignalFence();

	#pragma endregion


	#pragma region ClearPassData

	for (size_t p = 0; p < layerCount; p++)
	{
		auto& layer = renderFrame.layerList[p];

		layer.conditionRenderData = {};
		layer.lightRenderData = {};
		layer.lightMatrixRenderData = {};
		layer.layerRenderData = {};
	}

	#pragma endregion


	#pragma region ScheduledProcess

	ReadReleaseRequest();

	ScheduledReleaseResource();

	#pragma endregion
}

#pragma endregion



#pragma region SketchCall

void D3D12Renderer::SortRenderId()
{
	using namespace DataHub;

	auto& renderFrame = renderFrames[cpuFrameIndex];
	static vector<DX12_RenderRequestProxy> sortScratch;

	for (size_t l = 0; l < layerCount; l++)
	{
		auto& layer = renderFrame.layerList[l];

		for (size_t i = 1; i < layer.actionRenderRequestList.size(); i++)
		{
			auto& proxyList = layer.actionRenderRequestList[i];
			auto& action = layer.renderActionList[i];

			RenderIdProvider* provider = GetRenderIdProvider(action.renderIdType);

			bool isAscending = provider->IsAscendingSort();

			RadixSortRenderKeys(proxyList, sortScratch, isAscending);

			if (action.outputSigId == OUTPUT_SIGNATURE_INVALID || action.actionType == DX12_RenderActionType_Compute)
				continue;

			if (action.actionType == DX12_RenderActionType_Builtin)
			{
				BuildPSOVariant(SKYBOX_PASS_PSOID, action.outputSigId);
				BuildPSOVariant(BG_PASS_PSOID, action.outputSigId);
				continue;
			}


			RenderId_ParseFlag psoFlag, cTexFlag, sTexFlag;
			const uint64_t PSOMASK = provider->GetDrawPropIdMask(psoFlag);
			const uint64_t CTEXMASK = provider->GetTexPropIdMask(cTexFlag);
			const uint64_t STEXMASK = provider->GetTexIdMask(sTexFlag);
			const int PSOSHIFT = PSOMASK ? countr_zero(PSOMASK) : 0;
			const int CTEXSHIFT = CTEXMASK ? countr_zero(CTEXMASK) : 0;
			const int STEXSHIFT = STEXMASK ? countr_zero(STEXMASK) : 0;
			const bool usePSO = (psoFlag == RenderId_ParseFlag_OK) && PSOMASK != 0;
			const bool useCTex = (cTexFlag == RenderId_ParseFlag_OK) && CTEXMASK != 0;
			const bool useSTex = (sTexFlag == RenderId_ParseFlag_OK) && STEXMASK != 0;

			if (!usePSO)
			{
				for (auto& proxy : proxyList)
					proxy.renderId &= ~PSOMASK;
				continue;
			}

			PSOID prevPsoId = 0;
			bool prevPsoValid = false;
			STexID prevSTexId = 0xFFFF;

			#ifdef _DEBUG
			bool hazardReported = false;
			auto isWriteTarget = [&action](TexID texId) -> bool
			{
				if (texId == 0)
					return false;
				for (uint8_t s = 0; s < INSTANT_SLOT_MAX; s++)
				{
					if (OutTexId(action, s) == texId)
						return true;
				}
				return false;
			};
			#endif

			for (auto& proxy : proxyList)
			{
				const PSOID psoId = (PSOID)((proxy.renderId & PSOMASK) >> PSOSHIFT);

				if (psoId != prevPsoId)
				{
					prevPsoId = psoId;
					prevPsoValid = false;

					if (psoId != 0 && psoId < psoGeneratorList.size() && psoGeneratorList[psoId].psoBody.VSId != 0)
					{
						BuildPSOVariant(psoId, action.outputSigId);
						prevPsoValid = psoGeneratorList[psoId].psoVariants[action.outputSigId] != nullptr;
					}
				}
				if (!prevPsoValid)
				{
					proxy.renderId &= ~PSOMASK;
					continue;
				}

				if (useSTex)
				{
					STexID sTexId = (STexID)((proxy.renderId & STEXMASK) >> STEXSHIFT);

					#ifdef _DEBUG
					if (sTexId >= textureList.size())
					{
						DebugReport(DataHub::RenderReport_Contract, L"[Tex] 요청 texId 범위 밖 (0 으로)");
						proxy.renderId &= ~STEXMASK;
						sTexId = 0;
					}
					#endif

					if (sTexId != prevSTexId)
					{
						MarkOrBuildSingleTexture(sTexId);
						prevSTexId = sTexId;

						#ifdef _DEBUG
						if (!hazardReported && isWriteTarget(sTexId))
						{
							DebugReport(DataHub::RenderReport_Contract, L"[WTex] 액션이 쓰는 텍스처를 같은 액션의 드로우가 읽음 (STex)");
							hazardReported = true;
						}
						#endif
					}
				}

				#ifdef _DEBUG
				if (useCTex)
				{
					const CTexID cTexId = (CTexID)((proxy.renderId & CTEXMASK) >> CTEXSHIFT);

					if (cTexId >= combinedTextureList.size())
					{
						DebugReport(DataHub::RenderReport_Contract, L"[CTex] 요청 texPropId 범위 밖 (0 으로)");
						proxy.renderId &= ~CTEXMASK;
					}
					else if (!hazardReported)
					{
						for (size_t s = 0; s < CTEX_COMBINED_SIZE; s++)
						{
							if (isWriteTarget(combinedTextureBodyList[cTexId].texId[s]))
							{
								DebugReport(DataHub::RenderReport_Contract, L"[WTex] 액션이 쓰는 텍스처를 같은 액션의 드로우가 읽음 (CTex)");
								hazardReported = true;
								break;
							}
						}
					}
				}
				#endif
}
		}
	}

	
}

void D3D12Renderer::GetConditionData()
{
	using namespace DataHub;

	auto& renderFrame = renderFrames[cpuFrameIndex];

	auto& cbLight = cbArea_Light[cpuFrameIndex];
	auto& cbLightMatrix = cbArea_LightMatrix[cpuFrameIndex];

	cbLight.ReserveArea(layerCount, retiredCBAreaList);
	cbLightMatrix.ReserveArea(layerCount, retiredCBAreaList);

	DataBox_RenderLight lightPack[RENDER_LIGHT_MAX];
	DataBox_RenderLightMatrix lightMatrixPack[RENDER_LIGHT_MAX];

	const size_t lightCount = RenderLightList.size();
	const size_t lightMatrixCount = RenderLightMatrixList.size();


	for (size_t i = 0; i < layerCount; i++)
	{
		auto& layer = renderFrame.layerList[i];


		cbArea_Condition[cpuFrameIndex]->SetCBData(
			layer.conditionRenderData,
			LayerList[i].condition
		);

		for (size_t j = 0; j < RENDER_LIGHT_MAX; j++)
		{
			const DataBoxID lightId = LayerList[i].lights[j];

			if (lightId < lightCount)
				lightPack[j] = RenderLightList[lightId];
			else
				lightPack[j].type = 0;
		}

		cbLight.SetCBData(layer.lightRenderData, lightPack);

		for (size_t j = 0; j < RENDER_LIGHT_MAX; j++)
		{
			const DataBoxID matId = LayerList[i].lightMatrixes[j];

			if (matId < lightMatrixCount)
				lightMatrixPack[j] = RenderLightMatrixList[matId];
			else
				lightMatrixPack[j].lightViewProj = DirectX::XMMATRIX{};
		}

		cbLightMatrix.SetCBData(layer.lightMatrixRenderData, lightMatrixPack);
	}
}

void D3D12Renderer::GetDataFromSortedId()
{
	using namespace DataHub;

	auto& renderFrame = renderFrames[cpuFrameIndex];
	auto& cbAreaList = cbArea_DataBox[cpuFrameIndex];


	#pragma region CountPass

	static vector<uint32_t> tidCountList;
	tidCountList.assign(cbAreaList.size(), 0);

	const uint32_t cbAreaCount = (uint32_t)cbAreaList.size();

	for (size_t l = 0; l < layerCount; l++)
	{
		auto& layer = renderFrame.layerList[l];

		if (layer.layerDataTid >= cbAreaCount)
		{
			DebugReport(DataHub::RenderReport_Contract, L"[GetData] 레이어 데이터 tid 에 cbArea 없음 (렌더러 Init 뒤 등록된 타입, 무시됨)");
			layer.layerDataTid = 0;
		}
		if (layer.layerDataTid != 0)
			tidCountList[layer.layerDataTid]++;

		for (size_t i = 1; i < layer.renderActionList.size(); i++)
		{
			auto& action = layer.renderActionList[i];

			if (action.actionDataTid >= cbAreaCount)
			{
				DebugReport(DataHub::RenderReport_Contract, L"[GetData] 액션 데이터 tid 에 cbArea 없음 (렌더러 Init 뒤 등록된 타입, 무시됨)");
				action.actionDataTid = 0;
			}
			if (action.actionDataTid != 0)
				tidCountList[action.actionDataTid]++;

			if (action.actionType != DX12_RenderActionType_Draw)
				continue;

			const uint32_t drawCount = (uint32_t)layer.actionRenderRequestList[i].size();

			for (size_t k = 0; k < DRAWREQ_DATABOX_MAX; k++)
			{
				if (action.dataTids[k] == DataBoxType_CSBuffer)
					continue;

				if (action.dataTids[k] >= cbAreaCount)
				{
					DebugReport(DataHub::RenderReport_Contract, L"[GetData] 드로우 데이터 칸 tid 에 cbArea 없음 (렌더러 Init 뒤 등록된 타입, 칸 무시됨)");
					action.dataTids[k] = 0;
				}
				if (action.dataTids[k] != 0)
					tidCountList[action.dataTids[k]] += drawCount;
			}
		}
	}

	for (size_t tid = 0; tid < cbAreaList.size(); tid++)
	{
		if (tidCountList[tid] == 0)
			continue;

		cbAreaList[tid].ReserveArea(tidCountList[tid], retiredCBAreaList);

		const uint32_t dataCount = DataBoxCustomizer::GetProvider((DataBoxTID)tid)->size;
		if (dataSlotLUTList[tid].size() < dataCount)
		{
			dataSlotLUTList[tid].resize(dataCount, 0);
			dataSlotStampList[tid].resize(dataCount, 0);
		}
	}

	#pragma endregion


	#pragma region FillPass

	const uint32_t frameStamp = (uint32_t)currentFrameCount;

	for (size_t l = 0; l < layerCount; l++)
	{
		auto& layer = renderFrame.layerList[l];


		if (layer.layerDataTid != 0)
		{
			const DataBoxTID tid = layer.layerDataTid;
			IDtBProvider* provider = DataBoxCustomizer::GetProvider(tid);

			DataBoxID id = LayerList[l].layerDataBoxId;
			if (id >= provider->size)
				id = 0;

			if (provider->size > 0)
			{
				if (dataSlotStampList[tid][id] == frameStamp)
					layer.layerRenderData.CBAreaIndex = dataSlotLUTList[tid][id];
				else
				{
					cbAreaList[tid].SetCBData(layer.layerRenderData, provider->GetData(id));
					dataSlotLUTList[tid][id] = layer.layerRenderData.CBAreaIndex;
					dataSlotStampList[tid][id] = frameStamp;
				}
			}
		}


		for (size_t i = 1; i < layer.renderActionList.size(); i++)
		{
			auto& action = layer.renderActionList[i];


			if (action.actionDataTid != 0)
			{
				const DataBoxTID tid = action.actionDataTid;
				IDtBProvider* provider = DataBoxCustomizer::GetProvider(tid);

				DataBoxID id = GetActionSource(LayerList[l], layer.renderActionNodeList[i].actionId).actionDataBoxId;
				if (id >= provider->size)
					id = 0;

				if (provider->size > 0)
				{
					if (dataSlotStampList[tid][id] == frameStamp)
						action.actionRenderData.CBAreaIndex = dataSlotLUTList[tid][id];
					else
					{
						cbAreaList[tid].SetCBData(action.actionRenderData, provider->GetData(id));
						dataSlotLUTList[tid][id] = action.actionRenderData.CBAreaIndex;
						dataSlotStampList[tid][id] = frameStamp;
					}
				}
			}

			if (action.actionType != DX12_RenderActionType_Draw)
				continue;


			auto& proxyList = layer.actionRenderRequestList[i];
			auto& sortedList = layer.actionSortedRenderDataList[i];

			const size_t drawCount = proxyList.size();
			if (drawCount == 0)
				continue;

			const RTB_ActionDraw& srcAction = static_cast<const RTB_ActionDraw&>(GetActionSource(LayerList[l], layer.renderActionNodeList[i].actionId));
			const std::vector<RTB_DrawRequest>& reqList = srcAction.drawReqList;

			#ifdef _DEBUG
			RenderId_ParseFlag psoFlag;
			const uint64_t PSOMASK = GetRenderIdProvider(action.renderIdType)->GetDrawPropIdMask(psoFlag);
			const int PSOSHIFT = PSOMASK ? countr_zero(PSOMASK) : 0;
			#endif

			for (size_t j = 0; j < drawCount; j++)
			{
				const RTB_DrawRequest& req = reqList[proxyList[j].requestIdx];
				DX12_RenderDrawData& sorted = sortedList[j];

				sorted.renderId = proxyList[j].renderId;
				sorted.meshId = req.meshId;
				sorted.subMeshId = req.subMeshId;
				for (size_t k = 0; k < DRAWREQ_DATABOX_MAX; k++)
					sorted.datas[k].CBAreaIndex = req.dataBoxIds[k];

				#ifdef _DEBUG
				const PSOID psoId = (PSOID)((sorted.renderId & PSOMASK) >> PSOSHIFT);

				if (sorted.meshId >= meshList.size())
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Mesh] 요청 meshId 범위 밖 (0 으로)");
					sorted.meshId = 0;
				}
				else if (psoId != 0 && meshList[sorted.meshId].vertexType < psoGeneratorList[psoId].psoBody.vertexType)
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Mesh] 메쉬 정점 타입이 PSO 정점 타입보다 작음 (stride 너머 읽기, 0 으로)");
					sorted.meshId = 0;
				}

				if (sorted.subMeshId >= meshViewList[sorted.meshId].subMeshes.size())
				{
					DebugReport(DataHub::RenderReport_Contract, L"[Mesh] subMeshId 범위 밖 (0 으로)");
					sorted.subMeshId = 0;
				}
				#endif
			}

			for (size_t k = 0; k < DRAWREQ_DATABOX_MAX; k++)
			{
				const DataBoxTID tid = action.dataTids[k];
				if (tid == 0)
					continue;

				if (tid == DataBoxType_CSBuffer)
				{
					for (size_t j = 0; j < drawCount; j++)
					{
						uint32_t& id = sortedList[j].datas[k].CBAreaIndex;

						if (id >= csBufferList.size() || (id < releaseCSBufferIdMask.size() && releaseCSBufferIdMask[id]))
						{
							DebugReport(DataHub::RenderReport_Contract, L"[GetData] 컴퓨트 버퍼 칸 id 범위 밖 / 해제 예정 (더미 0 으로)");
							id = 0;
						}
					}
					continue;
				}

				IDtBProvider* provider = DataBoxCustomizer::GetProvider(tid);

				const uint32_t dataCount = provider->size;
				if (dataCount == 0)
				{
					DebugReport(DataHub::RenderReport_Contract, L"[GetData] 데이터박스 타입에 데이터가 하나도 없음 (0번 빈 데이터조차, 칸 비활성)");
					action.dataTids[k] = 0;
					continue;
				}

				const uint8_t* dataBase = (const uint8_t*)provider->data;
				const size_t dataStride = provider->sizeofType;
				CBAreaRaw& cbArea = cbAreaList[tid];
				uint32_t* slotLUT = dataSlotLUTList[tid].data();
				uint32_t* slotStamp = dataSlotStampList[tid].data();

				for (size_t j = 0; j < drawCount; j++)
				{
					DX12_RenderData& slot = sortedList[j].datas[k];

					uint32_t id = slot.CBAreaIndex;
					if (id >= dataCount)
						id = 0;

					if (slotStamp[id] == frameStamp)
						slot.CBAreaIndex = slotLUT[id];
					else
					{
						cbArea.SetCBData(slot, dataBase + (size_t)id * dataStride);
						slotLUT[id] = slot.CBAreaIndex;
						slotStamp[id] = frameStamp;
					}
				}
			}
		}


	}

	#pragma endregion
}

void D3D12Renderer::UploadRenderData()
{	
	retiredCBAreaList.clear();
	ReleaseRetiredResource();


	cbArea_Condition[cpuFrameIndex]->FlushCB();
	cbArea_Light[cpuFrameIndex].FlushCB();
	cbArea_LightMatrix[cpuFrameIndex].FlushCB();

	auto& cbAreas = cbArea_DataBox[cpuFrameIndex];
	for (size_t tid = 0; tid < cbAreas.size(); tid++)
	{
		if (cbAreas[tid].GetCount() > 0)
			cbAreas[tid].FlushCB();
	}
}

#pragma endregion



#pragma region RenderCall

void D3D12Renderer::RecordActionBarriers(ID3D12GraphicsCommandList* cmdList, const DX12_RenderLayer& layer,
	const vector<DX12_ActionBarrier>& barriers, UINT frameIdx)
{
	if (barriers.empty())
		return;

	static vector<D3D12_RESOURCE_BARRIER> barrierScratch;

	const auto& rtInstants = rtTextureInstantLists[frameIdx];
	const auto& dsInstants = dsTextureInstantLists[frameIdx];

	barrierScratch.clear();

	for (const DX12_ActionBarrier& barrier : barriers)
	{
		ID3D12Resource* res;

		switch (barrier.target)
		{
		case DX12_BarrierTarget_Loaded:		res = textureList[barrier.id].Tex.Get();							break;
		case DX12_BarrierTarget_RTInstant:	res = rtInstants[barrier.id + layer.rtInstantOffset].Tex.Get();	break;
		case DX12_BarrierTarget_Buffer:		res = csBufferList[barrier.id].Res.Get();							break;
		default:							res = dsInstants[barrier.id + layer.dsInstantOffset].Tex.Get();	break;
		}

		barrierScratch.push_back(CD3DX12_RESOURCE_BARRIER::Transition(res, barrier.before, barrier.after));
	}

	cmdList->ResourceBarrier((UINT)barrierScratch.size(), barrierScratch.data());
}


void D3D12Renderer::RecordComputeAction(ID3D12GraphicsCommandList* cmdList, const DX12_RenderLayer& layer, size_t actionIdx, UINT frameIdx, D3D12_GPU_DESCRIPTOR_HANDLE defaultSRHandle)
{
	const DX12_RenderAction& action = layer.renderActionList[actionIdx];
	const auto& requestList = layer.actionRenderRequestList[actionIdx];

	if (requestList.empty())
		return;

	const RenderID packed = requestList[0].renderId;
	const ShaderID csId = (ShaderID)(packed >> 48);
	const UINT dispatchX = (UINT)((packed >> 32) & 0xFFFF);
	const UINT dispatchY = (UINT)((packed >> 16) & 0xFFFF);
	const UINT dispatchZ = (UINT)(packed & 0xFFFF);

	ID3D12PipelineState* pso = GetComputePSO(csId);

	if (pso == nullptr || dispatchX == 0 || dispatchY == 0 || dispatchZ == 0)
		return;


	cmdList->SetComputeRootSignature(computeRootSig.Get());
	cmdList->SetPipelineState(pso);


	cmdList->SetComputeRootConstantBufferView(RootParamCS_ConditionCB, cbArea_Condition[frameIdx]->GetGPUAddress(layer.conditionRenderData.CBAreaIndex));

	if (layer.layerDataTid != 0)
		cmdList->SetComputeRootConstantBufferView(RootParamCS_LayerDataCB, cbArea_DataBox[frameIdx][layer.layerDataTid].GetGPUAddress(layer.layerRenderData.CBAreaIndex));

	if (action.actionDataTid != 0)
		cmdList->SetComputeRootConstantBufferView(RootParamCS_ActionCB, cbArea_DataBox[frameIdx][action.actionDataTid].GetGPUAddress(action.actionRenderData.CBAreaIndex));


	for (size_t s = 0; s < INSTANT_SLOT_MAX; s++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = defaultSRHandle;

		if (action.srInstantIdx[s] != TEX_INSTANT_NONE)
		{
			if (s < RENDER_TARGET_MAX)
				handle = rvHeap->GetGPUHandle(rtSTexInstantLists[frameIdx][(size_t)action.srInstantIdx[s] + layer.rtInstantOffset].RVHeapIndex);
			else
				handle = rvHeap->GetGPUHandle(dsSTexInstantLists[frameIdx][(size_t)action.srInstantIdx[s] + layer.dsInstantOffset].RVHeapIndex);
		}

		cmdList->SetComputeRootDescriptorTable((UINT)(RootParamCS_InstantSR0 + s), handle);
	}


	const D3D12_GPU_VIRTUAL_ADDRESS dummyAddress = csBufferViewList[0].address;

	for (UINT k = 0; k < COMPUTE_BINDING_MAX; k++)
	{
		const size_t readSlot = CS_SLOT_READ0 + k;
		const uint16_t readId = action.slotIds[readSlot];

		if (IsCSTexture(action, readSlot))
		{
			cmdList->SetComputeRootShaderResourceView(RootParamCS_ReadBuffer0 + k, dummyAddress);
			cmdList->SetComputeRootDescriptorTable(RootParamCS_ReadTex0 + k, rvHeap->GetGPUHandle(singleTextureList[readId].RVHeapIndex));
		}
		else
		{
			cmdList->SetComputeRootShaderResourceView(RootParamCS_ReadBuffer0 + k, csBufferViewList[readId].address);
			cmdList->SetComputeRootDescriptorTable(RootParamCS_ReadTex0 + k, defaultSRHandle);
		}

		const size_t writeSlot = CS_SLOT_WRITE0 + k;
		const uint16_t writeId = action.slotIds[writeSlot];

		if (writeId == 0)
			continue;

		if (IsCSTexture(action, writeSlot))
			cmdList->SetComputeRootDescriptorTable(RootParamCS_WriteTex0 + k, rvHeap->GetGPUHandle(writableTextureList[texRefList_WTex[writeId].idx].VHeapIndex));
		else
			cmdList->SetComputeRootUnorderedAccessView(RootParamCS_WriteBuffer0 + k, csBufferViewList[writeId].address);
	}


	cmdList->Dispatch(dispatchX, dispatchY, dispatchZ);
}

#pragma endregion

}