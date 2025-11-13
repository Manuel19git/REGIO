#include "D3D11Renderer.h"

D3D11Renderer::D3D11Renderer(HWND hwnd)
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
    desc.OutputWindow = hwnd;


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

    // Retrieve screen resolution from hwnd
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
	monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

    // Retrieve window resolution from hwnd
	RECT rect;
	if (GetClientRect(hwnd, &rect))
	{
		screenWidth = rect.right - rect.left;
		screenHeight = rect.bottom - rect.top;
	}

    
    //Create Depth Stencil Texture
    D3D11_TEXTURE2D_DESC depthTextureDesc = {};
    // When creating swap chain it width/height are set to fetch the size of output window (I suppose it will know the value here)
    // removing the need to pass the camera to the configuration of the pipeline
    depthTextureDesc.Width = screenWidth; //Width has to be the same as swap chain
    depthTextureDesc.Height = screenHeight; //Height has to be the same as swap chain
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

bool D3D11Renderer::CreateVertexShader(std::string shaderPath, ID3D11VertexShader** pVertexShader, ID3D11InputLayout** pInputLayout)
{
    HRESULT hr;

    // Read vertex shader
    wrl::ComPtr<ID3DBlob> pBlob;
	std::wstring wShaderPath = string2WideString(shaderPath);
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(wShaderPath.c_str(), &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pVertexShader));

    // Build Vertex Layout
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pInputLayout));


    return true;
}

bool D3D11Renderer::CreatePixelShader(std::string shaderPath, ID3D11PixelShader** pPixelShader)
{
    HRESULT hr;

    // Read pixel shader
    wrl::ComPtr<ID3DBlob> pBlob;
	std::wstring wShaderPath = string2WideString(shaderPath);
	GFX_THROW_INFO_ONLY(D3DReadFileToBlob(wShaderPath.c_str(), &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pPixelShader));


    return true;
}

bool D3D11Renderer::CreateTexture(std::string texturePath, ID3D11ShaderResourceView** textureResourceView)
{
    HRESULT hr;

	// Convert the string to the right type before feeding it to the following function
	const size_t pathSize = strlen(texturePath.c_str()) + 1;
	wchar_t* pathWideString = new wchar_t[pathSize];
	size_t retVal;
	mbstowcs_s(&retVal, pathWideString, pathSize, texturePath.c_str(), pathSize - 1);

	GFX_THROW_INFO(CreateWICTextureFromFile(
		pDevice.Get(),
		pathWideString,
		nullptr,
		textureResourceView));

    return true;
}


// In here I would put everything that needs to be done before the render/game loop
void D3D11Renderer::ConfigureRenderPass(HWND hwnd)
{
    HRESULT hr;

    // Load all shaders (from where? this info should be in each renderItem material handle
    // Solution 1: resource manager loads them (it also has access to renderer) YES!
    // Solution 2: I could load default shaders here (THose that I implement) and add them to an array/dictionary of shaders
    // and if a material comes with its own shader (.cso) I could add it to the array and the renderItem could access it


    // states here? (maybe if I want to change states while rendering I could configure states in the loop)

    // For now I am going to use no culling because in blender I have objects with only planes not volume
    // (TODO: In the future the scene should have all visible faces with its normals pointing towards camera)
    //Create rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
    GFX_THROW_INFO(pDevice->CreateRasterizerState(&rasterizerDesc, &pNoCullRS));


    // Sampler states (I don't think this will be changed every iteration)
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    GFX_THROW_INFO(pDevice->CreateSamplerState(&sampDesc, pSamplerState.GetAddressOf()));


    // Create the structs for constant buffers
	D3D11_BUFFER_DESC constDesc = {};
    constDesc.Usage = D3D11_USAGE_DYNAMIC;
    constDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    constDesc.ByteWidth = sizeof(cbPerFrame);
    GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc, nullptr, &pFrameConstantBuffer));

    constDesc.ByteWidth = sizeof(cbPerObject);
    GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc, nullptr, &pObjectConstantBuffer));



    // Set viewport here
    D3D11_VIEWPORT viewport;
    viewport.Width = monitorWidth;
    viewport.Height = monitorHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    pDeviceContext->RSSetViewports(1, &viewport);

    //Initialize sprint font and batch to render text (for debugging purposes)
	std::wstring spriteFontPath = string2WideString(searchFileInParentDirectories("/Data/Fonts/arial.spritefont"));
    spriteBatch = std::make_unique<SpriteBatch>(pDeviceContext.Get());
    spriteFont = std::make_unique<SpriteFont>(pDevice.Get(), spriteFontPath.c_str());


    start = std::chrono::system_clock::now();
}

