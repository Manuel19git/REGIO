#include "d3dclass.h"

#pragma comment(lib, "d3d11.lib")

D3DClass::D3DClass()
{
	pDevice = 0;
	pDeviceContext = 0;
    pSwap = 0;
}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(HWND hWnd)
{
    //Configure Swap Chain
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
    desc.BufferCount = 1; //This specifies that we want 1 back buffer (1 front and 1 back)
    desc.BufferDesc.Width = 0; //Lets him figure out the width and height
    desc.BufferDesc.Height = 0;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;      //multisampling setting (With this we specify that we don't want antialiasing)
    desc.SampleDesc.Quality = 0;    //vendor-specific flag
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.OutputWindow = hWnd;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,                          //Give me all feature levels you can
        D3D11_SDK_VERSION,
        &desc,
        &pSwap,
        &pDevice,
        nullptr,
        &pDeviceContext
    );

    //Get back buffer from the swap chain
    ID3D11Resource* pBackBuffer = nullptr;
    pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));

    pDevice->CreateRenderTargetView(
        pBackBuffer,
        nullptr,
        &pTarget
    );

    //Once the target view is created we don't need the handler to the back buffer
    pBackBuffer->Release();

    return true;
}

//This method will be in charge of flipping (Taking the back buffer and presenting it as the front)
void D3DClass::EndScene()
{
    //SyncInterval of 1u would mean 60 fps. And 2u for 30fps
    pSwap->Present(1u, 0u);
}

void D3DClass::Shutdown()
{
    if (pTarget != nullptr)
    {
        pTarget->Release();
    }
    if (pSwap != nullptr)
    {
        pSwap->Release();
    }
    if (pDeviceContext != nullptr)
    {
        pDeviceContext->Release();
    }
    if (pDevice != nullptr)
    {
        pDevice->Release();
    }
}