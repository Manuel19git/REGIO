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


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, InputClass* m_Input)
{
	mouse = &m_Input->mouse;
	m_hwnd = hwnd;

	//Read mesh
	importer = new Assimp::Importer();
	mScene = importer->ReadFile("..\\output\\Maxwell_cat\\source\\maxwell_scene.obj",
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	//Create window
	m_D3D = new D3DClass();
	bool result = m_D3D->Initialize(hwnd, mScene);
	if (!result)
	{
		return false;
	}

	//Initialize camera
	mainCamera = new Camera();

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


bool GraphicsClass::Frame()
{
	m_D3D->ClearBuffer(0.0f, 0.0f, 0.0f);
	m_D3D->DrawScene(mScene, mainCamera);
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
