#pragma once

#include <string>
#include <cstdint>
#include "Common.h"

struct RenderItem
{
	std::string meshHandle;
	std::string materialHandle;
	Matrix4x4 worldTransform; 
};
