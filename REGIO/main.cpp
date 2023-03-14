#include "systemclass.h"

//Punto de entrada para aplicaciones de windows
//hInstance -> Sistema operativo lo usa para localizar el EXE
//hPrevInstance -> no sirve para nada
//pCmdLine -> guarda la línea de comandos en string
//nCmdShow -> aplicación maximizada, minimizada o normal
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	SystemClass* systemClass = new SystemClass();
	bool result;

	//Inicializar ventana y algunas variables
	result = systemClass->Initialize();
	if (result)
	{
		systemClass->Run();
	}

	systemClass->Shutdown();
	delete systemClass;

	return 0;
}