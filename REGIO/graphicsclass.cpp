////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


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
void computeBoundingBox(const aiScene* scene, float& left, float& right, float& top, float& bottom, float &nearPlane, float &farPlane)
{
	left = FLT_MAX;
	right = -FLT_MAX;
	top = -FLT_MAX;
	bottom = FLT_MAX;
	nearPlane = FLT_MAX;
	farPlane = -FLT_MAX;

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			aiVector3D vertex = mesh->mVertices[j];
			if (vertex.x < left)
			{
				left = vertex.x;
			}
			if (vertex.x > right)
			{
				right = vertex.x;
			}
			if (vertex.y > top)
			{
				top = vertex.y;
			}
			if (vertex.y < bottom)
			{
				bottom = vertex.y;
			}
			if (vertex.z < nearPlane)
			{
				nearPlane = vertex.z;
			}
			if (vertex.z > farPlane)
			{
				farPlane = vertex.z;
			}
		}
	}

	// We want the bounding box to be a square box. In case it is a rectangle, it is corrected here
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

}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, InputClass* m_Input)
{
	mouse = &m_Input->mouse;
	m_hwnd = hwnd;

	// Read mesh
	importer = new Assimp::Importer();
	//mScene = importer->ReadFile("..\\output\\NIER\\Props\\turnstile_wall.usdc", //USD is not fully supported by assimp yet
	//	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	mScene = importer->ReadFile("..\\output\\Maxwell_cat\\source\\maxwell_scene.obj",
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	//mScene = importer->ReadFile("..\\output\\NIER\\nier_park.obj",
	//	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	//mScene = importer->ReadFile("..\\output\\NIER\\nier_park.glb",
	//	aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	// Compute bounding box
	computeBoundingBox(mScene, scenebbox.left, scenebbox.right, scenebbox.top, scenebbox.bottom, scenebbox.nearPlane, scenebbox.farPlane);

	// Initialize player camera before directX
	XMFLOAT3 startPosition = XMFLOAT3(0.5f, 2.0f, -4.0f);
	XMVECTOR startForward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	mainCamera = new Camera(startPosition, startForward, scenebbox);
	mainCamera->setResolution(screenWidth, screenHeight);

	// Create window
	m_D3D = new D3DClass();
	bool result = m_D3D->Initialize(hwnd, mScene, mainCamera);
	if (!result)
	{
		return false;
	}

	return true;
}

bool GraphicsClass::Frame()
{
	// In the future I may want to dynamically change sun position, hence we create sunCamera here :)
	float scale = 1.0f;
	//offset -= (fabs(offset) < 3) ? 0.001 : 0.0;
	XMFLOAT3 sunPosition = XMFLOAT3(1.0f * scale, 0.5f * scale, -0.5f + offset);
	XMVECTOR sunDirection = XMVector3Normalize(XMVectorSubtract(XMVectorZero(), XMLoadFloat3(&sunPosition)));
	Camera* sunCamera = new Camera(sunPosition, sunDirection, scenebbox);
	sunCamera->setResolution(mainCamera->getResolution().first, mainCamera->getResolution().second);
	m_D3D->sunCamera = sunCamera;

	m_D3D->ClearBuffer(0.0f, 0.0f, 0.0f);
	//m_D3D->DrawShadowMap(mScene, sunCamera);
	m_D3D->DrawScene(mScene, mainCamera);
	m_D3D->DrawSky(mScene, mainCamera);
	m_D3D->DrawDebug(mScene, mainCamera);
	m_D3D->EndScene();

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
	mainCamera->moveCamera(axis, sign);

	return true;
}

bool GraphicsClass::UpdateCameraLookAt(float x, float y)
{
	mainCamera->updateYawPitch(x, y);

	return true;
}

bool GraphicsClass::RotateCamera(int sign)
{
	mainCamera->updateRoll(sign);
	return true;
}

void GraphicsClass::Shutdown()
{
	if (importer != nullptr)
	{
		delete importer;
	}
	else if (mScene != nullptr)
	{
		delete mScene;
	}
	else if (mainCamera != nullptr)
	{
		delete mainCamera;
	}

	m_D3D->Shutdown();
	return;
}

