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
#include <wrl.h> //ComPtr

#include "MyException.h"
#include "DxgiInfoManager.h"

using namespace DirectX;

#define GFX_EXCEPT_NOINFO(hr) D3DClass::HrException( __LINE__,__FILE__,(hr) );
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw D3DClass::HrException( __LINE__,__FILE__,hr );

#ifndef NDEBUG
#define GFX_EXCEPT(hr) D3DClass::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D3DClass::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) D3DClass::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D3DClass::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#endif


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
		std::string GetErrorInfo() const;

	private:
		HRESULT hr;
		std::string info;
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
	void ClearBuffer(float red, float green, float blue);

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	IDXGISwapChain* GetSwapChain();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);


private:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
};


#endif