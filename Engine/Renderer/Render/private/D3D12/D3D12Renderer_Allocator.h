#pragma once

namespace Hakko
{

#pragma region Allocator

class D3D12Renderer::DescriptorHeapAllocator{
protected:
	DescriptorHeapAllocator(UINT numDescriptors);

protected:
	ComPtr<ID3D12DescriptorHeap> vHeap;
	const UINT NUM_DESCRIPTORS;
	UINT vHeapIncSize;
	UINT vHeapCount;

	CD3DX12_CPU_DESCRIPTOR_HANDLE vHeapCPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE vHeapGPUHandle;

	vector<pair<UINT, UINT>> blankedIndexList;

public:
	bool IsValid() const { return vHeap != nullptr; }
	ID3D12DescriptorHeap* GetDescriptorHeap() const { return vHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return vHeapCPUHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return vHeapGPUHandle; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT rvIndex) const { return { vHeapCPUHandle.ptr + (SIZE_T)rvIndex * vHeapIncSize }; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT rvIndex) const { return { vHeapGPUHandle.ptr + (UINT64)rvIndex * vHeapIncSize }; }

protected:
	void SortIndexList();
	
	UINT GetUsageIndexSingle();
	UINT GetUsageIndexCombined(UINT reqSize);

};


class D3D12Renderer::RVHeapAllocator : public DescriptorHeapAllocator{
public:
	RVHeapAllocator(UINT numDescriptors);

public:
	bool SetSRV(
		ID3D12Resource* subRes,
		D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc, 
		UINT& rvIndex
	);

	bool SetCombinedSRV(
		ID3D12Resource** subResList,
		D3D12_SHADER_RESOURCE_VIEW_DESC** SRVDescList,
		UINT reqSize,
		UINT& rvStartIndex
	);

	void UpdateSRV(
		UINT rvIndex,
		ID3D12Resource* subRes,
		D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc
	);

	bool SetUAV(
		ID3D12Resource* res,
		D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc,
		UINT& rvIndex
	);

	void UpdateUAV(
		UINT rvIndex,
		ID3D12Resource* res,
		D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc
	);


	void UnSetSRV(UINT rvStartIndex, UINT size = 1);

};

class D3D12Renderer::RTVHeapAllocator : public DescriptorHeapAllocator{
public:
	RTVHeapAllocator(UINT numDescriptors, UINT16 frameResCount);

private:
	const UINT16 FRAME_RES_COUNT;

public:
	bool SetRTV(
		ID3D12Resource* subRes, 
		D3D12_RENDER_TARGET_VIEW_DESC* RTVDesc, 
		UINT& rtvIndex
	);

	bool SetCombinedRTV(
		ID3D12Resource** subResList,
		D3D12_RENDER_TARGET_VIEW_DESC** RTVDescList,
		UINT reqSize,
		UINT& rtvStartIndex
	);

	void UpdateRTV(
		UINT rtvIndex,
		ID3D12Resource* subRes,
		D3D12_RENDER_TARGET_VIEW_DESC* RTVDesc
	);


	void UnSetRTV(UINT rtvStartIndex, UINT size = 1);

};

class D3D12Renderer::DSVHeapAllocator : public DescriptorHeapAllocator{
public:
	DSVHeapAllocator(UINT numDescriptors);

public:
	bool SetDSV(
		ID3D12Resource* subRes, 
		D3D12_DEPTH_STENCIL_VIEW_DESC* DSVDesc, 
		UINT& dsvIndex
	);

	bool SetCombinedDSV(
		ID3D12Resource** subResList,
		D3D12_DEPTH_STENCIL_VIEW_DESC** DSVDescList,
		UINT reqSize,
		UINT& dsvStartIndex
	);

	void UpdateDSV(
		UINT dsvIndex,
		ID3D12Resource* subRes,
		D3D12_DEPTH_STENCIL_VIEW_DESC* DSVDesc
	);


	void UnSetDSV(UINT dsvStartIndex, UINT size = 1);

};


template <typename T> class D3D12Renderer::CBAreaAllocator{
public:
	CBAreaAllocator(UINT areaNumSize, UINT areaShrinkFreq = 0, UINT areaShrinkThreshold = 0);

private:
	ComPtr<ID3D12Resource> cbArea;
	UINT8* pCBAreaData;
	D3D12_GPU_VIRTUAL_ADDRESS cbAreaGPUAddress;

	UINT AREA_MAX_NUM_SIZE;
	UINT T_DATA_NUM_SIZE;

	UINT AREA_SHRINK_FREQ;
	UINT AREA_SHRINK_THRESHOLD;
	UINT areaShrinkCount;

	uint32_t currentIndex;

	vector<T> dataCopyList;


public:
	void ReserveArea(uint32_t dataNum);

	bool SetCBData(DX12_RenderData& data, const T& rawData);

	void FlushCB();
	
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(uint32_t dataIdx);

};


class D3D12Renderer::CBAreaRaw{
public:
	CBAreaRaw() = default;

private:
	ComPtr<ID3D12Resource> cbArea;
	UINT8* pCBAreaData = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS cbAreaGPUAddress = 0;

	UINT stride = 0;
	UINT capacity = 0;
	UINT count = 0;

	UINT initCount = 0;
	UINT shrinkFreq = 0;
	UINT shrinkThreshold = 0;
	UINT shrinkCount = 0;

	std::vector<uint8_t> dataCopyList;

	void Allocate(UINT newCapacity, std::vector<ComPtr<ID3D12Resource>>& retired);

public:
	bool Init(UINT strideBytes, UINT initNum, UINT areaShrinkFreq, UINT areaShrinkThreshold);
	bool IsActive() const { return stride != 0; }
	UINT GetCount() const { return count; }
	UINT GetStride() const { return stride; }

	void ReserveArea(UINT dataNum, std::vector<ComPtr<ID3D12Resource>>& retired);
	bool SetCBData(DX12_RenderData& data, const void* rawData);
	void FlushCB();
	
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(uint32_t dataIdx) const { return cbAreaGPUAddress + (UINT64)dataIdx * stride; }
};

#pragma endregion



}


#include "D3D12Renderer_Template.h"
