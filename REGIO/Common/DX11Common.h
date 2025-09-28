#pragma once

#include <d3d11_2.h>
#include <wrl.h> //ComPtr

namespace wrl = Microsoft::WRL;

struct DX11Mesh
{
	wrl::ComPtr<ID3D11Buffer> vertexBuffer;
	wrl::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;

	// Should i put inside the mesh the transformation matrix? why not
	wrl::ComPtr<ID3D11Buffer> transformationBuffer;
};
