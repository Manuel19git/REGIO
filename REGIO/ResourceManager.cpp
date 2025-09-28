#include "ResourceManager.h"


#ifdef DX11
#include "D3D11Renderer.h"
#endif

bool ResourceManager::initialize(IRenderer* renderer)
{
#ifdef DX11
	renderer = new D3D11Renderer();
	m_renderer = renderer;
#endif
    return true;
}

bool ResourceManager::processNode(SceneData& scene, const SceneData::Node& node)
{
#ifdef DX11
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
#ifdef DX11
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

	}

	return true;
}

bool ResourceManager::processSceneResources(SceneData& scene)
{
	HRESULT hr;
	DxgiInfoManager infoManager;

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
