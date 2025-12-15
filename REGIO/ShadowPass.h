#pragma once

//#include "Interfaces/IRenderPass.h"
#include "ResourceManager.h"
#include "Interfaces/IRenderer.h"

class ShadowPass
{
public:
	ShadowPass();
	~ShadowPass();

	void setup(IRenderer& renderer, std::unique_ptr<ResourceManager>& resourceManager, HWND hwnd, Camera* camera);
	void execute(SceneData& scene, const std::vector<RenderItem>& items);

private:
	IRenderer* m_renderer;
	ResourceManager* m_resourceManager;


	Camera* m_mainCamera;

};

