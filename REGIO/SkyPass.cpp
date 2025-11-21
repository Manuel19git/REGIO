#include "SkyPass.h"

SkyPass::SkyPass()
{
}

SkyPass::~SkyPass()
{
}

void SkyPass::setup(IRenderer& renderer, ResourceManager& resourceManager, HWND hwnd, Camera* camera)
{
	mainCamera = camera;
	m_renderer = &renderer;
	m_resourceManager = &resourceManager;

#ifdef DX11_ENABLED
	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;
	d3d11renderer->ConfigureRenderPass(hwnd);
#endif
}

// scene is only needed to traverse camera from scene. TODO: improve if more than one camera present
void SkyPass::execute(SceneData& scene, const RenderItem& skyItem)
{
	if (!scene.cameras.empty())
		mainCamera = &scene.cameras[0];

#ifdef DX11_ENABLED

	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;

	{
		// Input layout and topology, this info should come from material but we can have default ones
		d3d11renderer->SetVertexLayoutAndTopology(m_resourceManager->pInputLayout.Get(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind vertex buffers (I have this)
		DX11Mesh mesh = m_resourceManager->meshResourceMap[skyItem.meshHandle];
		d3d11renderer->SetMeshBuffers(mesh.vertexBuffer.Get(), mesh.indexBuffer.Get());

		// Bind constant buffers? (transform and such)
		cbPerObject cbObject;
		DirectX::XMMATRIX viewMatrix = mainCamera->getViewMatrix();
		viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // Remove translation part of matrix
		DirectX::XMMATRIX projectionMatrix = mainCamera->getProjectionMatrix();
		cbObject.gTransform = DirectX::XMMatrixTranspose(viewMatrix * projectionMatrix);

		d3d11renderer->SetObjectConstantBufferVS(&cbObject, sizeof(cbObject), 0);

		// Bind shader resources such as textures (shadow map bind here if any)
		DX11Material material = m_resourceManager->materialResourceMap[skyItem.materialHandle];

		if (material.pDiffuseTexture)
			d3d11renderer->SetTextureAndSamplerResourcePS(material.pDiffuseTexture.Get(), 0, material.pSamplerState.Get());

		// Set vertex/pixel shaders
		d3d11renderer->SetShaders(material.pVertexShader.Get(), material.pPixelShader.Get());

		// Draw the object
		d3d11renderer->DrawItem(mesh.indexCount);

	}
#endif
}


