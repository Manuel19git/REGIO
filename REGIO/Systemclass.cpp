////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Systemclass.h"
#include <sstream>

SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize(std::wstring scenePath)
{
	bool result;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create and initialize the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new InputClass;

	m_Input->Initialize(m_hwnd);

	// Create and initialize the graphics object.  This object will handle rendering all the graphics for this application.
	m_Graphics = new GraphicsClass;

	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd, m_Input, scenePath);
	if (!result)
	{
		return false;
	}

	return true;
}

void GetMonitorRealResolution(HMONITOR monitor, int* pixelsWidth, int* pixelsHeight)
{
    MONITORINFOEX info = { sizeof(MONITORINFOEX) };
    GetMonitorInfo(monitor, &info);
    DEVMODE devmode = {};
    devmode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devmode);
    *pixelsWidth = devmode.dmPelsWidth;
    *pixelsHeight = devmode.dmPelsHeight;
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get an external pointer to this object.
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"REGIO";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc; //Function that process messages sent to this window
	wc.cbClsExtra = 0;										//no need for extra class memory
	wc.cbWndExtra = 0;										//no need for extra window memory
	wc.hInstance = m_hinstance;								
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				//Handle to cursor displayed when mouse on window (default arrow cursor)
	wc.hbrBackground = nullptr; //Black background
	wc.lpszMenuName = NULL;									//Window doesn't have menu	
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);							//Size of window class structure

	// Register the window class.
	RegisterClassEx(&wc);

	// I just want to get the screen size my guy, why was it so hard ToT
	POINT pt = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	GetMonitorRealResolution(monitor, &screenWidth, &screenHeight);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		
		// Place the window in the middle of the screen.
		posX = screenWidth / 2;
		posY = screenHeight / 2;
		m_hwnd = CreateWindowEx(0, m_applicationName, m_applicationName, WS_POPUP,
                0, 0, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		//Adjust window
		RECT wr;
		wr.left = 100;
		wr.right = screenWidth + wr.left;
		wr.top = 100;
		wr.bottom = screenHeight + wr.top;
		AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - (wr.right - wr.left)) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - (wr.bottom - wr.top)) / 2;

		// Create the window with the screen settings and get the handle to it.
		m_hwnd = CreateWindowEx(0, m_applicationName, m_applicationName, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			posX, posY, (wr.right - wr.left), (wr.bottom - wr.top), NULL, NULL, m_hinstance, NULL);
	}

	// If I don't put this here, the scale of the monitor could mess with other operations. 
	// Because I am going to be using the standard screen size and not the scaled one
	SetProcessDPIAware();

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Constrain cursor to window
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);

	POINT ul = { clientRect.left, clientRect.top };
	POINT lr = { clientRect.right, clientRect.bottom };
	ClientToScreen(m_hwnd, &ul);
	ClientToScreen(m_hwnd, &lr);
	RECT clipRect = { ul.x, ul.y, lr.x, lr.y };
	ClipCursor(&clipRect);

	// Show the mouse cursor.
	ShowCursor(false);

	return;

}

void SystemClass::Shutdown()
{
	// Release the graphics object.
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Release the input object.
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// Shutdown the window.
	ShutdownWindows();

	return;
}

void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}

