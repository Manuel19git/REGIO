#include "OpaquePass.h"

void OpaquePass::setup(IRenderer& renderer, HWND hwnd)
{
	m_renderer = &renderer;

	// Creting the swap chain and such should be done out of the game loop
#ifdef DX11_ENABLED
	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;

	d3d11renderer->ConfigureRenderPass(hwnd);
#endif
}
// I think that in execute I should also have the resourceManager, to access the resources referenced in RenderItems.
// This way, the renderer doesn't need to know of the existence of a resourceManager 
void OpaquePass::execute(IRenderer& renderer, ResourceManager& resourceManager, const std::vector<RenderItem>& items)
{
#ifdef DX11_ENABLED
	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;
	// 1. Begin frame (toda la limpia del frame anterior y eso? establecer target y eso va aqu�)
	d3d11renderer->BeginRenderPass();

	for (auto renderItem : items)
	{
		// Input layout and topology, this info should come from material but we can have default ones


		// Bind vertex buffers (I have this)

		// Bind constant buffers? (transform and such)

		// Where do emitter info and such go?

		// Viewport

		// Draw the object
	}


	// End frame (Presentamos? y limpiamos)
#endif
}

