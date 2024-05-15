#include <iostream>
#include "d3dclass.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


D3DClass::D3DClass()
{}


void DirectXError(HRESULT hr, const std::string& Msg, const std::string& File, int Line)
{
}

void D3DClass::BuildGeometry(const aiScene* pScene)
{
    HRESULT hr;

    //Define size and Initialize all offset arrays to zero
    pVertexOffsets = new UINT[pScene->mNumMeshes]();
    pIndexOffsets = new UINT[pScene->mNumMeshes]();
    pIndexCount = new UINT[pScene->mNumMeshes]();

    //I am going to take advantage of the for loops to fill the totalCount for vertex and index
    UINT totalVertexCount = pScene->mMeshes[0]->mNumVertices;
    UINT totalIndexCount = 0;
    
    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        pIndexCount[i] = pScene->mMeshes[i]->mNumFaces * pScene->mMeshes[i]->mFaces[0].mNumIndices;

        totalIndexCount += pIndexCount[i];
    }
    for (int i = 1; i < pScene->mNumMeshes; ++i)
    {
        pVertexOffsets[i] = pVertexOffsets[i - 1] + pScene->mMeshes[i - 1]->mNumVertices;
        pIndexOffsets[i] = pIndexOffsets[i - 1] + pIndexCount[i - 1];

        totalVertexCount += pScene->mMeshes[i]->mNumVertices;
    }
    
    //Fill vertices and index array with data from each mesh
    Vertex* vertices = new Vertex[totalVertexCount];
    u_short* indices = new u_short[totalIndexCount];
    for (int meshId = 0; meshId < pScene->mNumMeshes; ++meshId)
    {
        aiMesh* mesh = pScene->mMeshes[meshId];

        for (int vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
        {
            vertices[pVertexOffsets[meshId] + vertexId].pos.x = mesh->mVertices[vertexId].x;
            vertices[pVertexOffsets[meshId] + vertexId].pos.y = mesh->mVertices[vertexId].y;
            vertices[pVertexOffsets[meshId] + vertexId].pos.z = mesh->mVertices[vertexId].z;

            vertices[pVertexOffsets[meshId] + vertexId].normal.x = mesh->mNormals[vertexId].x;
            vertices[pVertexOffsets[meshId] + vertexId].normal.y = mesh->mNormals[vertexId].y;
            vertices[pVertexOffsets[meshId] + vertexId].normal.z = mesh->mNormals[vertexId].z;
        }

        for (int faceId = 0; faceId < mesh->mNumFaces; ++faceId)
        {
            for (int faceVertexId = 0; faceVertexId < mesh->mFaces->mNumIndices; ++faceVertexId)
            {
                indices[pIndexOffsets[meshId] + (faceId * mesh->mFaces->mNumIndices) + faceVertexId] = mesh->mFaces[faceId].mIndices[faceVertexId];
            }
        }
    }
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(Vertex) * totalVertexCount;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = vertices;
    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffer));


    D3D11_BUFFER_DESC indexDesc = {};
    //indexDesc.ByteWidth = sizeof(aiFace) * mesh->mNumFaces;
    indexDesc.ByteWidth = sizeof(u_short) * totalIndexCount;
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = sizeof(u_short);
    D3D11_SUBRESOURCE_DATA isd;
    isd.pSysMem = indices;
    GFX_THROW_INFO(pDevice->CreateBuffer(&indexDesc, &isd, &pIndexBuffer));

    delete[] vertices;
    delete[] indices;
}

void D3DClass::BuildVertexLayout()
{
    HRESULT hr;

    //D3D11_APPEND_ALIGNED_ELEMENT is a way to let d3d calculate the offset from previous element
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayout));
}

