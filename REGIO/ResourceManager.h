#pragma once

#include <map>
#include <string>

#include "Interfaces/IRenderer.h"
#include "Common/Common.h"
#include "SceneLoader.h"

#include "Common/DX11Common.h"

#ifdef DX11_ENABLED
#include "D3D11Renderer.h"
#endif


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	bool initialize(IRenderer* renderer);
	bool loadSceneResources(SceneData& scene);
	std::string loadDefaultMaterialResource();
	bool loadDefaultShaders();

	std::string loadSkyMeshResource();
	std::string loadSkyMaterialResource();

#ifdef DX11_ENABLED
	// TODO: I would like to abstract this resources, but I still need to study other render APIs
	std::map <std::string, DX11Mesh> meshResourceMap;
	std::map <std::string, DX11Material> materialResourceMap;

	std::vector<wrl::ComPtr<ID3D11VertexShader>> vertexShaders; // 0: VertexShader
	std::vector<wrl::ComPtr<ID3D11PixelShader>> pixelShaders; // 0: SimplePixelShader, 1: PixelShader, 2: SkyPixelShader

	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	wrl::ComPtr<ID3D11ShaderResourceView> pShadowMap; // For now we only have one shadow map
#endif

private :
	bool processNode(SceneData& scene, const SceneData::Node& node);

private:
	IRenderer* m_renderer;
};
