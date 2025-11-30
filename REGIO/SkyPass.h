#pragma once

#include "Interfaces/IRenderPass.h"
#include "Interfaces/IRenderer.h"

class SkyPass
{
public:
	SkyPass();
	~SkyPass();

	void setup(IRenderer& renderer, ResourceManager& resourceManager, HWND hwnd, Camera* camera);
	void execute(SceneData& scene, const RenderItem& skyItem);

private:
	IRenderer* m_renderer;
	ResourceManager* m_resourceManager;

	Camera* m_mainCamera;

};
