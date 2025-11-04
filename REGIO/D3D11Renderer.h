#pragma once

#include "Interfaces/IRenderer.h"
#include "Common/Common.h"

// DirectX11 includes
#include "DX11Buffer.h"
#include <d3d11_2.h>
#include <d3dcompiler.h>
#include "MyException.h"
#include "DxgiInfoManager.h"
#include <WICTextureLoader.h>

// Debugging
#include "SpriteFont.h"
#include <chrono>

using namespace DirectX;

class D3D11Renderer : public IRenderer
{
public:
	D3D11Renderer();
	~D3D11Renderer();

	//void DrawItem(RenderItem& renderItem) override;
	void ConfigureRenderPass(HWND hWnd) override;
	void BeginRenderPass() override;
	void EndRenderPass() override;
	void DrawSky() override;

	bool CreateBuffer(const void* data, ID3D11Buffer** outBuffer, D3D11_BUFFER_DESC bufferDesc);
	bool CreateVertexShader(std::string shaderPath, ID3D11VertexShader* pVertexShader, ID3D11InputLayout* pInputLayout);
	bool CreatePixelShader(std::string shaderPath, ID3D11PixelShader* pPixelShader);
	bool CreateTexture(std::string texturePath, ID3D11ShaderResourceView* textureResourceView);
	bool ClearBuffer() override;

#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;

	//Renderer should have views right?
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthBuffer;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;

	// Show fps
	std::unique_ptr<SpriteFont> spriteFont;
	std::unique_ptr <SpriteBatch> spriteBatch;
	std::chrono::time_point<std::chrono::system_clock> start;

};
