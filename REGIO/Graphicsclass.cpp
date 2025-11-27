////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Graphicsclass.h"


GraphicsClass::GraphicsClass()
{

}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}

// Everything besides scene is output parameter. For this to work the scene meshes should in world space
void computeBoundingBox(const SceneData& scene, float& left, float& right, float& top, float& bottom, float &nearPlane, float &farPlane)
{
	PROFILE_SCOPE();

	left = FLT_MAX;
	right = -FLT_MAX;
	top = -FLT_MAX;
	bottom = FLT_MAX;
	nearPlane = FLT_MAX;
	farPlane = -FLT_MAX;

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		MeshNode mesh = scene.meshes[i];
		for (int j = 0; j < mesh.vertices.size(); j++)
		{
			Vertex vertex = mesh.vertices[j];
			if (vertex.pos.x < left)
			{
				left = vertex.pos.x;
			}
			if (vertex.pos.x > right)
			{
				right = vertex.pos.x;
			}
			if (vertex.pos.y > top)
			{
				top = vertex.pos.y;
			}
			if (vertex.pos.y < bottom)
			{
				bottom = vertex.pos.y;
			}
			if (vertex.pos.z < nearPlane)
			{
				nearPlane = vertex.pos.z;
			}
			if (vertex.pos.z > farPlane)
			{
				farPlane = vertex.pos.z;
			}
		}
	}

	// We want the bounding box to be a square box. In case it is a rectangle, it is corrected here
	// XY Plane
	float vLength = fabs(top - bottom);
	float hLength = fabs(right - left);
	if ( vLength < hLength)
	{
		float offset = (hLength - vLength) / 2;
		top += offset;
		bottom -= offset;
	}
	else
	{
		float offset = (vLength - hLength) / 2;
		right += offset;
		left -= offset;
	}
	// XZ Plane
	hLength = fabs(right - left);
	float zLength = fabs(farPlane - nearPlane);
	if (hLength < zLength)
	{
		float offset = (zLength - hLength) / 2;
		right += offset;
		left -= offset;
	}
	else
	{
		float offset = (hLength - zLength) / 2;
		farPlane += offset;
		nearPlane -= offset;
	}

}

BoundingBox ComputeSunFrustum(Camera* mainCamera, Camera* sunCamera)
{
    BoundingBox frustum = BoundingBox();

    // We want to get the shadow map with a good quality without it changing the main camera far which is very far
    // so we set far of the mainCamera to the 
    float farDefault = mainCamera->getFar();
    mainCamera->setFar(std::abs(mainCamera->getSceneBBox().farPlane - mainCamera->getSceneBBox().nearPlane)); // WARNING: Shitty way of solving this, might break in the future

    // 1. Coger la matriz de transformacion de la camara principal
    DirectX::XMMATRIX view = mainCamera->getViewMatrix();
    DirectX::XMMATRIX proj = mainCamera->getProjectionMatrix();
    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj );
    DirectX::XMMATRIX sunViewProj = sunCamera->getViewMatrix();

    mainCamera->setFar(farDefault);

    // 2. Multiplicarla por la posicion del cubo can�nico
    DirectX::XMVECTOR ndcCorners[8] = {
        {-1,-1,0,1}, {+1,-1,0,1}, {-1,+1,0,1}, {+1,+1,0,1},
        {-1,-1,1,1}, {+1,-1,1,1}, {-1,+1,1,1}, {+1,+1,1,1}
	};
    DirectX::XMVECTOR sunViewCorners[8];
    for (int i = 0; i < 8; ++i)
    {
        DirectX::XMVECTOR worldPos = DirectX::XMVector4Transform(ndcCorners[i], invViewProj);
        DirectX::XMFLOAT4 worldPosFloat = { 0,0,0,0 };

        DirectX::XMStoreFloat4(&worldPosFloat, worldPos);

        // From homogeneous space to cartesian world
        worldPos /= worldPosFloat.w;

        // From cartesian world to sun view space
        sunViewCorners[i] = DirectX::XMVector4Transform(worldPos, sunViewProj);
    }


    for (DirectX::XMVECTOR corner : sunViewCorners)
    {
        DirectX::XMFLOAT4 cornerFloat = { 0,0,0,0 };
        DirectX::XMStoreFloat4(&cornerFloat, corner);
        if (cornerFloat.x < frustum.left)
        {
            frustum.left = cornerFloat.x;
        }
        if (cornerFloat.x > frustum.right)
        {
            frustum.right = cornerFloat.x;
        }
        if (cornerFloat.y > frustum.top)
        {
            frustum.top = cornerFloat.y;
        }
        if (cornerFloat.y < frustum.bottom)
        {
            frustum.bottom = cornerFloat.y;
        }
        if (cornerFloat.z < frustum.nearPlane)
        {
            frustum.nearPlane = cornerFloat.z;
        }
        if (cornerFloat.z > frustum.farPlane)
        {
            frustum.farPlane = cornerFloat.z;
        }
    }


    return frustum;
}