bool D3DClass::Initialize(HWND hWnd, const aiScene* pScene)
{
    //Configure Swap Chain
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

    //Create Stencil Buffer
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE; 
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthBuffer));
    
    //Create Depth Stencil Texture
    D3D11_TEXTURE2D_DESC depthTextureDesc = {};
    depthTextureDesc.Width = 800; //Width has to be the same as swap chain
    depthTextureDesc.Height = 600; //Height has to be the same as swap chain
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

    //Build Geometry
    BuildGeometry(pScene);

    //Build Effects
    GFX_THROW_INFO(D3DX11CreateEffectFromFile(L"LightEffect.fxo", 0, pDevice.Get(), pEffect.GetAddressOf()));
    pTechnique = pEffect->GetTechniqueByName("LighTech");

    //Build Vertex Layout
    BuildVertexLayout();

    //Textures here
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0; //This means highest level of detail
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    GFX_THROW_INFO(pDevice->CreateSamplerState(&sampDesc, samplerState.GetAddressOf()));


    //Create texture
    GFX_THROW_INFO(CreateWICTextureFromFile(
        pDevice.Get(), 
        L"C:\\Users\\Akira\\Desktop\\Proyectos\\REGIO\\output\\Maxwell_cat\\textures\\dingus_nowhiskers.jpg", 
        nullptr, 
        myTexture.GetAddressOf()));


    //Create light and material
    dirLight.Ambient    = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    dirLight.Diffuse    = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.Specular   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.Direction  = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    material.Ambient    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Diffuse    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Specular   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

    fxDirLight  = pEffect->GetVariableByName("gDirLight");
    fxEyePos    = pEffect->GetVariableByName("gEyePosW");
    fxTransform = pEffect->GetVariableByName("gTransform");
    fxMaterial  = pEffect->GetVariableByName("gMaterial");

    //Initialize sprint font and batch to render text
    spriteBatch = std::make_unique<SpriteBatch>(pDeviceContext.Get());
    spriteFont = std::make_unique<SpriteFont>(pDevice.Get(), L"..\\Data\\Fonts\\arial.spritefont");

    start = std::chrono::system_clock::now();

    return true;
}

void D3DClass::ClearBuffer(float red, float green, float blue)
{
    const float color[] = { red, green, blue, 1.0f };
    pDeviceContext->ClearRenderTargetView(pTarget.Get(), color);
    pDeviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DClass::DrawScene(const aiScene* scene, float z)
{
    HRESULT hr;

    //Bind Vertex Layout and Primitive Topology
    pDeviceContext->IASetInputLayout(pInputLayout.Get());
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Bind Vertex and Index buffer
    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0));

    //Set constant buffers
    DirectX::XMMATRIX transformation = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixTranslation(0.0f, -1.0f, z + 4.0f) *
            DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
            );

    fxDirLight->SetRawValue(&dirLight, 0, sizeof(DirectionalLight));
    XMFLOAT3 eyePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    fxEyePos->SetRawValue(&eyePos, 0, sizeof(XMFLOAT3));
    fxTransform->SetRawValue(&transformation, 0, sizeof(XMMATRIX));
    fxMaterial->SetRawValue(&material, 0, sizeof(Material));

    //Viewport
    D3D11_VIEWPORT viewport;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    pDeviceContext->RSSetViewports(1, &viewport);


    D3DX11_TECHNIQUE_DESC techDesc;
    pTechnique->GetDesc(&techDesc);
    for (UINT32 p = 0; p < techDesc.Passes; ++p)
    {
        for (int meshId = 0; meshId < scene->mNumMeshes; ++meshId)
        {
            pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext.Get());
            pDeviceContext->DrawIndexed(pIndexCount[meshId], pIndexOffsets[meshId], pVertexOffsets[meshId]);
        }
        
    }
}

//This method will be in charge of flipping (Taking the back buffer and presenting it as the front)
void D3DClass::EndScene()
{
    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set();
#endif 

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

    delete[] pVertexOffsets;
    delete[] pIndexOffsets;
    delete[] pIndexCount;
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


