#pragma once

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <wrl.h> //ComPtr

namespace wrl = Microsoft::WRL;

struct DX11Mesh
{
	wrl::ComPtr<ID3D11Buffer> vertexBuffer;
	wrl::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;
};

struct DX11Material
{
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;

	// For now we assume only one texture per type in an object
	wrl::ComPtr<ID3D11ShaderResourceView> pDiffuseTexture;
	wrl::ComPtr<ID3D11ShaderResourceView> pSpecularTexture;
	wrl::ComPtr<ID3D11ShaderResourceView> pNormalTexture;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pShadowSamplerState;

	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 reflect;
};