void buildSceneRenderItems(SceneData::Node& node, std::vector<RenderItem>& items)
{
	// Using preorder traverse method for no particular reason :)
	if (node.type == NodeType::MESH)
	{
		RenderItem item;
		item.meshHandle = node.name;
		item.materialHandle = node.materialName;
		item.worldTransform = node.transform;

		items.push_back(item);
	}
	//Emitters?

	if (node.children.size() > 0)
	{
		for (auto child : node.children)
		{
			buildSceneRenderItems(child, items);
		}
	}
}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, InputClass* m_Input, std::wstring wideScenePath)
{
	PROFILE_SCOPE();

	mouse = &m_Input->mouse;
	m_hwnd = hwnd;

	std::string scenePath = wideString2String(wideScenePath);
	if ( scenePath == "")
		scenePath = searchFileInParentDirectories("\\output\\Maxwell_cat\\source\\maxwell_scene.obj");

	//------------------------------------------------------------- NEW -------------------------------------------------------------
#ifdef DX11_ENABLED
	m_renderer = std::make_shared<D3D11Renderer>(hwnd);
#endif

	// 1. Load scene and generate Scene Data
	m_sceneLoader = std::make_unique<SceneLoader>();
	m_sceneLoader->loadScene(scenePath);
	
	// 2. Process scene and create RenderItems with its resources.
	m_resourceManager = std::make_unique<ResourceManager>();
	m_resourceManager->initialize(m_renderer.get());
	m_resourceManager->loadDefaultShaders(); // Now resource manager has its resources (meshes only for now)
	m_resourceManager->loadDefaultMaterialResource(); 

	m_resourceManager->loadSceneResources(*m_sceneLoader->pScene.get()); // Now resource manager has its resources (meshes only for now)

	// 3. Process Scene Data with resources to generate batch of Render Items sharing same shader
	buildSceneRenderItems(*m_sceneLoader->pScene->rootNode, renderItems);

	// Build sky render item
	skyItem.meshHandle = m_resourceManager->loadSkyMeshResource();
	skyItem.materialHandle = m_resourceManager->loadSkyMaterialResource();

	//------------------------------------------------------------- OLD -------------------------------------------------------------

	// Read mesh
	//importer = new Assimp::Importer();
	////mScene = importer->ReadFile("..\\output\\NIER\\Props\\turnstile_wall.usdc", //USD is not fully supported by assimp yet
	////	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	//mScene = importer->ReadFile(scenePath,
	//	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	////mScene = importer->ReadFile("..\\output\\NIER\\nier_park.obj",
	////	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	////mScene = importer->ReadFile("..\\output\\NIER\\nier_park.glb",
	////	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	//if (!mScene)
	//{
	//	MessageBoxA(nullptr, importer->GetErrorString(), "Assimp Importer error", 0);
	//}

	// Compute bounding box
	computeBoundingBox(*m_sceneLoader->pScene, scenebbox.left, scenebbox.right, scenebbox.top, scenebbox.bottom, scenebbox.nearPlane, scenebbox.farPlane);

	// Initialize player camera before directX
	XMFLOAT3 startPosition = XMFLOAT3(0.5f, 2.0f, -4.0f);
	XMVECTOR startForward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	mainCamera = new Camera(startPosition, startForward);
	mainCamera->setResolution(screenWidth, screenHeight);
	mainCamera->setSceneBBox(scenebbox);

	// Initialize sun camera
	XMFLOAT3 sunPosition = XMFLOAT3(0.0f, 100.0f , 0.0f ) ;
	XMVECTOR sunForward = XMVector3Normalize(XMVectorSubtract(XMVectorZero(), XMLoadFloat3(&sunPosition)));
	sunCamera = new Camera(sunPosition, sunForward);
	sunCamera->setResolution(mainCamera->getResolution().first, mainCamera->getResolution().second);


	//// TODO: Renderer is the one that should be initialized and used. With this we can add new APIs in the future
	////m_Renderer = std::make_unique<D3DClass>();

	//// Create window
	//m_D3D = new D3DClass();
	//bool result = m_D3D->Initialize(hwnd, mScene, mainCamera);
	//if (!result)
	//{
	//	return false;
	//}
	//-------------------------------------------------------------------------------------------------------------------------------
	m_shadowPass = std::make_unique<ShadowPass>();
	m_shadowPass->setup(*m_renderer, m_resourceManager, hwnd, sunCamera);

	m_opaquePass = std::make_unique<OpaquePass>();
	m_opaquePass->setup(*m_renderer, *m_resourceManager, hwnd, mainCamera, sunCamera); //sunCamera needed for shadowPass

	m_skyPass = std::make_unique<SkyPass>();
	m_skyPass->setup(*m_renderer, *m_resourceManager, hwnd, mainCamera);

	return true;
}

