#pragma once

#include "IRenderer.h"
#include "../Common/RenderItem.h"

class IRenderPass
{
public:

	virtual void setup() = 0;
	virtual void execute(IRenderer* renderer, const std::vector<RenderItem>& items) = 0;

};
