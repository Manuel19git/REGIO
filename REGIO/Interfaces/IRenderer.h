#pragma once


// Unless we define this NOMINMAX, whenever we includ winowds.h its going to give an error
#define NOMINMAX 
#include "Windows.h" // HWND
#include "wrl.h"
#include "../Common/RenderItem.h"
#include "../Common/Profiler.h"

namespace wrl = Microsoft::WRL;

class IRenderer
{
	// This is not needed if we end up implementing render passes
	//virtual void DrawItem(RenderItem& renderItem) = 0;
	virtual void ConfigureRenderPass(HWND hWnd, float screenWidth, float screenHeight) = 0;


	virtual void BeginRenderFrame() = 0;
	virtual void EndRenderFrame() = 0;


	// I am still not sure if this should go here
	virtual void DrawSky() = 0;


	// Externally used methods? I don't know if this is final
	//virtual bool CreateBuffer(void* data, size_t dataSize, size_t byteSize, IGPUBuffer& buffer) = 0; //DX11 specific
	virtual bool ClearBuffer() = 0;

};
