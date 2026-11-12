#pragma once

#include "D3D12Core.h"

// 일반화 전용


namespace Hakko
{

#pragma region CBAreaAllocator

template <typename T>
D3D12Renderer::CBAreaAllocator<T>::CBAreaAllocator(UINT areaNumSize, UINT areaShrinkFreq, UINT areaShrinkThreshold)
	: AREA_MAX_NUM_SIZE(areaNumSize), T_DATA_NUM_SIZE(sizeof(T) / 256), 
	AREA_SHRINK_FREQ(areaShrinkFreq), AREA_SHRINK_THRESHOLD(areaShrinkThreshold), areaShrinkCount(0),
	currentIndex(0)
{

	if (areaNumSize < T_DATA_NUM_SIZE)
		return;
	
	auto device = D3D12Core::getInstance().GetDevice();

	const CD3DX12_RANGE readRange(0, 0);
	ComPtr<ID3D12Resource> CB;
	UINT8* pCBBegin;


	const CD3DX12_HEAP_PROPERTIES UPLOAD_HEAP_PROPS(D3D12_HEAP_TYPE_UPLOAD);
	auto resDec = CD3DX12_RESOURCE_DESC::Buffer(AREA_MAX_NUM_SIZE * 256);
	
	device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&resDec,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&CB)
	);

	CB->Map(0, &readRange, (void**)&pCBBegin);
	const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = CB->GetGPUVirtualAddress();
	

	cbArea = std::move(CB);
	pCBAreaData = pCBBegin;
	cbAreaGPUAddress = gpuAddress;

	dataCopyList.reserve(AREA_MAX_NUM_SIZE / T_DATA_NUM_SIZE);
}


template <typename T>
void D3D12Renderer::CBAreaAllocator<T>::ReserveArea(uint32_t dataNum)
{

	const uint32_t areaNum = AREA_MAX_NUM_SIZE / T_DATA_NUM_SIZE;

	if (areaNum < dataNum)
	{
		if (AREA_SHRINK_FREQ == 0)
			return;

			
		if (dataNum / areaNum < AREA_SHRINK_THRESHOLD)
			areaShrinkCount++;

		if (areaShrinkCount < AREA_SHRINK_FREQ)
			return;
	}
	

	auto device = D3D12Core::getInstance().GetDevice();

	const CD3DX12_RANGE readRange(0, 0);
	ComPtr<ID3D12Resource> CB;
	UINT8* pCBBegin;

	AREA_MAX_NUM_SIZE = dataNum * T_DATA_NUM_SIZE * 1.125f;

	const CD3DX12_HEAP_PROPERTIES UPLOAD_HEAP_PROPS(D3D12_HEAP_TYPE_UPLOAD);
	auto resDec = CD3DX12_RESOURCE_DESC::Buffer(AREA_MAX_NUM_SIZE * 256);
	
	device->CreateCommittedResource(
		&UPLOAD_HEAP_PROPS,
		D3D12_HEAP_FLAG_NONE,
		&resDec,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&CB)
	);

	CB->Map(0, &readRange, (void**)&pCBBegin);
	const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = CB->GetGPUVirtualAddress();
	

	cbArea = std::move(CB);
	pCBAreaData = pCBBegin;
	cbAreaGPUAddress = gpuAddress;

	dataCopyList.reserve(AREA_MAX_NUM_SIZE / T_DATA_NUM_SIZE);

	areaShrinkCount = 0;
}

template <typename T>
bool D3D12Renderer::CBAreaAllocator<T>::SetCBData(DX12_RenderData& data, const T& rawData)
{
	using namespace DataHub;

	if ((AREA_MAX_NUM_SIZE - currentIndex) < T_DATA_NUM_SIZE)
	{
		DebugReport(DataHub::RenderReport_Invariant, L"[CBArea] 영역 꽉참 (ReserveArea 누락)");
		return false;
	}

	dataCopyList.emplace_back(rawData);

	data.CBAreaIndex = currentIndex;

	currentIndex += T_DATA_NUM_SIZE;

	return true;
}

template <typename T>
void D3D12Renderer::CBAreaAllocator<T>::FlushCB()
{

	UINT8* pDataBegin = reinterpret_cast<UINT8*>(dataCopyList.data());
	memcpy(pCBAreaData, pDataBegin, AREA_MAX_NUM_SIZE * 256);


	dataCopyList.clear();
	
	currentIndex = 0;
}

template <typename T>
D3D12_GPU_VIRTUAL_ADDRESS D3D12Renderer::CBAreaAllocator<T>::GetGPUAddress(uint32_t dataIdx)
{
	const UINT offset = dataIdx * 256;

	return cbAreaGPUAddress + offset;
}

#pragma endregion

}