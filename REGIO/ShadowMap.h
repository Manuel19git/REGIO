#pragma once

#include <d3d11_2.h>
#include <wrl.h> //ComPtr
#include "Common/Macros.h"
#include "DxgiInfoManager.h"
#include "MyException.h"

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* device, float width, float height);
	~ShadowMap();

	void BindDSVandNullTarget(ID3D11DeviceContext* deviceContext);


	float width;
	float height;
	D3D11_VIEWPORT viewport;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pSunTarget;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSunShaderResourceView;

#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
};
