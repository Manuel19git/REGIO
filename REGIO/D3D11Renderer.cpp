#include "D3D11Renderer.h"

D3D11Renderer::D3D11Renderer()
{
}

D3D11Renderer::~D3D11Renderer()
{
}

bool D3D11Renderer::CreateBuffer( const void* data, ID3D11Buffer** outBuffer, D3D11_BUFFER_DESC bufferDesc)
{
    HRESULT hr;

    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = data;

    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, outBuffer));

	return true;
}

bool D3D11Renderer::CreateVertexShader(std::string shaderPath, ID3D11VertexShader* pVertexShader, ID3D11InputLayout* pInputLayout)
{
    HRESULT hr;

    // Read vertex shader
    wrl::ComPtr<ID3DBlob> pBlob;
	std::wstring wShaderPath = string2WideString(shaderPath);
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(wShaderPath.c_str(), &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

    // Build Vertex Layout
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));


    return true;
}

bool D3D11Renderer::CreatePixelShader(std::string shaderPath, ID3D11PixelShader* pPixelShader)
{
    HRESULT hr;

    // Read pixel shader
    wrl::ComPtr<ID3DBlob> pBlob;
	std::wstring wShaderPath = string2WideString(shaderPath);
	GFX_THROW_INFO_ONLY(D3DReadFileToBlob(wShaderPath.c_str(), &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));


    return true;
}


// In here I would put everything that needs to be done before the render/game loop
void D3D11Renderer::ConfigureRenderPass(HWND hWnd)
{
    HRESULT hr;

    // Configure Swap Chain
    DXGI_SWAP_CHAIN_DESC desc = {};
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


    //Create Stencil Buffer
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE; 
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthBuffer));
    
    //Create Depth Stencil Texture
    D3D11_TEXTURE2D_DESC depthTextureDesc = {};
    // When creating swap chain it width/height are set to fetch the size of output window (I suppose it will know the value here)
    // removing the need to pass the camera to the configuration of the pipeline
    depthTextureDesc.Width = desc.BufferDesc.Width; //Width has to be the same as swap chain
    depthTextureDesc.Height = desc.BufferDesc.Height; //Height has to be the same as swap chain
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT; //Each element of buffer has 32 bits of float type to store depth
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthTexture;
    GFX_THROW_INFO(pDevice->CreateTexture2D(&depthTextureDesc, nullptr, &pDepthTexture));

    //Create view of depth stencil texture
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthTexture.Get(), &depthStencilViewDesc, &pDepthStencilView));


    // Load all shaders (from where? this info should be in each renderItem material handle
    // Solution 1: resource manager loads them (it also has access to renderer) YES!


    // states here? (maybe if I want to change states while rendering I could configure states in the loop)

    // Sampler states (I don't think this will be changed every iteration)
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    GFX_THROW_INFO(pDevice->CreateSamplerState(&sampDesc, pSamplerState.GetAddressOf()));


    // I can create constant buffers here if needed

    //Initialize sprint font and batch to render text (for debugging purposes)
	std::wstring spriteFontPath = string2WideString(searchFileInParentDirectories("/Data/Fonts/arial.spritefont"));
    spriteBatch = std::make_unique<SpriteBatch>(pDeviceContext.Get());
    spriteFont = std::make_unique<SpriteFont>(pDevice.Get(), spriteFontPath.c_str());


    start = std::chrono::system_clock::now();
}

void D3D11Renderer::BeginRenderPass()
{
    // Clear buffer
    const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pDeviceContext->ClearRenderTargetView(pTarget.Get(), color);
    pDeviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11Renderer::EndRenderPass()
{

}

void D3D11Renderer::DrawSky()
{

}
bool D3D11Renderer::ClearBuffer()
{
    return false;
}

