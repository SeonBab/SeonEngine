#include "D3D12Renderer.h"

#include "D3D12Core.h"

#include <cmath>
#include <algorithm>


namespace Hakko
{

#pragma region Allocator

#pragma region Helper

#ifdef _DEBUG
static void CheckBlankOverlap(const std::vector<std::pair<UINT, UINT>>& blankList, UINT startIndex, UINT size)
{
	for (const auto& blank : blankList)
	{
		if (startIndex < blank.second + blank.first && blank.second < startIndex + size)
		{
			DebugReport(DataHub::RenderReport_Invariant, L"[Heap] 이미 반납된 디스크립터 칸을 또 반납");
			return;
		}
	}
}
#endif

#pragma endregion


#pragma region HeapAllocator

#pragma region AllocatorBase

D3D12Renderer::DescriptorHeapAllocator::DescriptorHeapAllocator(UINT numDescriptors)
	: vHeapCount(0), NUM_DESCRIPTORS(numDescriptors)
{
}


void D3D12Renderer::DescriptorHeapAllocator::SortIndexList()
{
	std::sort(blankedIndexList.begin(), blankedIndexList.end(), 
		[](const auto& a, const auto& b) {
			return a.first > b.first;
		});
}

UINT D3D12Renderer::DescriptorHeapAllocator::GetUsageIndexSingle()
{
	if (blankedIndexList.size() > 0)
	{
		SortIndexList();

		auto [size, startIndex] = blankedIndexList.back();
		blankedIndexList.pop_back();

		if (size > 1)
		{
			blankedIndexList.emplace_back(size - 1, startIndex + 1);
		}

		return startIndex;
	}
	else
	{
		if (vHeapCount >= NUM_DESCRIPTORS)
		{
			DebugReport(DataHub::RenderReport_Limit, L"[Heap] 디스크립터 힙 꽉참");
			return NUM_DESCRIPTORS;
		}

		UINT t = vHeapCount;
		vHeapCount++;

		return t;
	}
}

UINT D3D12Renderer::DescriptorHeapAllocator::GetUsageIndexCombined(UINT reqSize)
{

	if (blankedIndexList.size() )
	{
		SortIndexList();

		auto rit = std::lower_bound(blankedIndexList.rbegin(), blankedIndexList.rend(), reqSize, 
			[](const auto& element, UINT val) {
				return element.first < val;
			});


		if (rit != blankedIndexList.rend())
		{
			auto it = rit.base() - 1;

			auto [size, startIndex] = *it;
			blankedIndexList.erase(it);

			if (size > reqSize)
			{
				blankedIndexList.emplace_back(size - reqSize, startIndex + reqSize);
			}
			
			return startIndex;
		}
		else
		{
			if (vHeapCount >= NUM_DESCRIPTORS)
			{
				DebugReport(DataHub::RenderReport_Limit, L"[Heap] 디스크립터 힙 꽉참");
				return NUM_DESCRIPTORS;
			}

			UINT t = vHeapCount;
			vHeapCount += reqSize;

			return t;
		}
	}
	else
	{
		if (vHeapCount >= NUM_DESCRIPTORS)
		{
			DebugReport(DataHub::RenderReport_Limit, L"[Heap] 디스크립터 힙 꽉참");
			return NUM_DESCRIPTORS;
		}

		UINT t = vHeapCount;
		vHeapCount += reqSize;

		return t;
	}
}

#pragma endregion


#pragma region RVHeapAllocator

D3D12Renderer::RVHeapAllocator::RVHeapAllocator(UINT numDescriptors)
	: DescriptorHeapAllocator(numDescriptors)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	D3D12_DESCRIPTOR_HEAP_DESC rvHeapDesc = {};
	rvHeapDesc.NumDescriptors = NUM_DESCRIPTORS;
	rvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	rvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	
	if (FAILED(device->CreateDescriptorHeap(&rvHeapDesc, IID_PPV_ARGS(&vHeap))))
		return;

	vHeapIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	vHeapCPUHandle = vHeap->GetCPUDescriptorHandleForHeapStart();
	vHeapGPUHandle = vHeap->GetGPUDescriptorHandleForHeapStart();
}


bool D3D12Renderer::RVHeapAllocator::SetSRV(
	ID3D12Resource *subRes, D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc, UINT &rvIndex)
{
	UINT usageIndex = GetUsageIndexSingle();
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;
	
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();
	

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = vHeapCPUHandle;
	srvCPUHandle.Offset(usageIndex, vHeapIncSize);
	device->CreateShaderResourceView(subRes, SRVDesc, srvCPUHandle);

	rvIndex = usageIndex;


	return true;
}

bool D3D12Renderer::RVHeapAllocator::SetCombinedSRV(ID3D12Resource **subResList, D3D12_SHADER_RESOURCE_VIEW_DESC **SRVDescList, UINT reqSize, UINT &rvStartIndex)
{
	UINT usageIndex = GetUsageIndexCombined(reqSize);
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;


	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = vHeapCPUHandle;
	srvCPUHandle.Offset(usageIndex, vHeapIncSize);

	for (UINT i = 0; i < reqSize; i++)
	{
		device->CreateShaderResourceView(subResList[i], SRVDescList[i], srvCPUHandle);
		srvCPUHandle.Offset(1, vHeapIncSize);

	}

	rvStartIndex = usageIndex;
	

	return true;
}

void D3D12Renderer::RVHeapAllocator::UpdateSRV(UINT rvIndex, ID3D12Resource *subRes, D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = vHeapCPUHandle;
	srvCPUHandle.Offset(rvIndex, vHeapIncSize);
	device->CreateShaderResourceView(subRes, SRVDesc, srvCPUHandle);

}


bool D3D12Renderer::RVHeapAllocator::SetUAV(ID3D12Resource* res, D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc, UINT& rvIndex)
{
	UINT usageIndex = GetUsageIndexSingle();
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE uavCPUHandle = vHeapCPUHandle;
	uavCPUHandle.Offset(usageIndex, vHeapIncSize);
	device->CreateUnorderedAccessView(res, nullptr, UAVDesc, uavCPUHandle);


	rvIndex = usageIndex;


	return true;
}

void D3D12Renderer::RVHeapAllocator::UpdateUAV(UINT rvIndex, ID3D12Resource* res, D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE uavCPUHandle = vHeapCPUHandle;
	uavCPUHandle.Offset(rvIndex, vHeapIncSize);
	device->CreateUnorderedAccessView(res, nullptr, UAVDesc, uavCPUHandle);
}

void D3D12Renderer::RVHeapAllocator::UnSetSRV(UINT rvStartIndex, UINT size)
{
	#ifdef _DEBUG
	CheckBlankOverlap(blankedIndexList, rvStartIndex, size);
	#endif

	blankedIndexList.emplace_back(size, rvStartIndex);
}

#pragma endregion


#pragma region RTVHeapAllocator

D3D12Renderer::RTVHeapAllocator::RTVHeapAllocator(UINT numDescriptors, UINT16 frameResCount)
	: DescriptorHeapAllocator(numDescriptors), FRAME_RES_COUNT(frameResCount)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = NUM_DESCRIPTORS;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&vHeap))))
		return;

	vHeapIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	vHeapCPUHandle = vHeap->GetCPUDescriptorHandleForHeapStart();
	vHeapGPUHandle = {};
}


bool D3D12Renderer::RTVHeapAllocator::SetRTV(ID3D12Resource *subRes, D3D12_RENDER_TARGET_VIEW_DESC *RTVDesc, UINT &rtvIndex)
{
	UINT usageIndex = GetUsageIndexSingle();
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;
	

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();
	

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = vHeapCPUHandle;
	rtvCPUHandle.Offset(usageIndex, vHeapIncSize);
	device->CreateRenderTargetView(subRes, RTVDesc, rtvCPUHandle);


	rtvIndex = usageIndex;

	return true;
}

