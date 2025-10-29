#include "Common/DX11Common.h"
#include "Interfaces/IRenderPass.h"
#include "Interfaces/IRenderer.h"
#include "D3D11Renderer.h"


class OpaquePass : public IRenderPass
{
	void setup(IRenderer& renderer, HWND hwnd);
	void execute(IRenderer& renderer, const std::vector<RenderItem>& items);

public:
	IRenderer* m_renderer;
};