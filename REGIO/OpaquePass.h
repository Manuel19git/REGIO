#include "Common/DX11Common.h"
#include "D3D11Renderer.h"

#include "Interfaces/IRenderPass.h"
#include "Interfaces/IRenderer.h"

#include "ResourceManager.h"

class OpaquePass : public IRenderPass
{
	void setup(IRenderer& renderer, HWND hwnd);
	void execute(IRenderer& renderer, ResourceManager& resourceManager, const std::vector<RenderItem>& items);

public:
	IRenderer* m_renderer;
};