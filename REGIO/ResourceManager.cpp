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

std::string ResourceManager::loadDefaultMaterialResource()
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

	return "DefaultMaterial";

}

std::string ResourceManager::loadSkyMeshResource()
{
	std::string skyResourceName = "Im-Singing_In.the+rain";

#ifdef DX11_ENABLED
	D3D11Renderer* d3d11Renderer = (D3D11Renderer*)m_renderer;

    float scale = 300.0f;
	Vertex vertices[] = {
		// Back Face
		{ {-1.0f * scale, -1.0f * scale, -1.0f * scale}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
		{ {1.0f * scale, -1.0f * scale, -1.0f * scale}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
		{ {1.0f * scale,  1.0f * scale, -1.0f * scale}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
		{ {-1.0f * scale,  1.0f * scale, -1.0f * scale}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },

		// Front Face
		{ {-1.0f * scale, -1.0f * scale,  1.0f * scale}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },
		{ {1.0f * scale, -1.0f * scale,  1.0f * scale}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
		{ {1.0f * scale,  1.0f * scale,  1.0f * scale}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
		{ {-1.0f * scale,  1.0f * scale,  1.0f * scale}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },

		// Left Face
		{ {-1.0f * scale, -1.0f * scale,  1.0f * scale}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
		{ {-1.0f * scale, -1.0f * scale, -1.0f * scale}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
		{ {-1.0f * scale,  1.0f * scale, -1.0f * scale}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
		{ {-1.0f * scale,  1.0f * scale,  1.0f * scale}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },

		// Right Face
		{ {1.0f * scale, -1.0f * scale,  1.0f * scale}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
		{ {1.0f * scale, -1.0f * scale, -1.0f * scale}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
		{ {1.0f * scale,  1.0f * scale, -1.0f * scale}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
		{ {1.0f * scale,  1.0f * scale,  1.0f * scale}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },

		// Top Face
		{ {-1.0f * scale,  1.0f * scale, -1.0f * scale}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
		{ {1.0f * scale,  1.0f * scale, -1.0f * scale}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
		{ {1.0f * scale,  1.0f * scale,  1.0f * scale}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
		{ {-1.0f * scale,  1.0f * scale,  1.0f * scale}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },


		// Bottom Face
		{ {-1.0f * scale, -1.0f * scale, -1.0f * scale}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
		{ {1.0f * scale, -1.0f * scale, -1.0f * scale}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
		{ {1.0f * scale, -1.0f * scale,  1.0f * scale}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
		{ {-1.0f * scale, -1.0f * scale,  1.0f * scale}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
	};


	unsigned int indices[] = {
		// Back Face
		0, 1, 2,
		0, 2, 3,

		// Front Face
		4, 6, 5,
		4, 7, 6,

		// Left Face
		8, 9, 10,
		8, 10, 11,

		// Right Face
		12, 14, 13,
		12, 15, 14,

		// Top Face
		16, 17, 18,
		16, 18, 19,

		// Bottom Face
		20, 22, 21,
		20, 23, 22
	};

	DX11Mesh skyMeshResource = {};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	// Vertex buffer
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(Vertex);

	d3d11Renderer->CreateBuffer(vertices, &skyMeshResource.vertexBuffer, bufferDesc);

	// Index buffer
	bufferDesc.ByteWidth = sizeof(indices);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(UINT);

	d3d11Renderer->CreateBuffer(indices, &skyMeshResource.indexBuffer, bufferDesc);
	skyMeshResource.indexCount = sizeof(indices)/sizeof(indices[0]);

	meshResourceMap.insert({ skyResourceName, skyMeshResource});
#endif

	return skyResourceName;
}

std::string ResourceManager::loadSkyMaterialResource()
{
	std::string skyResourceName = "Im-Singing_In.the+rain";

#ifdef DX11_ENABLED
	D3D11Renderer* d3d11Renderer = (D3D11Renderer*)m_renderer;

	DX11Material skyMaterialResource;
	skyMaterialResource.pVertexShader = vertexShaders[0];
	skyMaterialResource.pPixelShader = pixelShaders[2];

	d3d11Renderer->CreateSamplerState(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_COMPARISON_NEVER,
		skyMaterialResource.pSamplerState.GetAddressOf());

	std::string skyMapTexturePath = searchFileInParentDirectories("\\output\\NIER\\Props\\textures\\otro_cielo.dds");
	d3d11Renderer->CreateDDSTexture(skyMapTexturePath, skyMaterialResource.pDiffuseTexture.GetAddressOf());

	materialResourceMap.insert({ skyResourceName, skyMaterialResource});
#endif

	return skyResourceName;
}

bool ResourceManager::loadSceneResources(SceneData& scene)
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

		d3d11Renderer->CreateSamplerState(
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_COMPARISON_NEVER, 
			materialResource.pSamplerState.GetAddressOf());

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
