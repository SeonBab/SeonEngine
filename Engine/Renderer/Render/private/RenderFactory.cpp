#include "RenderFactory.h"

#include "D3D12Core.h"
#include "D3D12Renderer.h"


namespace Hakko
{

	bool QueryRenderSpec(RenderAPI api, RenderSpec& out)
	{
		out = {};
		out.api = api;

		if (api != RenderAPI_D3D12)
			return false;

		D3D12Core& core = D3D12Core::getInstance();
		ID3D12Device* device = core.GetDevice();
		if (device == nullptr)
			return false;

		IDXGIAdapter1* adapter = core.GetAdapter();
		if (adapter != nullptr)
		{
			DXGI_ADAPTER_DESC1 desc = {};
			if (SUCCEEDED(adapter->GetDesc1(&desc)))
			{
				out.dedicatedVideoMemory = desc.DedicatedVideoMemory;
				wcsncpy_s(out.adapterName, desc.Description, _TRUNCATE);
			}
		}

		const D3D_SHADER_MODEL candidates[] = {
			D3D_SHADER_MODEL_6_6, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_4,
			D3D_SHADER_MODEL_6_3, D3D_SHADER_MODEL_6_2, D3D_SHADER_MODEL_6_1, D3D_SHADER_MODEL_6_0 };

		for (size_t i = 0; i < _countof(candidates); i++)
		{
			D3D12_FEATURE_DATA_SHADER_MODEL sm = { candidates[i] };
			if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &sm, sizeof(sm))))
			{
				out.shaderModelMajor = (uint32_t)sm.HighestShaderModel >> 4;
				out.shaderModelMinor = (uint32_t)sm.HighestShaderModel & 0xF;
				break;
			}
		}

		{
			ComPtr<IDXGIFactory5> factory5;
			BOOL allowTearing = FALSE;
			if (SUCCEEDED(core.GetFactory()->QueryInterface(IID_PPV_ARGS(&factory5))) &&
				SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
				out.tearingSupported = allowTearing == TRUE;
		}

		out.available = out.shaderModelMajor >= 6;

		return out.available;
	}


	RenderAdapter* CreateRenderer(const RendererDesc& desc)
	{
		switch (desc.api)
		{
		case RenderAPI_D3D12:
		{
			D3D12Renderer* renderer = new D3D12Renderer(desc.limits);
			renderer->SetVSync(desc.vsync);
			renderer->SetTuning(desc.tuning);
			return renderer;
		}
		}

		return nullptr;
	}

	void DestroyRenderer(RenderAdapter* renderer)
	{
		delete renderer;
	}

}