bool SystemClass::Frame()
{
	bool result;

	// Adjust window
	RECT clientRect = RECT();
	POINT clientTopLeft = { clientRect.left, clientRect.top };
	ClientToScreen(m_hwnd, &clientTopLeft);

	int centerX = clientTopLeft.x + (screenWidth / 2);
	int centerY = clientTopLeft.y + (screenHeight / 2);


	// Check if the user pressed escape and wants to exit the application.
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}
	if (m_Input->IsKeyDown('P'))
	{
		isPause = true;
		ShowCursor(true);
		ClipCursor(nullptr);
	}
	if (m_Input->IsKeyDown(VK_RETURN))
	{
		isPause = false;
		SetCursorPos(centerX, centerY);
		while (ShowCursor(false) >= 0) {} //ShowCursor keeps count for everytime it was called
	}
	if (isPause)
		return true;

	if (m_Input->IsKeyDown('W'))
	{
		m_Graphics->UpdateCamera(Z, POSITIVE);
	}
	if (m_Input->IsKeyDown('S'))
	{
		m_Graphics->UpdateCamera(Z, NEGATIVE);
	}
	if (m_Input->IsKeyDown('D'))
	{
		m_Graphics->UpdateCamera(X, POSITIVE);
	}
	if (m_Input->IsKeyDown('A'))
	{
		m_Graphics->UpdateCamera(X, NEGATIVE);
	}
	if (m_Input->IsKeyDown('F'))
	{
		m_Graphics->UpdateCamera(Y, POSITIVE);
	}
	if (m_Input->IsKeyDown('G'))
	{
		m_Graphics->UpdateCamera(Y, NEGATIVE);
	}
	if (m_Input->IsKeyDown('Q'))
	{
		m_Graphics->RotateCamera(POSITIVE);
	}
	if (m_Input->IsKeyDown('E'))
	{
		m_Graphics->RotateCamera(NEGATIVE);
	}
	if (m_Input->IsKeyDown('I'))
	{
		m_Graphics->switchSun(true);
	}
	if (m_Input->IsKeyDown('O'))
	{
		m_Graphics->switchSun(false);
	}

	
	// Process previous mouse delta movements and get the medium delta
	float xDelta = m_Input->mouse.GetPosXDelta();
	float yDelta = m_Input->mouse.GetPosYDelta();
	m_Graphics->UpdateCameraLookAt(xDelta, yDelta);

	m_Input->mouse.RestartMouseMoveState(); // By restarting mouse state, when I don't receive raw input, it writes the xy to zero

	// Do the frame processing for the graphics object.
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
		// We must process all (using "while" instead of "if") previous window messages before rendering current frame
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}
		/*if (!m_Input->mouse.IsEmpty())
		{
			std::ostringstream oss;
			const auto e = m_Input->mouse.Read();
			switch (e->GetType())
			{
			case Mouse::Event::Type::Leave:
			{
				oss << "GONE!";
				break;
			}

			case Mouse::Event::Type::Move:
				oss << "Mouse moved to (" << (e->GetPosX() / (width / 2.0f) - 1.0f) << "," << (e->GetPosY() / (height / 2.0f) - 1.0f) << ")";
				
			}
			SetWindowTextA(m_hwnd, oss.str().c_str());
		}*/
		

	}

	return;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	// New CHECK MOUSE MESSAGES WITH RAW INPUT API
	case WM_INPUT:
	{
		// isPause stops windows queue from registering mouse moves when in pause
		if (isPause)
			break;
		UINT dwSize;
		GetRawInputData(
			(HRAWINPUT)lparam,
			RID_INPUT,
			NULL,
			&dwSize,
			sizeof(RAWINPUTHEADER)
		);

		LPBYTE lpb = new BYTE[dwSize];
		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		if (lpb == NULL)
			break;

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			m_Input->mouse.OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			break;
		}
		else if (raw->header.dwType == RIM_TYPEKEYBOARD && raw->data.keyboard.Message == WM_KEYDOWN && !m_Input->IsKeyDown(raw->data.keyboard.VKey))
		{
			m_Input->KeyDown(raw->data.keyboard.VKey);
			break;
		}
		else if (raw->header.dwType == RIM_TYPEKEYBOARD && raw->data.keyboard.Message == WM_KEYUP)
		{
			m_Input->KeyUp(raw->data.keyboard.VKey);
			break;
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lparam);
		m_Input->mouse.OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lparam);
		m_Input->mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lparam);
		m_Input->mouse.OnLeftReleased(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lparam);
		m_Input->mouse.OnRightReleased(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lparam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
		m_Input->mouse.OnWheelDelta(pt.x, pt.y, delta);

		/*if (GET_WHEEL_DELTA_WPARAM(wparam) > 0)
		{
			m_Input->mouse.OnWheelUp(pt.x, pt.y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wparam) < 0)
		{
			m_Input->mouse.OnWheelDown(pt.x, pt.y);
		}*/
		break;
	}

	// Any other messages send to the default message handler as our application won't make use of them.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

//Window exceptions
SystemClass::HrException::HrException(int line, const char* file, HRESULT hr)
	:
	MyException(line, file),
	hr(hr)
{}

const char* SystemClass::HrException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* SystemClass::HrException::GetType() const
{
	return "Chili Window Exception";
}

HRESULT SystemClass::HrException::GetErrorCode() const
{
	return hr;
}

std::string SystemClass::HrException::GetErrorDescription() const
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}


const char* SystemClass::NoGfxException::GetType() const
{
	return "Chili Window Exception [No Graphics]";
}