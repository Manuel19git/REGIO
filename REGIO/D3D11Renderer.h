#pragma once

#include "Interfaces/IRenderer.h"
#include "Common/Common.h"
#include "Common/LightHelper.h"
#include "Common/Macros.h"

// DirectX11 includes
#include <d3d11_2.h>
#include <d3dcompiler.h>
#include "MyException.h"
#include "DxgiInfoManager.h"
#include <WICTextureLoader.h>
#include "DDSTextureLoader.h"

// Debugging
#include "SpriteFont.h"
#include <chrono>

using namespace DirectX;

class D3D11Renderer : public IRenderer
{
public:
	D3D11Renderer(HWND hwnd);
	~D3D11Renderer();

	//void DrawItem(RenderItem& renderItem) override;
	void ConfigureRenderPass(HWND hwnd, float screenWidth, float screenHeight) override;

	// I don't like having to define a method for each action done with the API 
	// (TODO: rework this in the future with a little bit more of knowledge)
	void SetVertexLayoutAndTopology(ID3D11InputLayout* inputLayout, D3D_PRIMITIVE_TOPOLOGY topology);
	void SetMeshBuffers(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer);
	void SetObjectConstantBufferVS(const void* bufferData, size_t bufferSize, int registerId);
	void SetObjectConstantBufferPS(const void* bufferData, size_t bufferSize, int registerId);
	void SetFrameConstantBufferPS(const void* bufferData, size_t bufferSize, int registerId);
	void SetTextureAndSamplerResourcePS(ID3D11ShaderResourceView* textureResource, int registerId, ID3D11SamplerState* samplerState);
	void SetShaders(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader);
	void DrawItem(uint32_t indexCount);


	void BeginRenderFrame() override;
	void EndRenderFrame() override;
	void DrawSky() override;

	bool CreateBuffer(const void* data, ID3D11Buffer** outBuffer, D3D11_BUFFER_DESC bufferDesc);
	bool CreateVertexShader(std::string shaderPath, ID3D11VertexShader** pVertexShader, ID3D11InputLayout** pInputLayout);
	bool CreatePixelShader(std::string shaderPath, ID3D11PixelShader** pPixelShader);
	bool CreateTexture(std::string texturePath, ID3D11ShaderResourceView** textureResourceView);
	bool CreateDDSTexture(std::string texturePath, ID3D11ShaderResourceView** textureResourceView);
	bool CreateSamplerState(D3D11_FILTER filterMode, D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_COMPARISON_FUNC compFunc, ID3D11SamplerState** samplerState);

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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pShadowSamplerState;

	// States
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pNoCullRS;

	// Constant buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> pObjectConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pFrameConstantBuffer;

	// Show fps
	std::unique_ptr<SpriteFont> spriteFont;
	std::unique_ptr <SpriteBatch> spriteBatch;
	std::chrono::time_point<std::chrono::system_clock> start;

	int monitorWidth;
	int monitorHeight;
	int screenWidth;
	int screenHeight;
	D3D11_VIEWPORT renderViewport;

};
