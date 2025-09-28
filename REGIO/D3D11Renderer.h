#pragma once

#include "Interfaces/IRenderer.h"

// DirectX11 includes
#include "DX11Buffer.h"
#include <d3d11_2.h>
#include "MyException.h"
#include "DxgiInfoManager.h"


class D3D11Renderer : public IRenderer
{
public:
	D3D11Renderer();
	~D3D11Renderer();

	//void DrawItem(RenderItem& renderItem) override;
	void EndScene() override;
	void DrawSky() override;

	bool CreateBuffer(const void* data, ID3D11Buffer** outBuffer, D3D11_BUFFER_DESC bufferDesc);
	bool ClearBuffer() override;

#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
};
