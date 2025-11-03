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

	// Should i put inside the mesh the transformation matrix? why not
	wrl::ComPtr<ID3D11Buffer> transformationBuffer;
};

struct DX11Material
{
	// material for scene and material for dx11? Or unified
	/*
	shader path (if any)
	ID3D11VertexShader/ID3D11PixelShader

	texture path (if any)
	ID3D11ShaderResourceView

	parameters (if any apply but i can put default ones)
	constant buffer?

	Parece que si que necesito separar una de la otra. Material para la escena y material para gpu

	InputLayout donde?
	Ese deber�a ir en el material para gpu creado usando el resourceManager
	*/
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;

	wrl::ComPtr<ID3D11ShaderResourceView> pDiffuseTexture;
	wrl::ComPtr<ID3D11ShaderResourceView> pSpecularTexture;
	wrl::ComPtr<ID3D11ShaderResourceView> pNormalTexture;

	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
};
