#pragma once
#include <cstdint>
#include <DirectXMath.h>

using namespace DirectX;

struct RenderItem
{
	uint32_t meshHandle;
	uint32_t materialHandle;
	XMMATRIX worldTransform; // Putting XMMATRIX I am making the engine use DirectX, I should have a generic type
};
