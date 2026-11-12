#pragma once

#include <cstdint>

#include "RenderAdapter.h"


namespace Hakko
{

	enum RenderAPI
	{
		RenderAPI_D3D12 = 0,
	};

	struct RenderLimits
	{
		uint32_t rvDescriptors = 262144;
		uint32_t rtvDescriptors = 1024;
		uint32_t dsvDescriptors = 1024;
		uint32_t cbAreaInitNum = 1024;
		uint32_t cbAreaShrinkFreq = 180;
		uint32_t cbAreaShrinkThreshold = 4;
	};

	struct RendererDesc
	{
		RenderAPI api = RenderAPI_D3D12;
		bool vsync = true;
		RenderLimits limits;
		RenderTuning tuning;
	};

	struct RenderSpec
	{
		RenderAPI api = RenderAPI_D3D12;
		bool available = false;
		uint32_t shaderModelMajor = 0;
		uint32_t shaderModelMinor = 0;
		bool tearingSupported = false;
		uint64_t dedicatedVideoMemory = 0;
		wchar_t adapterName[128] = {};
	};


	bool QueryRenderSpec(RenderAPI api, RenderSpec& out);

	RenderAdapter* CreateRenderer(const RendererDesc& desc);
	void DestroyRenderer(RenderAdapter* renderer);

}
