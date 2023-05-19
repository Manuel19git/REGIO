////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

/////////////
// GLOBALS //
/////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: D3DClass
////////////////////////////////////////////////////////////////////////////////
class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&) = delete;
	~D3DClass();

	bool Initialize(HWND hWnd);
	void Shutdown();

	void BeginScene();
	void EndScene();
	void ClearBuffer(float red, float green, float blue) noexcept
	{
		const float color[] = {red, green, blue, 1.0f};
		pDeviceContext->ClearRenderTargetView(pTarget, color);
	}

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	IDXGISwapChain* GetSwapChain();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);


private:
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	IDXGISwapChain* pSwap;
	ID3D11RenderTargetView* pTarget;
};

#endif