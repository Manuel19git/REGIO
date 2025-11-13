#include "ResourceManager.h"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::initialize(IRenderer* renderer)
{
	m_renderer = renderer;
    return true;
}

bool ResourceManager::processNode(SceneData& scene, const SceneData::Node& node)
{
#ifdef DX11_ENABLED
	D3D11Renderer* d3d11Renderer = (D3D11Renderer*)m_renderer;
#endif

	if (node.children.size() > 0)
	{
		for (const auto childNode : node.children)
		{
			processNode(scene, childNode);
		}
	}
	else
	{
		if (node.type == NodeType::MESH)
		{
#ifdef DX11_ENABLED
			DX11Mesh meshResource = {};
			MeshNode mesh = scene.meshes[node.id];

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			// Vertex buffer
			bufferDesc.ByteWidth = sizeof(Vertex) * mesh.vertices.size();
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.StructureByteStride = sizeof(Vertex);

			d3d11Renderer->CreateBuffer(mesh.vertices.data(), &meshResource.vertexBuffer, bufferDesc);

			// Index buffer
			bufferDesc.ByteWidth = sizeof(UINT) * mesh.indices.size();
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.StructureByteStride = sizeof(UINT);

			d3d11Renderer->CreateBuffer(mesh.indices.data(), &meshResource.indexBuffer, bufferDesc);
			meshResource.indexCount = mesh.indices.size();

			meshResourceMap.insert({ node.name, meshResource });
#endif
		}
		else if (node.type == NodeType::EMITTER)
		{
			// What should I do with emitters? (Think about this)
		}
	}

	return true;
}

bool ResourceManager::loadDefaultShaders()
{
#ifdef DX11_ENABLED
	D3D11Renderer* d3d11Renderer = (D3D11Renderer*)m_renderer;

	// Load Vertex Shaders
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	d3d11Renderer->CreateVertexShader(searchFileInParentDirectories("\\shaders\\VertexShader.cso"), pVertexShader.GetAddressOf(), pInputLayout.GetAddressOf());
	vertexShaders.push_back(pVertexShader);

	// Load Pixel Shaders
	wrl::ComPtr<ID3D11PixelShader> pSimplePixelShader;
	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\SimplePixelShader.cso"), pSimplePixelShader.GetAddressOf());
	pixelShaders.push_back(pSimplePixelShader);

	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\PixelShader.cso"), pPixelShader.GetAddressOf());
	pixelShaders.push_back(pPixelShader);

	wrl::ComPtr<ID3D11PixelShader> pSkyPixelShader;
	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\SkyPixelShader.cso"), pSkyPixelShader.GetAddressOf());
	pixelShaders.push_back(pSkyPixelShader);
#endif

	return true;
}

bool ResourceManager::loadDefaultMaterial()
{
#ifdef DX11_ENABLED
	DX11Material defaultMaterial;

	// Default shaders (VertexShader.cso & PixelShader.cso)
	defaultMaterial.pVertexShader = vertexShaders[0];
	defaultMaterial.pPixelShader = pixelShaders[1];

    defaultMaterial.ambient    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMaterial.diffuse    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMaterial.specular   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	materialResourceMap.insert({ "DefaultMaterial", defaultMaterial});
#endif

	return true;

}

bool ResourceManager::processSceneResources(SceneData& scene)
{
	HRESULT hr;

	// Process scene
	processNode(scene, *scene.rootNode);

	// Process materials
#ifdef DX11_ENABLED
	D3D11Renderer* d3d11Renderer = (D3D11Renderer*)m_renderer;

	for (const auto& material : scene.materials)
	{
		DX11Material materialResource;

		// Default shaders (VertexShader.cso & PixelShader.cso)
		materialResource.pVertexShader = vertexShaders[0];
		materialResource.pPixelShader = pixelShaders[1];

		if (material.diffuseTexturePath != "")
			d3d11Renderer->CreateTexture(material.diffuseTexturePath, materialResource.pDiffuseTexture.GetAddressOf());

		if (material.specularTexturePath != "")
			d3d11Renderer->CreateTexture(material.specularTexturePath, materialResource.pSpecularTexture.GetAddressOf());

		if (material.normalTexturePath != "")
			d3d11Renderer->CreateTexture(material.normalTexturePath, materialResource.pNormalTexture.GetAddressOf());

		materialResource.ambient = XMFLOAT4(material.ambient.x, material.ambient.y, material.ambient.z, material.ambient.w);
		materialResource.diffuse = XMFLOAT4(material.diffuse.x, material.diffuse.y, material.diffuse.z, material.diffuse.w);
		materialResource.specular = XMFLOAT4(material.specular.x, material.specular.y, material.specular.z, material.specular.w);
		materialResource.reflect = XMFLOAT4(material.reflect.x, material.reflect.y, material.reflect.z, material.reflect.w);

		materialResourceMap.insert({ material.name, materialResource });
	}
#endif

	return true;
}