void D3D11Renderer::SetVertexLayoutAndTopology(ID3D11InputLayout* inputLayout, D3D_PRIMITIVE_TOPOLOGY topology)
{
    pDeviceContext->IASetInputLayout(inputLayout);
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11Renderer::SetMeshBuffers(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer)
{
    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0));
}

void D3D11Renderer::SetObjectConstantBufferVS(const void* bufferData, size_t bufferSize, int registerId)
{
    // TODO: Mapping and unmapping must be a costly task, I have to think of a way to optimize this
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    pDeviceContext->Map(pObjectConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, bufferData, bufferSize);
	pDeviceContext->Unmap(pObjectConstantBuffer.Get(), 0);
    GFX_THROW_INFO_ONLY(pDeviceContext->VSSetConstantBuffers(registerId, 1, pObjectConstantBuffer.GetAddressOf()));
}

void D3D11Renderer::SetObjectConstantBufferPS(const void* bufferData, size_t bufferSize, int registerId)
{
    // TODO: Mapping and unmapping must be a costly task, I have to think of a way to optimize this
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    pDeviceContext->Map(pObjectConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, bufferData, bufferSize);
	pDeviceContext->Unmap(pObjectConstantBuffer.Get(), 0);
    GFX_THROW_INFO_ONLY(pDeviceContext->PSSetConstantBuffers(registerId, 1, pObjectConstantBuffer.GetAddressOf()));
}

void D3D11Renderer::SetFrameConstantBufferPS(const void* bufferData, size_t bufferSize, int registerId)
{
    // TODO: Mapping and unmapping must be a costly task, I have to think of a way to optimize this
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    pDeviceContext->Map(pFrameConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, bufferData, bufferSize);
	pDeviceContext->Unmap(pFrameConstantBuffer.Get(), 0);
    GFX_THROW_INFO_ONLY(pDeviceContext->PSSetConstantBuffers(registerId, 1, pFrameConstantBuffer.GetAddressOf()));
}

// This sucks!
void D3D11Renderer::SetShaderResourcePS(ID3D11ShaderResourceView* shaderResource, int registerId)
{
    GFX_THROW_INFO_ONLY(pDeviceContext->PSSetShaderResources(registerId, 1, &shaderResource));
}

void D3D11Renderer::SetShaders(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
    pDeviceContext->VSSetShader(vertexShader, 0, 0);
    pDeviceContext->PSSetShader(pixelShader, 0, 0);
}

void D3D11Renderer::DrawItem(uint32_t indexCount)
{
    // We don't need offsets for vertex/index buffers because we know (at least for now) that those buffers contains only one mesh
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void D3D11Renderer::BeginRenderPass()
{
    // Clear buffer
    const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pDeviceContext->ClearRenderTargetView(pTarget.Get(), color);
    pDeviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // I still don't know where to put this, so until then it stays here
    pDeviceContext->RSSetState(pNoCullRS.Get());
}

void D3D11Renderer::EndRenderPass()
{
    PROFILE_SCOPE();

    HRESULT hr;

    static int fpsCounter = 0;
    fpsCounter++;

    auto end = std::chrono::system_clock::now();

    auto now = end - start;
    static std::wstring wst = L"FPS: ";
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 1000)
    {
        wst = L"FPS: " + std::to_wstring(fpsCounter);
        fpsCounter = 0;
        start = std::chrono::system_clock::now();
    }
    //wst = std::to_wstring(spotLight.Position.x) + L"," + std::to_wstring(spotLight.Position.y) + L"," + std::to_wstring(spotLight.Position.z);
    
    //XMFLOAT3 forward;
    //XMStoreFloat3(&forward, camera->getForward());
    //wst = std::to_wstring(forward.x) + L"," + std::to_wstring(forward.y) + L"," + std::to_wstring(forward.z);
    //if (std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 10)
		//wst = std::to_wstring(camera->getYaw()) + L"," + std::to_wstring(camera->getPitch());
    const wchar_t* value = wst.c_str();

    spriteBatch->Begin();
    spriteFont->DrawString(spriteBatch.get(), value, XMFLOAT2(0, 0), Colors::White, 0.0f, XMFLOAT2(0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f));
    spriteBatch->End();

    ///I have to bind depth nbuffer and stencil after spriteBatch::End() to avoid problems with depth buffer
    GFX_THROW_INFO_ONLY(pDeviceContext->OMSetDepthStencilState(pDepthBuffer.Get(), 1u));
    GFX_THROW_INFO_ONLY(pDeviceContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get()));


    //SyncInterval of 1u would mean 60 fps. And 2u for 30fps
    if (FAILED(hr = pSwap->Present(1u, 0u)))
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

void D3D11Renderer::DrawSky()
{

}
bool D3D11Renderer::ClearBuffer()
{
    return false;
}