bool D3D12Renderer::RTVHeapAllocator::SetCombinedRTV(ID3D12Resource **subResList, D3D12_RENDER_TARGET_VIEW_DESC **RTVDescList, UINT reqSize, UINT &rtvStartIndex)
{
	UINT usageIndex = GetUsageIndexCombined(reqSize);
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;
	

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = vHeapCPUHandle;
	rtvCPUHandle.Offset(usageIndex, vHeapIncSize);

	for (UINT i = 0; i < reqSize; i++)
	{
		device->CreateRenderTargetView(subResList[i], RTVDescList[i], rtvCPUHandle);
		rtvCPUHandle.Offset(1, vHeapIncSize);
	}

	rtvStartIndex = usageIndex;

	return true;
}

void D3D12Renderer::RTVHeapAllocator::UpdateRTV(UINT rtvIndex, ID3D12Resource *subRes, D3D12_RENDER_TARGET_VIEW_DESC *RTVDesc)
{
	if (rtvIndex == 0)
	{
		DebugReport(DataHub::RenderReport_Invariant, L"[Heap] 예약 RTV 칸 갱신 시도");
		return;
	}

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = vHeapCPUHandle;
	rtvCPUHandle.Offset(rtvIndex, vHeapIncSize);
	device->CreateRenderTargetView(subRes, RTVDesc, rtvCPUHandle);
}


void D3D12Renderer::RTVHeapAllocator::UnSetRTV(UINT rtvStartIndex, UINT size)
{
	if (rtvStartIndex < FRAME_RES_COUNT + 1)
	{
		DebugReport(DataHub::RenderReport_Invariant, L"[Heap] 예약 RTV 칸 해제 시도");
		return;
	}

	#ifdef _DEBUG
	CheckBlankOverlap(blankedIndexList, rtvStartIndex, size);
	#endif

	blankedIndexList.emplace_back(size, rtvStartIndex);
}

#pragma endregion


#pragma region DSVHeapAllocator

D3D12Renderer::DSVHeapAllocator::DSVHeapAllocator(UINT numDescriptors)
	: DescriptorHeapAllocator(numDescriptors)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = NUM_DESCRIPTORS;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&vHeap))))
		return;

	vHeapIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	vHeapCPUHandle = vHeap->GetCPUDescriptorHandleForHeapStart();
	vHeapGPUHandle = {};
}


bool D3D12Renderer::DSVHeapAllocator::SetDSV(ID3D12Resource *subRes, D3D12_DEPTH_STENCIL_VIEW_DESC *DSVDesc, UINT &dsvIndex)
{
	UINT usageIndex = GetUsageIndexSingle();
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;
	

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();
	

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = vHeapCPUHandle;
	dsvCPUHandle.Offset(usageIndex, vHeapIncSize);
	device->CreateDepthStencilView(subRes, DSVDesc, dsvCPUHandle);


	dsvIndex = usageIndex;

	return true;
}

bool D3D12Renderer::DSVHeapAllocator::SetCombinedDSV(ID3D12Resource **subResList, D3D12_DEPTH_STENCIL_VIEW_DESC **DSVDescList, UINT reqSize, UINT &dsvStartIndex)
{
	UINT usageIndex = GetUsageIndexCombined(reqSize);
	if (usageIndex >= NUM_DESCRIPTORS)
		return false;
	

	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = vHeapCPUHandle;
	dsvCPUHandle.Offset(usageIndex, vHeapIncSize);

	for (UINT i = 0; i < reqSize; i++)
	{
		device->CreateDepthStencilView(subResList[i], DSVDescList[i], dsvCPUHandle);
		dsvCPUHandle.Offset(1, vHeapIncSize);
	}

	dsvStartIndex = usageIndex;

	return true;
}

void D3D12Renderer::DSVHeapAllocator::UpdateDSV(UINT dsvIndex, ID3D12Resource *subRes, D3D12_DEPTH_STENCIL_VIEW_DESC *DSVDesc)
{
	ID3D12Device* device = D3D12Core::getInstance().GetDevice();


	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = vHeapCPUHandle;
	dsvCPUHandle.Offset(dsvIndex, vHeapIncSize);
	device->CreateDepthStencilView(subRes, DSVDesc, dsvCPUHandle);
}


