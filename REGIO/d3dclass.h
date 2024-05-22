////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

//Unless we define this NOMINMAX, whenever we includ winowds.h its going to give an error
#define NOMINMAX 

#include <d3d11_2.h>
#include <d3dx11effect.h>
#include <DirectXMath.h>
#include <vector>
#include <sstream>
#include <wrl.h> //ComPtr
#include <d3dcompiler.h>

#include "MyException.h"
#include "DxgiInfoManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <WICTextureLoader.h>

//Light and material structs
#include "LightHelper.h"

#include "SpriteFont.h"
#include <chrono>
#include <Windows.h>

using namespace DirectX;
namespace wrl = Microsoft::WRL;

#define GFX_EXCEPT_NOINFO(hr) D3DClass::HrException( __LINE__,__FILE__,(hr) );
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw D3DClass::HrException( __LINE__,__FILE__,hr );

#ifndef NDEBUG
#define GFX_EXCEPT(hr) D3DClass::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw D3DClass::InfoException( __LINE__,__FILE__,v );}}
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D3DClass::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) D3DClass::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_THROW_INFO_ONLY(call) (call)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D3DClass::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#endif


/////////////
// STRUCTS //
/////////////

struct Vertex
{
	struct
	{
		float x;
		float y;
		float z;
	} pos;

	struct
	{
		float x;
		float y;
		float z;
	} normal;

	struct
	{
		float u;
		float v;
	} tex;

};

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
	class InfoException : public MyException
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs = {});
		const char* what() const override;
		const char* GetType() const override;
		std::string GetErrorInfo() const;

	private:
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

	bool Initialize(HWND hWnd, const aiScene* pScene);
	void Shutdown();

	void BuildGeometry(const aiScene* scene);
	void BuildVertexLayout();
	void DrawScene(const aiScene* scene, float z);
	
	void ClearBuffer(float red, float green, float blue);
	void EndScene();

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
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	//Initialize depth stencil buffer
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthTexture;

	//Scene offset for efficient use of vertex/index buffers
	UINT* pVertexOffsets;
	UINT* pIndexOffsets;
	UINT* pIndexCount;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	//Texture
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureMaxwell;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureMonkey;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureGrass;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSky;
	Microsoft::WRL::ComPtr<ID3DX11EffectShaderResourceVariable> shaderResource;


	//Effects
	Microsoft::WRL::ComPtr<ID3DX11Effect> pEffect;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechnique;
	D3DX11_PASS_DESC passDesc;

	//Temporal place to save lights and materials for the scene
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxDirLight;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxEyePos;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxTransform;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxMaterial;
	DirectionalLight dirLight;
	Material material;

	//Show fps
	std::unique_ptr<SpriteFont> spriteFont;
	std::unique_ptr <SpriteBatch> spriteBatch;
	std::chrono::time_point<std::chrono::system_clock> start;
};


#endif