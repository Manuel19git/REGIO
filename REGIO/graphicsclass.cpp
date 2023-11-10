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


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
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
	m_D3D->DrawTestTriangle(count);
	m_D3D->EndScene();

	return true;
}