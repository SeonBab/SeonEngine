#include "D3D12Renderer.h"


namespace Hakko
{

#pragma region FrameControl

void D3D12Renderer::WaitGPUProcess()
{
	if (fence->GetCompletedValue() < frameFenceValues[cpuFrameIndex])
	{
		fence->SetEventOnCompletion(frameFenceValues[cpuFrameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void D3D12Renderer::SignalFence()
{
	renderCMDQueue->Signal(fence.Get(), currentFenceValue);
	frameFenceValues[cpuFrameIndex] = currentFenceValue;

	currentFenceValue++;
}

#pragma endregion



#pragma region LayerControl

bool D3D12Renderer::InitCanvasLayer()
{
	for (size_t i = 0; i < FRAME_COUNT; i++)
	{
		renderFrames[i].layerList.assign(RENDER_LAYER_MAX, {});
	}

	layerCount = 1;


	return true;
}

void D3D12Renderer::UpdateCanvasLayer()
{
	size_t count = DataHub::LayerList.size();

	if (count > RENDER_LAYER_MAX)
	{
		DebugReport(DataHub::RenderReport_Contract, L"[Layer] 레이어 수가 RENDER_LAYER_MAX 를 넘음 (넘치는 레이어는 안 그림)");
		count = RENDER_LAYER_MAX;
	}

	layerCount = (UINT8)count;
}

#pragma endregion

}
