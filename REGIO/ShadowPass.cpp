#include "ShadowPass.h"

ShadowPass::ShadowPass()
{
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::setup(IRenderer& renderer, std::unique_ptr<ResourceManager>& resourceManager, HWND hwnd, Camera* camera)
{
	m_mainCamera = camera;
	m_renderer = &renderer;
	m_resourceManager = resourceManager.get();

#ifdef DX11_ENABLED
	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;
	d3d11renderer->ConfigureShadowPass(hwnd, resourceManager->pShadowMap.GetAddressOf());
#endif
}

// scene is only needed to traverse camera from scene. TODO: improve if more than one camera present
void ShadowPass::execute(SceneData& scene, const std::vector<RenderItem>& items)
{
	if (!scene.cameras.empty())
		m_mainCamera = &scene.cameras[0];

#ifdef DX11_ENABLED

	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;

	// we bind new targets
	d3d11renderer->SetNullTargetAndRS();

	XMMATRIX transform = m_mainCamera->getTransform(true);

	for (auto renderItem : items)
	{
		// Input layout and topology, this info should come from material but we can have default ones
		d3d11renderer->SetVertexLayoutAndTopology(m_resourceManager->pInputLayout.Get(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind vertex buffers
		DX11Mesh mesh = m_resourceManager->meshResourceMap[renderItem.meshHandle];
		d3d11renderer->SetMeshBuffers(mesh.vertexBuffer.Get(), mesh.indexBuffer.Get());

		// Bind constant buffers
		DX11Material material = m_resourceManager->materialResourceMap[renderItem.materialHandle];
		cbPerObject cbObject;
		cbObject.gTransform = renderItem.worldTransform.ToXMMATRIX() * transform;
		cbObject.gTransformSun = renderItem.worldTransform.ToXMMATRIX() * transform;
		d3d11renderer->SetObjectConstantBufferVS(&cbObject, sizeof(cbObject), 0);

		// We don't need pixel shader with shadow pass
		d3d11renderer->SetShaders(material.pVertexShader.Get(), NULL);

		// Draw the object
		d3d11renderer->DrawItem(mesh.indexCount);
	}
	// Restore target and 
	d3d11renderer->RestoreTargetAndRS();
#endif
}

