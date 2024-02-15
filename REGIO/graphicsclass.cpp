////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"
#include <time.h>


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

	//Create window

	m_D3D = new D3DClass();
	bool result = m_D3D->Initialize(hwnd);
	if (!result)
	{
		return false;
	}
	return true;
}


void GraphicsClass::Shutdown()
{
	m_D3D->Shutdown();
	return;
}


bool GraphicsClass::Frame()
{
	float color = sin(count);
	count += 0.03f;
	m_D3D->ClearBuffer(color, color, 0.5f);
	std::ostringstream oss;
	oss << mouse->GetPosX() << "," << mouse->GetPosY();
	SetWindowTextA(m_hwnd, oss.str().c_str());
	//Division is to normalize mouse position. Division is done to go from [0,2]
	//Because we want [-1,1] we substract 1. And there is a need to invert the Y position
	m_D3D->DrawTestTriangle(count,
		0.0f,
		0.0f);
	m_D3D->DrawTestTriangle(count + 2.0f, 
		mouse->GetPosX() / 400.0f - 1.0f, 
		-(mouse->GetPosY() / 300.0f - 1.0f));
	m_D3D->EndScene();

	return true;
}