void D3D12Renderer::DSVHeapAllocator::UnSetDSV(UINT dsvStartIndex, UINT size)
{
	#ifdef _DEBUG
	CheckBlankOverlap(blankedIndexList, dsvStartIndex, size);
	#endif

	blankedIndexList.emplace_back(size, dsvStartIndex);
}

#pragma endregion

#pragma endregion



#pragma region CBAreaRaw

void D3D12Renderer::CBAreaRaw::Allocate(UINT newCapacity, std::vector<ComPtr<ID3D12Resource>>& retired)
{
	auto device = D3D12Core::getInstance().GetDevice();

	if (cbArea != nullptr)
	{
		cbArea->Unmap(0, nullptr);
		retired.push_back(std::move(cbArea));
		cbArea = nullptr;
	}

	pCBAreaData = nullptr;
	cbAreaGPUAddress = 0;
	capacity = 0;


	const CD3DX12_RANGE readRange(0, 0);
	ComPtr<ID3D12Resource> CB;
	UINT8* pCBBegin = nullptr;

	const CD3DX12_HEAP_PROPERTIES UPLOAD_HEAP_PROPS(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)newCapacity * stride);

	HRESULT hr = device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&CB)
	);

	if (FAILED(hr))
	{
		DebugReport(DataHub::RenderReport_Limit, L"[CBArea] 상수버퍼 영역 할당 실패 (이 타입은 이번 프레임 비활성)");
		return;
	}

	CB->Map(0, &readRange, (void**)&pCBBegin);


	cbArea = std::move(CB);
	pCBAreaData = pCBBegin;
	cbAreaGPUAddress = cbArea->GetGPUVirtualAddress();
	capacity = newCapacity;

	dataCopyList.resize((size_t)capacity * stride);
	shrinkCount = 0;
}

bool D3D12Renderer::CBAreaRaw::Init(UINT strideBytes, UINT initNum, UINT areaShrinkFreq, UINT areaShrinkThreshold)
{
	if (strideBytes == 0 || strideBytes % 256 != 0)
	{
		stride = 0;
		return false;
	}

	stride = strideBytes;
	initCount = initNum > 0 ? initNum : 1;
	shrinkFreq = areaShrinkFreq;
	shrinkThreshold = areaShrinkThreshold > 1 ? areaShrinkThreshold : 2;
	count = 0;

	std::vector<ComPtr<ID3D12Resource>> none;
	Allocate(initCount, none);

	return capacity != 0;
}

void D3D12Renderer::CBAreaRaw::ReserveArea(UINT dataNum, std::vector<ComPtr<ID3D12Resource>>& retired)
{
	if (stride == 0)
		return;

	UINT wantCapacity = dataNum + dataNum / 8;
	if (wantCapacity < initCount)
		wantCapacity = initCount;


	if (dataNum > capacity)
	{
		Allocate(wantCapacity, retired);
		return;
	}

	if (shrinkFreq == 0 || wantCapacity >= capacity)
	{
		shrinkCount = 0;
		return;
	}

	if ((UINT64)dataNum * shrinkThreshold < capacity)
	{
		shrinkCount++;
		if (shrinkCount >= shrinkFreq)
			Allocate(wantCapacity, retired);
	}
	else
		shrinkCount = 0;
}

bool D3D12Renderer::CBAreaRaw::SetCBData(DX12_RenderData& data, const void* rawData)
{
	if (stride == 0 || count >= capacity)
	{
		return false;
	}

	memcpy(dataCopyList.data() + (size_t)count * stride, rawData, stride);

	data.CBAreaIndex = count;

	count++;

	return true;
}

void D3D12Renderer::CBAreaRaw::FlushCB()
{
	if (count > 0 && pCBAreaData != nullptr)
		memcpy(pCBAreaData, dataCopyList.data(), (size_t)count * stride);

	count = 0;
}

#pragma endregion

#pragma endregion

}