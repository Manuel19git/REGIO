#include "OpaquePass.h"

OpaquePass::OpaquePass()
{
}

OpaquePass::~OpaquePass()
{
}

void OpaquePass::setup(IRenderer& renderer, ResourceManager& resourceManager, HWND hwnd, Camera* camera)
{
	mainCamera = camera;
	m_renderer = &renderer;
	m_resourceManager = &resourceManager;

#ifdef DX11_ENABLED
	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;
	d3d11renderer->ConfigureRenderPass(hwnd);
#endif
}

// I think that in execute I should also have the resourceManager, to access the resources referenced in RenderItems.
// This way, the renderer doesn't need to know of the existence of a resourceManager 
void OpaquePass::execute(SceneData& scene, const std::vector<RenderItem>& items)
{
	if (!scene.cameras.empty())
		mainCamera = &scene.cameras[0];

#ifdef DX11_ENABLED

	D3D11Renderer* d3d11renderer = (D3D11Renderer*)m_renderer;
	// 1. Begin frame (toda la limpia del frame anterior y eso? establecer target y eso va aqu�)
	d3d11renderer->BeginRenderPass();

	for (auto renderItem : items)
	{
		// Should I do a single drawItem call or individual calls?

		// Input layout and topology, this info should come from material but we can have default ones
		d3d11renderer->SetVertexLayoutAndTopology(m_resourceManager->pInputLayout.Get(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind vertex buffers (I have this)
		DX11Mesh mesh = m_resourceManager->meshResourceMap[renderItem.meshHandle];
		d3d11renderer->SetMeshBuffers(mesh.vertexBuffer.Get(), mesh.indexBuffer.Get());

		// Bind constant buffers? (transform and such)
		DX11Material material = m_resourceManager->materialResourceMap[renderItem.materialHandle];
		cbPerObject cbObject;
		cbObject.gTransform = renderItem.worldTransform.ToXMMATRIX() * mainCamera->getTransform();
		//cbObject.gTransform = mainCamera->getTransform();
		cbObject.hasTexture = (material.pDiffuseTexture || material.pSpecularTexture || material.pNormalTexture) ? true : false;

		cbObject.gMaterial.Ambient = material.ambient;
		cbObject.gMaterial.Specular = material.specular;
		cbObject.gMaterial.Diffuse = material.diffuse;
		cbObject.gMaterial.Reflect = material.reflect;

		d3d11renderer->SetObjectConstantBufferVS(&cbObject, sizeof(cbObject), 0);
		d3d11renderer->SetObjectConstantBufferPS(&cbObject, sizeof(cbObject), 0);

		// Bind per frame buffer
		cbPerFrame cbFrame;
		cbFrame.gEyePosW = mainCamera->getPosition();

		// I don't whink this emitter info should be uploaded every frame
		float down = 10.0f;
		cbFrame.gDirLight.Ambient = XMFLOAT4(0.6156f / down, 0.3568f / down, 0.6392f / down, 1.0f);
		cbFrame.gDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		cbFrame.gDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		cbFrame.gDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, -0.57735f);
		cbFrame.gDirLight.Intensity = 1.0f;
		cbFrame.sunActive = (sunActive) ? 1 : 0; // Why not sunActive directly

		d3d11renderer->SetFrameConstantBufferPS(&cbFrame, sizeof(cbPerFrame), 1);


		// Bind shader resources such as textures (shadow map bind here if any)
		if (material.pDiffuseTexture)
			d3d11renderer->SetShaderResourcePS(material.pDiffuseTexture.Get(), 1);

		// Set vertex/pixel shaders
		d3d11renderer->SetShaders(material.pVertexShader.Get(), material.pPixelShader.Get());

		// Set sampler states with shadow maps

		// Draw the object
		d3d11renderer->DrawItem(mesh.indexCount);

	}
	// End frame (Presentamos? y limpiamos)
	d3d11renderer->EndRenderPass();
#endif
}