bool GraphicsClass::Frame()
{
	PROFILE_SCOPE();

	//// In the future I may want to dynamically change sun position, hence we create sunCamera here :)
	//float scale = 1.0f;
	////offset -= (fabs(offset) < 3) ? 0.001 : 0.0;
	//offset = 0;
	////XMFLOAT3 sunPosition = XMFLOAT3(1.0f * scale, 0.5f * scale, -0.5f + offset);
	//XMFLOAT3 sunPosition = XMFLOAT3(0.0f, 100.0f , 0.0f ) ;
	//XMVECTOR sunDirection = XMVector3Normalize(XMVectorSubtract(XMVectorZero(), XMLoadFloat3(&sunPosition)));
	//Camera* sunCamera = new Camera(sunPosition, sunDirection);
	//sunCamera->setResolution(mainCamera->getResolution().first, mainCamera->getResolution().second);
	//m_D3D->sunCamera = sunCamera;
	//m_D3D->sunActive = sunActive;

	//m_D3D->ClearBuffer(0.0f, 0.0f, 0.0f);
	////m_D3D->DrawShadowMap(mScene, sunCamera);

	//scenebbox = m_D3D->ComputeSunFrustum();
	//sunCamera->setSceneBBox(scenebbox);
	//m_D3D->DrawScene(mScene, mainCamera);
	//m_D3D->DrawSky(mScene, mainCamera);
	//m_D3D->DrawDebug(mScene, mainCamera);
	//m_D3D->EndScene();
	sunCamera->setSceneBBox(ComputeSunFrustum(mainCamera, sunCamera));

	// I should clear target and here
	((D3D11Renderer*)m_renderer.get())->BeginRenderFrame(); // ugly :(
	// Shadow pass here. where do i save the generated shadow map
	m_shadowPass->execute(*m_sceneLoader->pScene.get(), renderItems);
	m_opaquePass->sunActive = sunActive;
	m_opaquePass->execute(*m_sceneLoader->pScene.get(), renderItems);
	m_skyPass->execute(*m_sceneLoader->pScene.get(), skyItem);
	// I should present and swap here
	((D3D11Renderer*)m_renderer.get())->EndRenderFrame(); // ugly :(

	//float color = sin(count);
	//count += 0.03f;
	//m_D3D->ClearBuffer(color, color, 0.5f);
	//std::ostringstream oss;
	//oss << mouse->GetPosX() << "," << mouse->GetPosY();
	//SetWindowTextA(m_hwnd, oss.str().c_str());
	////Division is to normalize mouse position. Division is done to go from [0,2]
	////Because we want [-1,1] we substract 1. And there is a need to invert the Y position
	//m_D3D->DrawTestTriangle(count,
	//	0.0f,
	//	0.0f);
	//m_D3D->DrawTestTriangle(count + 2.0f, 
	//	mouse->GetPosX() / 400.0f - 1.0f, 
	//	-(mouse->GetPosY() / 300.0f - 1.0f));
	//m_D3D->EndScene();

	return true;
}

bool GraphicsClass::UpdateCamera(Axis axis, int sign)
{
	PROFILE_SCOPE();

	mainCamera->moveCamera(axis, sign);

	return true;
}

bool GraphicsClass::UpdateCameraLookAt(float x, float y)
{
	PROFILE_SCOPE();

	mainCamera->updateYawPitch(x, y);

	return true;
}

bool GraphicsClass::RotateCamera(int sign)
{
	PROFILE_SCOPE();

	mainCamera->updateRoll(sign);
	return true;
}

void GraphicsClass::Shutdown()
{
	//if (importer != nullptr)
	//{
	//	delete importer;
	//}
	//else if (mScene != nullptr)
	//{
	//	delete mScene;
	//}
	//else if (mainCamera != nullptr)
	//{
	//	delete mainCamera;
	//}

	//m_D3D->Shutdown();
	return;
}

