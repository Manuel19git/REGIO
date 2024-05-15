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
	mScene = importer->ReadFile("C:\\Users\\Akira\\Desktop\\Proyectos\\REGIO\\output\\Maxwell_cat\\source\\maxwell_scene.obj",
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	//Create window
	m_D3D = new D3DClass();
	bool result = m_D3D->Initialize(hwnd, mScene);
	if (!result)
	{
		return false;
	}
	return true;
}


void GraphicsClass::Shutdown()
{
	delete importer;
	delete mScene;

	m_D3D->Shutdown();
	return;
}


bool GraphicsClass::Frame()
{
	m_D3D->ClearBuffer(0.0f, 0.0f, 0.0f);
	m_D3D->DrawScene(mScene,
			-(mouse->GetPosY() / 300.0f - 1.0f));
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