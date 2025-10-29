#pragma once

#include <map>
#include <string>

#include "Interfaces/IRenderer.h"
#include "Common/Common.h"
#include "SceneLoader.h"

#include "Common/DX11Common.h"

#ifdef DX11_ENABLED
#include "D3D11Renderer.h"
#endif


struct MeshGPUResource
{
	IGPUBuffer* vertexBuffer;
	IGPUBuffer* indexBuffer;
	int indexCount;
};

class ResourceManager
{
public:
	//TODO
	bool initialize(IRenderer* renderer);
	bool processSceneResources(SceneData& scene);
	bool processNode(SceneData& scene, const SceneData::Node& node);

	uint32_t loadMeshToGPU(std::string meshPath);

private :
#ifdef DX11_ENABLED
	std::map <std::string, DX11Mesh> meshResourceMap;
#endif
	//std::map <uint32_t, MaterialGPUResource> materialResourceMap;

	IRenderer* m_renderer;
};
