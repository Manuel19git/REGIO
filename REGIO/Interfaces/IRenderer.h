#pragma once
#include "Common/RenderItem.h"

class IRenderer
{
	virtual void DrawItem(RenderItem& renderItem) = 0;


	virtual void EndScene() = 0;


	// I am still not sure if this should go here
	virtual void DrawSky() = 0;


	// Externally used methods? I don't know if this is final
	virtual bool CreateBuffer() = 0;
	virtual bool ClearBuffer() = 0;

};
