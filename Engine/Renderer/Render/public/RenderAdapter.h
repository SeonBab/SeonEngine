#pragma once

#include "RenderOption.h"
#include "DataHub.h"

typedef struct HWND__* HWND;


namespace Hakko
{

struct RenderTuning
{
	uint32_t resCollectFreq = 4096;
	uint32_t texCollectThreshold = 4096;
};

class RenderAdapter
{
    
public:
    RenderAdapter() = default;
    virtual ~RenderAdapter() = default;


public:
	virtual bool Init(HWND hwnd, int width, int height) = 0;
	virtual void Finalize() = 0;

	virtual void OnResize(int width, int height) = 0;
	virtual void SetVSync(bool enable) = 0;
	virtual void SetTuning(const RenderTuning& tuning) = 0;

	virtual void Sketch() = 0;
	virtual void Render() = 0;

};

}
