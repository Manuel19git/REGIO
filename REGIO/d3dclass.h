////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <sstream>

#include "MyException.h"

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
	class HrException : public MyException
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {});
		const char* what() const override;
		const char* GetType() const override;
		HRESULT GetErrorCode() const;
		std::string GetErrorString() const;
		std::string GetErrorDescription() const;

	private:
		HRESULT hr;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const override;
	};

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

#define D3_EXCEPT(hr) D3DClass::HrException(__LINE__, __FILE__, hr);

#endif