#include "Common/DX11Common.h"
#include "D3D11Renderer.h"

#include "Interfaces/IRenderPass.h"
#include "Interfaces/IRenderer.h"

class OpaquePass /*: public IRenderPass*/
{
public:
	OpaquePass();
	~OpaquePass();

	void setup(IRenderer& renderer, ResourceManager& resourceManager, HWND hwnd, Camera* camera);
	void execute(SceneData& scene, const std::vector<RenderItem>& items);

	IRenderer* m_renderer;
	ResourceManager* m_resourceManager;

	bool sunActive = false;

private:
	Camera* mainCamera;
};