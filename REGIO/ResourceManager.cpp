#include "ResourceManager.h"


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

			// Constant buffer?
			bufferDesc.ByteWidth = sizeof(Matrix4x4);
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // CPU Write and GPU Read (Used for buffers updated at least once per frame)
			bufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Allow CPU to write to the buffer

			d3d11Renderer->CreateBuffer(nullptr, &meshResource.transformationBuffer, bufferDesc);

			meshResourceMap.insert({ node.name, meshResource });
#endif
		}
		else if (node.type == NodeType::EMITTER)
		{
			// What should I do with emitters? (Think about this)
		}
		else if (node.type == NodeType::MATERIAL)
		{
#ifdef DX11_ENABLED
			DX11Material materialResource;
			MaterialNode material = scene.materials[node.id];

			// Default shaders (VertexShader.cso & PixelShader.cso)
			materialResource.pVertexShader = vertexShaders[0];
			materialResource.pPixelShader = pixelShaders[1];

			d3d11Renderer->CreateTexture(material.diffuseTexturePath, materialResource.pDiffuseTexture.Get());
			d3d11Renderer->CreateTexture(material.specularTexturePath, materialResource.pSpecularTexture.Get());
			d3d11Renderer->CreateTexture(material.normalTexturePath, materialResource.pNormalTexture.Get());

			materialResource.ambient = XMFLOAT4(material.ambient.x, material.ambient.y, material.ambient.z, material.ambient.w);
			materialResource.diffuse = XMFLOAT4(material.diffuse.x, material.diffuse.y, material.diffuse.z, material.diffuse.w);
			materialResource.specular = XMFLOAT4(material.specular.x, material.specular.y, material.specular.z, material.specular.w);


			materialResourceMap.insert({ node.name, materialResource });
#endif
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
	d3d11Renderer->CreateVertexShader(searchFileInParentDirectories("\\shaders\\VertexShader.cso"), pVertexShader.Get(), pInputLayout.Get());
	vertexShaders.push_back(pVertexShader);

	// Load Pixel Shaders
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\SimplePixelShader.cso"), pPixelShader.Get());
	pixelShaders.push_back(pPixelShader);

	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\PixelShader.cso"), pPixelShader.Get());
	pixelShaders.push_back(pPixelShader);

	d3d11Renderer->CreatePixelShader(searchFileInParentDirectories("\\shaders\\SkyPixelShader.cso"), pPixelShader.Get());
	pixelShaders.push_back(pPixelShader);
#endif

	return true;
}

bool ResourceManager::processSceneResources(SceneData& scene)
{
	HRESULT hr;

	// Process scene
	processNode(scene, *scene.rootNode);

	// Process materials
	//for (const auto& material : scene.materials)
	{
		// Load material resources here
		// ...
	}

	return true;
}
