#pragma once

#include<map>
#include<string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXMath.h>

#include "Common/Common.h"
#include "Camera.h"
#include "MyException.h"

#include <memory>

enum NodeType
{
	EMPTY,
	MESH,
	CAMERA,
	EMITTER,
};
struct SceneData
{
	struct Node
	{
		std::string name;
		Matrix4x4 transform;
		NodeType type;
		//uint32_t id; // Id pointing to the scene maps depending on the NodeType
		std::vector<Node> children;
		Node& operator = (const Node& other)
		{
			name = other.name;
			transform = other.transform;
			type = other.type;
			//id = other.id;
			children = other.children;
			return *this;
		}
	};

	Node* rootNode;

	//std::map<uint32_t, Mesh> meshesMap;
	//std::map<uint32_t, Camera> camerasMap;
	//std::map<uint32_t, Emitter> emittersMap;
	std::vector<MeshNode> meshes;
	std::vector<Camera> cameras;
	std::vector<EmitterNode> emitters;
};

class SceneLoader
{
private:
	void processNode(SceneData::Node& parentNode, const aiScene* scene, const aiNode* node);

public:
	SceneLoader();

	void loadScene(std::string scenePath);

	std::unique_ptr<SceneData> pScene;
	
};
