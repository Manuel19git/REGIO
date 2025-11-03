#include "SceneLoader.h"

SceneLoader::SceneLoader()
{
	pScene = std::make_unique<SceneData>();
}

void SceneLoader::loadScene(std::string scenePath)
{
	Assimp::Importer* importer = new Assimp::Importer();
	const aiScene* aiScene = importer->ReadFile(scenePath,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (!aiScene)
	{
		MessageBoxA(nullptr, importer->GetErrorString(), "Assimp Importer error", 0);
	}

	//Process aiScene to fill SceneData with the data we need
	SceneData::Node rootNode;
	rootNode.type = NodeType::EMPTY;
	rootNode.name = aiScene->mRootNode->mName.C_Str();
	rootNode.transform = aiScene->mRootNode->mTransformation;
	
	for (int i = 0; i < aiScene->mRootNode->mNumChildren; ++i)
	{
		processNode(rootNode, aiScene, aiScene->mRootNode->mChildren[i]);
	}
}

std::pair<NodeType,int> getNodeTypeAndID(const aiScene* aiScene, std::string nodeName)
{
	// Loop meshes
	for (int meshId = 0; meshId < aiScene->mNumMeshes; ++meshId)
	{
		if (aiScene->mMeshes[meshId]->mName.C_Str() == nodeName)
		{
			return std::pair(NodeType::MESH, meshId);
		}
	}

	// Loop emitters
	for (int emitterId = 0; emitterId < aiScene->mNumLights; ++emitterId)
	{
		if (aiScene->mLights[emitterId]->mName.C_Str() == nodeName)
		{
			return std::pair(NodeType::EMITTER, emitterId);
		}
	}

	// Loop cameras
	for (int cameraId = 0; cameraId < aiScene->mNumCameras; ++cameraId)
	{
		if (aiScene->mCameras[cameraId]->mName.C_Str() == nodeName)
		{
			return std::pair(NodeType::CAMERA, cameraId);
		}
	}
	for (int materialId = 0; materialId < aiScene->mNumMaterials; ++materialId)
	{
		if (aiScene->mMaterials[materialId]->GetName().C_Str() == nodeName)
		{
			return std::pair(NodeType::MATERIAL, materialId);
		}
	}
	return std::pair(NodeType::EMPTY, -1);
}

//TODO: Nodes with children will be "empty" for now
void SceneLoader::processNode(SceneData::Node& parentNode,const aiScene* aiScene, const aiNode* aiNode)
{
	int numChildren = aiNode->mNumChildren;
	if (numChildren > 0) // Still process
	{
		SceneData::Node node;
		node.type = NodeType::EMPTY;
		node.name = aiNode->mName.C_Str();
		node.transform = aiNode->mTransformation;

		parentNode.children.push_back(node);

		for (int i = 0; i < aiNode->mNumChildren; ++i)
		{
			processNode(node, aiScene, aiNode->mChildren[i]);
		}
	}
	else // Leaf node
	{
		SceneData::Node leafNode;
		leafNode.name = aiNode->mName.C_Str();
		leafNode.transform = aiNode->mTransformation;

		std::pair<NodeType,int> nodeTypeId = getNodeTypeAndID(aiScene, aiNode->mName.C_Str());
		leafNode.type = nodeTypeId.first;
		int nodeId = nodeTypeId.second;
		//TODO:  For now only support one mesh per node (but the idea is to support more than one in the future)
		if (leafNode.type == NodeType::MESH)
		{
			//aiMesh* aiMesh = aiScene.mMeshes[aiNode->mMeshes[0]];
			aiMesh* aiMesh = aiScene->mMeshes[nodeId];

			MeshNode mesh;

			mesh.vertices.resize(aiMesh->mNumVertices);
			mesh.indices.resize(aiMesh->mNumFaces * aiMesh->mFaces->mNumIndices);

			for (int vertexId = 0; vertexId < aiMesh->mNumVertices; ++vertexId)
			{
				mesh.vertices[vertexId].pos.x = aiMesh->mVertices[vertexId].x;
				mesh.vertices[vertexId].pos.y = aiMesh->mVertices[vertexId].y;
				mesh.vertices[vertexId].pos.z = aiMesh->mVertices[vertexId].z;

				mesh.vertices[vertexId].normal.x = aiMesh->mNormals[vertexId].x;
				mesh.vertices[vertexId].normal.y = aiMesh->mNormals[vertexId].y;
				mesh.vertices[vertexId].normal.z = aiMesh->mNormals[vertexId].z;


				if (aiMesh->HasTextureCoords(0)) // We only check for one texture per mesh
				{
					mesh.vertices[vertexId].tex.u = aiMesh->mTextureCoords[0][vertexId].x;
					mesh.vertices[vertexId].tex.v = aiMesh->mTextureCoords[0][vertexId].y;
				}
			}

			for (int faceId = 0; faceId < aiMesh->mNumFaces; ++faceId)
			{
				for (int faceVertexId = 0; faceVertexId < aiMesh->mFaces->mNumIndices; ++faceVertexId)
				{
					mesh.indices[(faceId * aiMesh->mFaces->mNumIndices) + faceVertexId] = aiMesh->mFaces[faceId].mIndices[faceVertexId];
				}
			}
			pScene->meshes.push_back(mesh);

			leafNode.id = pScene->meshes.size() - 1;

		}
		else if (leafNode.type == NodeType::EMITTER)
		{
			aiLight* aiEmitter = aiScene->mLights[nodeId];

			// For now emitters are only pointLights
			EmitterNode emitter(
				Vector(aiEmitter->mPosition.x, aiEmitter->mPosition.y, aiEmitter->mPosition.z, 1.0f),
				0.0f
			);

			pScene->emitters.push_back(emitter);

			leafNode.id = pScene->emitters.size() - 1;
		}
		else if (leafNode.type == NodeType::CAMERA)
		{
			aiCamera* aiCamera = aiScene->mCameras[nodeId];

			// No need to compose camera with Node (transformation is already in aiCamera)
			Camera camera(
				Vector(aiCamera->mPosition.x,aiCamera->mPosition.y, aiCamera->mPosition.z, 1.0f), 
				Vector(aiCamera->mLookAt.x,aiCamera->mLookAt.y, aiCamera->mLookAt.z, 1.0f)
			);

			pScene->cameras.push_back(camera);

			leafNode.id = pScene->cameras.size() - 1;
		}
		else if (leafNode.type == NodeType::MATERIAL)
		{
			aiMaterial* aiMaterial = aiScene->mMaterials[nodeId];

			MaterialNode material;
			aiColor3D ambient;
			aiColor3D diffuse;
			aiColor3D specular;
			
			if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient))
			{
				material.ambient = Vector(ambient.r, ambient.g, ambient.b, 1.0f);
			}
			if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse))
			{
				material.diffuse = Vector(diffuse.r, diffuse.g, diffuse.b, 1.0f);
			}
			if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular))
			{
				material.specular = Vector(specular.r, specular.g, specular.b, 1.0f);
			}

			aiString normalTexture;
			aiString diffuseTexture;
			aiString specularTexture;
			if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexture))
			{
				material.diffuseTexturePath = diffuseTexture.C_Str();
			}
			if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specularTexture))
			{
				material.specularTexturePath = specularTexture.C_Str();
			}
			if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexture))
			{
				material.normalTexturePath = normalTexture.C_Str();
			}

			// Do materials come with shaderPaths?

			pScene->materials.push_back(material);
			leafNode.id = pScene->materials.size() - 1;
		}

		parentNode.children.push_back(leafNode);
	}

}
