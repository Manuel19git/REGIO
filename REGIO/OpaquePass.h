#pragma once

#include "Common/DX11Common.h"
#include "D3D11Renderer.h"

//#include "Interfaces/IRenderPass.h"
#include "Interfaces/IRenderer.h"
#include "ResourceManager.h"

class OpaquePass /*: public IRenderPass*/
{
public:
	OpaquePass();
	~OpaquePass();

	void setup(IRenderer& renderer, ResourceManager& resourceManager, HWND hwnd, Camera* camera, Camera* sunCamera);
	void execute(SceneData& scene, const std::vector<RenderItem>& items);


	bool sunActive = false;


private:
	IRenderer* m_renderer;
	ResourceManager* m_resourceManager;

	Camera* m_mainCamera;
	Camera* m_sunCamera;
};
