#include "SceneLoader.h"
#include "Common/Common.h"

#ifdef _DEBUG
#include <iostream>
#endif

SceneLoader::SceneLoader()
{
	pScene = std::make_unique<SceneData>();

	pScene->rootNode = new SceneData::Node();
}

void SceneLoader::loadScene(std::string scenePath)
{
	commonSearchDirectories.push_back(getDirectory(scenePath));
	std::string ext = getExtension(scenePath);
	
	Assimp::Importer* importer = new Assimp::Importer();
	const aiScene* aiScene = importer->ReadFile(scenePath,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (!aiScene)
	{
		MessageBoxA(nullptr, importer->GetErrorString(), "Assimp Importer error", 0);
	}

	// Materials are not represented as "nodes" on the scene, so we load them separately
	loadMaterials(aiScene);

	//Process aiScene to fill SceneData with the data we need
	pScene->rootNode->type = NodeType::EMPTY;
	pScene->rootNode->name = aiScene->mRootNode->mName.C_Str();
	pScene->rootNode->transform = aiScene->mRootNode->mTransformation;
	if (ext == ".fbx" || ext == ".gltf" || ext == ".glb")
		pScene->rootNode->transform.Transpose();

	for (int i = 0; i < aiScene->mRootNode->mNumChildren; ++i)
	{
		processNode(*pScene->rootNode, aiScene, aiScene->mRootNode->mChildren[i], ext);
	}
	delete importer;

#ifdef _DEBUG
	logDebugInfo();
#endif
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

void SceneLoader::loadMaterials(const aiScene* scene)
{
	for (int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* aiMaterial = scene->mMaterials[i];

		MaterialNode material;
		material.name = aiMaterial->GetName().C_Str();

		aiColor3D ambient;
		aiColor3D diffuse;
		aiColor3D specular;
		aiColor3D reflect;

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
		if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, reflect))
		{
			material.reflect = Vector(reflect.r, reflect.g, reflect.b, 1.0f);
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

		pScene->materials.push_back(material);
	}
}

void SceneLoader::processNode(SceneData::Node& parentNode,const aiScene* aiScene, const aiNode* aiNode, std::string ext)
{
	int numChildren = aiNode->mNumChildren;

	// fbx,gltf/glb are column major, thus we transpose them
	Matrix4x4 auxNodeTransform = aiNode->mTransformation;
	if (ext == ".fbx" || ext == ".gltf" || ext == ".glb")
		auxNodeTransform.Transpose();
	
	if (numChildren > 0) // Still process
	{
		SceneData::Node node;
		node.type = NodeType::EMPTY;
		node.name = aiNode->mName.C_Str();
		node.transform = auxNodeTransform * parentNode.transform;
		parentNode.children.push_back(node);

		for (int i = 0; i < aiNode->mNumChildren; ++i)
		{
			processNode(parentNode.children.back(), aiScene, aiNode->mChildren[i], ext);
		}
	}
	else // Leaf node
	{
		SceneData::Node leafNode;
		leafNode.name = aiNode->mName.C_Str();
  		leafNode.transform =  auxNodeTransform * parentNode.transform;
		if (aiNode->mNumMeshes > 0)
		{
			leafNode.type = NodeType::MESH;
			//TODO:  For now only support one mesh per node (but the idea is to support more than one in the future)
			aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[0]];
			leafNode.materialName = aiScene->mMaterials[aiMesh->mMaterialIndex]->GetName().C_Str();
			
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
		else if (aiScene->mNumLights > 0)
		{
			leafNode.type = NodeType::EMITTER;
			
			// I should get more than one light if there are any
			aiLight* aiEmitter = aiScene->mLights[0];

			// For now emitters are only pointLights
			EmitterNode emitter(
				Vector(aiEmitter->mPosition.x, aiEmitter->mPosition.y, aiEmitter->mPosition.z, 1.0f),
				0.0f
			);

			pScene->emitters.push_back(emitter);

			leafNode.id = pScene->emitters.size() - 1;
		}
		else if (aiScene->mNumCameras > 0)
		{
			leafNode.type = NodeType::CAMERA;
			
			// I should get more than one camera if there are any
			aiCamera* aiCamera = aiScene->mCameras[0];

			// No need to compose camera with Node (transformation is already in aiCamera)
			Vector startPos = Vector(aiCamera->mPosition.x,aiCamera->mPosition.y, aiCamera->mPosition.z, 1.0f);
			Vector startForward = Vector(aiCamera->mLookAt.x,aiCamera->mLookAt.y, aiCamera->mLookAt.z, 1.0f);
			
			Camera camera( startPos, startForward );

			pScene->cameras.push_back(camera);

			leafNode.id = pScene->cameras.size() - 1;
		}
		parentNode.children.push_back(leafNode);
	}

}

#ifdef _DEBUG
void SceneLoader::logDebugInfo()
{
	std::cout << "Meshes:" << pScene->meshes.size() << std::endl;
	std::cout << "Cameras:" << pScene->cameras.size() << std::endl;
	std::cout << "Materials:" << pScene->materials.size() << std::endl;
	std::cout << "Emitters:" << pScene->emitters.size() << std::endl;
}
#endif
