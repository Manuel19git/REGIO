////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


//////////////
// INCLUDES //
//////////////
#include "D3dclass.h"
#include "MyException.h"
#include "Inputclass.h"
#include "Interfaces/IRenderer.h"
#include "ResourceManager.h"
#include "SceneLoader.h"

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND, InputClass*, std::wstring scenePath);
	void Shutdown();
	bool Frame();
	float count = 0;

	bool UpdateCamera(Axis axis, int sign);
	bool UpdateCameraLookAt(float x, float y);
	bool RotateCamera(int sign);

	// Switch the sun on/off
	void switchSun(bool value) { sunActive = value; };

private:
	
	std::unique_ptr<IRenderer> m_renderer;
	std::unique_ptr<ResourceManager> m_resourceManager;

	D3DClass* m_D3D;
	Mouse* mouse;
	HWND m_hwnd;

	Camera* mainCamera;

	//Scene and mesh persists as long as the importer
	Assimp::Importer* importer;
	const aiScene* mScene;

	BoundingBox scenebbox;

	float offset = 0.0f;

	bool sunActive = true;

};

#endif