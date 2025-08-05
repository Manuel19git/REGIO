#pragma once

#include <map>
#include <string>

#include "Interfaces/IRenderer.h"
#include "Common/Common.h"


struct MeshGPUResource
{
	IGPUBuffer* vertexBuffer;
	IGPUBuffer* indexBuffer;
	int indexCount;
};


class ResourceManager
{
	//TODO
	uint32_t loadMeshToGPU(std::string meshPath);

private :
	std::map <uint32_t, MeshGPUResource> meshResourceMap;
	std::map <uint32_t, MaterialGPUResource> materialResourceMap;

	IRenderer* renderer;
};
