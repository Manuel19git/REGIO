#include <iostream>
#include "d3dclass.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


D3DClass::D3DClass()
{}


void DirectXError(HRESULT hr, const std::string& Msg, const std::string& File, int Line)
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


    UINT swapCreateFlags = 0u;
#ifndef NDEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    //I have to initialize hr in order for the macro to work
    HRESULT hr;

    GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        swapCreateFlags,
        nullptr,
        0,                          //Give me all feature levels you can
        D3D11_SDK_VERSION,
        &desc,
        &pSwap,
        &pDevice,
        nullptr,
        &pDeviceContext
    ));

    //Get back buffer from the swap chain
    Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
    GFX_THROW_INFO( pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer) );

    GFX_THROW_INFO( pDevice->CreateRenderTargetView(
        pBackBuffer.Get(),
        nullptr,
        &pTarget
    ));

    return true;
}

//This method will be in charge of flipping (Taking the back buffer and presenting it as the front)
void D3DClass::EndScene()
{
    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set();
#endif 


    //SyncInterval of 1u would mean 60 fps. And 2u for 30fps
    if (FAILED( hr = pSwap->Present(1u, 0u)))
    {
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
        }
        else
        {
            GFX_THROW_INFO(hr);
        }
    }
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

void D3DClass::ClearBuffer(float red, float green, float blue)
{
    const float color[] = { red, green, blue, 1.0f };
    pDeviceContext->ClearRenderTargetView(pTarget.Get(), color);
}

void D3DClass::DrawTestTriangle()
{
    HRESULT hr;
    namespace wrl = Microsoft::WRL;

    //Creation of buffer with vertex for triangle
    struct Vertex
    {
        float x;
        float y;
    };
    const Vertex triangle[] = {
        {-0.5, 0.5},
        {0.5, -0.5},
        {-0.5, -0.5}
    };
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = sizeof(triangle);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = triangle;
    wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffer));

    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    //Bind vertex buffer to pipeline (Side note: this method usually doesn't show the errors so throwing here doesn't make sense)
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &strides, &offset));

    //Create vertex shader
    wrl::ComPtr<ID3D11VertexShader> pVertexShader;
    wrl::ComPtr<ID3DBlob> pBlob;
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

    //Bind vertex to context
    pDeviceContext->VSSetShader(pVertexShader.Get(), 0, 0);

    GFX_THROW_INFO_ONLY(pDeviceContext->Draw((UINT)std::size(triangle), 0u));
}

//Here we implement hr exceptions

D3DClass::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs)
    :
    MyException(line, file),
    hr(hr)
{
    //join messages
    for (const auto& m : infoMsgs)
    {
        info += m;
        info.push_back('\n');
    }
    //remove final line if exists
    if (!info.empty())
    {
        info.pop_back();
    }
}

const char* D3DClass::HrException::what() const
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << "(" << (unsigned long)GetErrorCode() << ")" << std::endl
        << "[Error String] " << GetErrorString() << std::endl
        << "[Error Description] " << GetErrorDescription() << std::endl;
    if (!info.empty())
    {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* D3DClass::HrException::GetType() const
{
    return "My Graphics Exception";
}
HRESULT D3DClass::HrException::GetErrorCode() const
{
    return hr;
}
std::string D3DClass::HrException::GetErrorString() const
{
    return DXGetErrorStringA(hr);
}
std::string D3DClass::HrException::GetErrorDescription() const
{
    char buf[512];
    DXGetErrorDescriptionA(hr, buf, sizeof(buf));
    return buf;
}
std::string D3DClass::HrException::GetErrorInfo() const
{
    return info;
}

D3DClass::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs)
    :
    MyException(line, file)
{
    //join messages
    for (const auto& m : infoMsgs)
    {
        info += m;
        info.push_back('\n');
    }
    //remove final line if exists
    if (!info.empty())
    {
        info.pop_back();
    }
}

const char* D3DClass::InfoException::what() const
{
    std::ostringstream oss;
    oss << GetType() << std::endl;
    if (!info.empty())
    {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* D3DClass::InfoException::GetType() const
{
    return "Output Info Only Exception";
}

std::string D3DClass::InfoException::GetErrorInfo() const
{
    return info;
}


const char* D3DClass::DeviceRemovedException::GetType() const
{
    return "My Graphics Exception [Device Removed] DXGI_ERROR_DEVICE_REMOVED";
}


