////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

// Unless we define this NOMINMAX, whenever we includ winowds.h its going to give an error
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
#include "DDSTextureLoader.h"

// Light and material structs
#include "LightHelper.h"

#include "SpriteFont.h"
#include <chrono>
#include <Windows.h>

#include "Camera.h"
#include "Macros.h"
#include "ShadowMap.h"

using namespace DirectX;
namespace wrl = Microsoft::WRL;

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
	D3DClass();
	D3DClass(const D3DClass&) = delete;
	~D3DClass();

	bool Initialize(HWND hWnd, const aiScene* pScene, Camera* mainCamera);
	void Shutdown();

	void BuildGeometry(const aiScene* scene);
	void BuildVertexLayout();
	void BuildTextures(const aiScene* scene);
	void BuildSkymap();
	void DrawShadowMap(const aiScene* scene, Camera* sunCamera);
	void DrawScene(const aiScene* scene, Camera* camera);
	void DrawSky(const aiScene* scene, Camera* camera);
	void DrawDebug(const aiScene* scene, Camera* camera);
	
	void ClearBuffer(float red, float green, float blue);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	IDXGISwapChain* GetSwapChain();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	// For debugging
	Camera* camera;
	Camera* sunCamera;

private:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	// Initialize depth stencil buffer
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pNoCullRS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthTexture;

	// Scene offset for efficient use of vertex/index buffers
	UINT* pVertexOffsets;
	UINT* pIndexOffsets;
	UINT* pIndexCount;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	// Texture
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pTextures;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubemapTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureMaxwell;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureMonkey;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureGrass;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSky;
	Microsoft::WRL::ComPtr<ID3DX11EffectShaderResourceVariable> shaderResource;


	// Effects
	Microsoft::WRL::ComPtr<ID3DX11Effect> pEffect;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechniqueLight;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechniqueLightTex;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechniqueDebug;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechniqueSky;
	Microsoft::WRL::ComPtr<ID3DX11EffectTechnique> pTechniqueShadow;
	D3DX11_PASS_DESC passDesc;

	// Temporal place to save lights and materials for the scene
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxDirLight;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxPointLight;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxSpotLight;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxEyePos;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxTransform;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxTransformSkybox;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxTransformSun;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxMaterial;
	Microsoft::WRL::ComPtr<ID3DX11EffectVariable> fxPointLights;
	DirectionalLight dirLight;
	PointLight pointLight;
	PointLight pointLights[6];
	SpotLight spotLight;
	Material material;

	// Show fps
	std::unique_ptr<SpriteFont> spriteFont;
	std::unique_ptr <SpriteBatch> spriteBatch;
	std::chrono::time_point<std::chrono::system_clock> start;

	// Shadow map
	ShadowMap* pShadowMap;

	// Window
	float screenWidth;
	float screenHeight;
};


#endif