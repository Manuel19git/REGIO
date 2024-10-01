#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN
//Unless we define this NOMINMAX, whenever we includ winowds.h its going to give an error
#define NOMINMAX

///////////////////////////////
// INCLUDES //
///////////////////////////////
//#include<Windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "inputclass.h"
#include "graphicsclass.h"

#define WND_EXCEPT( hr ) SystemClass::HrException( __LINE__,__FILE__,(hr) );
#define WND_LAST_EXCEPT() SystemClass::HrException( __LINE__,__FILE__,GetLastError() );
#define WND_NOGFX_EXCEPT() SystemClass::NoGfxException( __LINE__,__FILE__ );

class SystemClass
{
public:
	class HrException : public MyException
	{
	public:
		HrException(int line, const char* file, HRESULT hr);
		const char* what() const override;
		const char* GetType() const override;
		HRESULT GetErrorCode() const;
		std::string GetErrorDescription() const;
	private:
		HRESULT hr;
	};
	class NoGfxException : public MyException
	{
	public:
		using MyException::MyException;
		const char* GetType() const override;
	};
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	GraphicsClass* m_Graphics;

	int width;
	int height;
	bool isPause = false;
};

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static SystemClass* ApplicationHandle = 0;


#endif