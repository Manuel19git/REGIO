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

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthBuffer;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthBuffer));
    //bind depth buffer
    GFX_THROW_INFO_ONLY(pDeviceContext->OMSetDepthStencilState(pDepthBuffer.Get(), 1u));

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
    //bind depth stencil view to pipeline
    GFX_THROW_INFO_ONLY(pDeviceContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get()));


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

    //Create effect
    GFX_THROW_INFO(D3DX11CreateEffectFromFile(L"LightEffect.fxo", 0, pDevice.Get(), pEffect.GetAddressOf()));
    pTechnique = pEffect->GetTechniqueByName("LighTech");

    
    pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

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

    return true;
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
    pDeviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DClass::DrawTestLight(const aiScene* scene, float angle, float z)
{
    HRESULT hr;
    namespace wrl = Microsoft::WRL;

    //Creation of buffer with vertex for triangle
    struct Vertex
    {
        struct
        {
            float x;
            float y;
            float z;
        } pos;

        struct
        {
            float x;
            float y;
            float z;
        } normal;

        struct
        {
            float u;
            float v;
        } tex;

    };

    //We suppose there is only one mesh in the scene
    aiMesh* mesh = scene->mMeshes[0];

    Vertex* vertices = new Vertex[mesh->mNumVertices];
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices[i].pos.x = mesh->mVertices[i].x;
        vertices[i].pos.y = mesh->mVertices[i].y;
        vertices[i].pos.z = mesh->mVertices[i].z;

        vertices[i].normal.x = mesh->mNormals[i].x;
        vertices[i].normal.y = mesh->mNormals[i].y;
        vertices[i].normal.z = mesh->mNormals[i].z;
    }


    D3D11_BUFFER_DESC bufferDesc = {};
    //bufferDesc.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
    bufferDesc.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = vertices;
    wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffer));


    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    //Bind vertex buffer to pipeline (Side note: this method usually doesn't show the errors so throwing here doesn't make sense)
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));


    aiFace* faces = mesh->mFaces;
    int total_indices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
    u_short* indices = new u_short[total_indices];
    char buffer[256];
    for (int i = 0; i < mesh->mNumFaces; ++i)
    {
        for (int j = 0; j < mesh->mFaces->mNumIndices; ++j)
        {
            indices[(i * mesh->mFaces->mNumIndices) + j] = mesh->mFaces[i].mIndices[j];
        }
    }
    

    D3D11_BUFFER_DESC indexDesc = {};
    //indexDesc.ByteWidth = sizeof(aiFace) * mesh->mNumFaces;
    indexDesc.ByteWidth = sizeof(u_short) * total_indices;
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = sizeof(u_short);
    D3D11_SUBRESOURCE_DATA isd;
    isd.pSysMem = indices;
    wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&indexDesc, &isd, &pIndexBuffer));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0));

    //Set constant buffers
    struct ConstantBuffer
    {
        DirectX::XMMATRIX transformation;
    };
    const ConstantBuffer cb =
    {
        //Multiply by 3/4 in the x axis, to fix the stretching taking place for the 4:3 aspect ratio of the viewport
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixTranslation(0.0f, -1.0f, z + 4.0f) *
            DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
        )
    };
    fxDirLight->SetRawValue(&dirLight, 0, sizeof(DirectionalLight));
    XMFLOAT3 eyePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    fxEyePos->SetRawValue(&eyePos, 0, sizeof(XMFLOAT3));
    fxTransform->SetRawValue(&cb.transformation, 0, sizeof(XMMATRIX));
    fxMaterial->SetRawValue(&material, 0, sizeof(Material));


    //Set Input Layout
    wrl::ComPtr<ID3D11InputLayout> pInputLayout;
    //D3D11_APPEND_ALIGNED_ELEMENT is a way to let d3d calculate the offset from previous element
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    //Blob en este caso debe ser del vertex shader, debe hacer la comprobaci�n de si el layout coincide con el del shade
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayout));

    //Bind Input Layout to pipeline
    pDeviceContext->IASetInputLayout(pInputLayout.Get());

    //Set primitive topology to triangle
    
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
        pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext.Get());

        pDeviceContext->DrawIndexed(total_indices, 0u, 0u);
    }
    
    delete[] vertices;
    delete[] indices;
}

void D3DClass::Draw(const aiScene* scene, float angle, float z)
{
    HRESULT hr;
    namespace wrl = Microsoft::WRL;

    //Creation of buffer with vertex for triangle
    struct Vertex
    {
        struct
        {
            float x;
            float y;
            float z;
        } pos;

        struct
        {
            float u;
            float v;
        } tex;
    };

    //We suppose there is only one mesh in the scene
    aiMesh* mesh = scene->mMeshes[0];

    Vertex* vertices = new Vertex[mesh->mNumVertices];
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices[i].pos.x = mesh->mVertices[i].x;
        vertices[i].pos.y = mesh->mVertices[i].y;
        vertices[i].pos.z = mesh->mVertices[i].z;

        vertices[i].tex.u = mesh->mTextureCoords[0][i].x;
        vertices[i].tex.v = mesh->mTextureCoords[0][i].y;
    }


    D3D11_BUFFER_DESC bufferDesc = {};
    //bufferDesc.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
    bufferDesc.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = vertices;
    wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffer));


    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    //Bind vertex buffer to pipeline (Side note: this method usually doesn't show the errors so throwing here doesn't make sense)
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));


    aiFace* faces = mesh->mFaces;
    int total_indices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
    u_short* indices = new u_short[total_indices];
    char buffer[256];
    for (int i = 0; i < mesh->mNumFaces; ++i)
    {
        for (int j = 0; j < mesh->mFaces->mNumIndices; ++j)
        {
            indices[(i * mesh->mFaces->mNumIndices) + j] = mesh->mFaces[i].mIndices[j];
        }
    }
    

    D3D11_BUFFER_DESC indexDesc = {};
    //indexDesc.ByteWidth = sizeof(aiFace) * mesh->mNumFaces;
    indexDesc.ByteWidth = sizeof(u_short) * total_indices;
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = sizeof(u_short);
    D3D11_SUBRESOURCE_DATA isd;
    isd.pSysMem = indices;
    wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&indexDesc, &isd, &pIndexBuffer));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0));

    //Create transform constant buffer
    struct ConstantBuffer
    {
        DirectX::XMMATRIX transformation;
    };
    const ConstantBuffer cb =
    {
        //DirectX::XMMatrixTranspose(
        //    DirectX::XMMatrixTranslation(0, 0, 0.0f)
        //)
        //Multiply by 3/4 in the x axis, to fix the stretching taking place for the 4:3 aspect ratio of the viewport
        DirectX::XMMatrixTranspose(/*
            DirectX::XMMatrixRotationZ(angle) *
            DirectX::XMMatrixRotationX(angle) **/
            DirectX::XMMatrixTranslation(0.0f, -1.0f, z + 4.0f) *
            DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
        )
    };

    D3D11_BUFFER_DESC constDesc = {};
    constDesc.ByteWidth = sizeof(cb);
    constDesc.Usage = D3D11_USAGE_DYNAMIC;
    constDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constDesc.MiscFlags = 0;
    constDesc.StructureByteStride = 0u;
    D3D11_SUBRESOURCE_DATA csd;
    csd.pSysMem = &cb;
    wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc, &csd, &pConstantBuffer));
    GFX_THROW_INFO_ONLY(pDeviceContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf()));

    //Create buffer to store the color of faces
    //struct ConstantBuffer2
    //{
    //    struct
    //    {
    //        float r;
    //        float g;
    //        float b;
    //        float a;
    //    } face_colors[6];
    //};
    //const ConstantBuffer2 cb2 =
    //{
    //    {
    //        {1.0f, 0.0f, 1.0f},
    //        {1.0f, 0.0f, 0.0f},
    //        {0.0f, 1.0f, 0.0f},
    //        {0.0f, 1.0f, 1.0f},
    //        {1.0f, 1.0f, 0.0f},
    //        {0.0f, 0.0f, 0.0f}
    //    }
    //};
    //D3D11_BUFFER_DESC constDesc2 = {};
    //constDesc2.ByteWidth = sizeof(cb2);
    //constDesc2.Usage = D3D11_USAGE_DEFAULT;
    //constDesc2.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    //constDesc2.CPUAccessFlags = 0u;
    //constDesc2.MiscFlags = 0;
    //constDesc2.StructureByteStride = 0u;
    //D3D11_SUBRESOURCE_DATA csd2;
    //csd2.pSysMem = &cb2;
    //wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
    //GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc2, &csd2, &pConstantBuffer2));
    //GFX_THROW_INFO_ONLY(pDeviceContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf()));

    //Create pixel shader
    wrl::ComPtr<ID3D11PixelShader> pPixelShader;
    wrl::ComPtr<ID3DBlob> pBlob;
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

    //Bind pixel shader to context
    pDeviceContext->PSSetShader(pPixelShader.Get(), 0, 0);


    //Create vertex shader
    wrl::ComPtr<ID3D11VertexShader> pVertexShader;
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

    //Bind vertex shader to context
    pDeviceContext->VSSetShader(pVertexShader.Get(), 0, 0);

    //Set Input Layout
    wrl::ComPtr<ID3D11InputLayout> pInputLayout;
    //D3D11_APPEND_ALIGNED_ELEMENT is a way to let d3d calculate the offset from previous element
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    //Blob en este caso debe ser del vertex shader, debe hacer la comprobaci�n de si el layout coincide con el del shader
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));

    //Bind Input Layout to pipeline
    pDeviceContext->IASetInputLayout(pInputLayout.Get());

    //Set primitive topology to triangle
    
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    //Set Texture resource
    pDeviceContext->PSSetShaderResources(0, 1, myTexture.GetAddressOf());
    //Set sampler state
    pDeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    //Viewport
    D3D11_VIEWPORT viewport;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    pDeviceContext->RSSetViewports(1, &viewport);


    //GFX_THROW_INFO_ONLY(pDeviceContext->DrawIndexed(std::size(indices), 0u, 0u));
    pDeviceContext->DrawIndexed(total_indices, 0u, 0u);

    delete[] vertices;
    delete[] indices;
}

void D3DClass::DrawTestTriangle(float angle, float x, float z)
{
    HRESULT hr;
    namespace wrl = Microsoft::WRL;

    //Creation of buffer with vertex for triangle
    struct Vertex
    {
        struct
        {
            float x;
            float y;
            float z;
        } pos;
    };
    //There has to be a better way to store extra values in vertex.
    Vertex vertices[] = {
        {-1.0f, -1.0f, -1.0f },
        {1.0f, -1.0f, -1.0f  },
        {-1.0f, 1.0f, -1.0f  },
        {1.0f, 1.0f, -1.0f   },
        {-1.0f, -1.0f, 1.0f  },
        {1.0f, -1.0f, 1.0f   },
        {-1.0f, 1.0f, 1.0f   },
        {1.0f, 1.0f, 1.0f    },
    };
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = vertices;
    wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffer));


    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    //Bind vertex buffer to pipeline (Side note: this method usually doesn't show the errors so throwing here doesn't make sense)
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));

    //Create index buffer
    const unsigned short indices[] =
    {
        0,2,1, 2,3,1,
        1,3,5, 3,7,5,
        2,6,3, 3,6,7,
        4,5,7, 4,7,6,
        0,4,2, 2,4,6,
        0,1,4, 1,5,4
    };
    D3D11_BUFFER_DESC indexDesc = {};
    indexDesc.ByteWidth = sizeof(indices);
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = sizeof(unsigned short);
    D3D11_SUBRESOURCE_DATA isd;
    isd.pSysMem = indices;
    wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&indexDesc, &isd, &pIndexBuffer));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0));

    //Create transform constant buffer
    struct ConstantBuffer
    {
        DirectX::XMMATRIX transformation;
    };
    const ConstantBuffer cb =
    {
        //Multiply by 3/4 in the x axis, to fix the stretching taking place for the 4:3 aspect ratio of the viewport
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixRotationZ(angle) *
            DirectX::XMMatrixRotationX(angle)*
            DirectX::XMMatrixTranslation(x, 0.0f, z + 4.0f) *
            DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
        )
    };

    D3D11_BUFFER_DESC constDesc = {};
    constDesc.ByteWidth = sizeof(cb);
    constDesc.Usage = D3D11_USAGE_DYNAMIC;
    constDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constDesc.MiscFlags = 0;
    constDesc.StructureByteStride = 0u;
    D3D11_SUBRESOURCE_DATA csd;
    csd.pSysMem = &cb;
    wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
    GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc, &csd, &pConstantBuffer));
    GFX_THROW_INFO_ONLY(pDeviceContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf()));

    //Create buffer to store the color of faces
    struct ConstantBuffer2
    {
        struct
        {
            float r;
            float g;
            float b;
            float a;
        } face_colors[6];
    };
    const ConstantBuffer2 cb2 =
    {
        {
            {1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}
        }
    };
    D3D11_BUFFER_DESC constDesc2 = {};
    constDesc2.ByteWidth = sizeof(cb2);
    constDesc2.Usage = D3D11_USAGE_DEFAULT;
    constDesc2.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    constDesc2.CPUAccessFlags = 0u;
    constDesc2.MiscFlags = 0;
    constDesc2.StructureByteStride = 0u;
    D3D11_SUBRESOURCE_DATA csd2;
    csd2.pSysMem = &cb2;
    wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
    GFX_THROW_INFO(pDevice->CreateBuffer(&constDesc2, &csd2, &pConstantBuffer2));
    GFX_THROW_INFO_ONLY(pDeviceContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf()));

    //Create pixel shader
    wrl::ComPtr<ID3D11PixelShader> pPixelShader;
    wrl::ComPtr<ID3DBlob> pBlob;
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

    //Bind pixel shader to context
    pDeviceContext->PSSetShader(pPixelShader.Get(), 0, 0);


    //Create vertex shader
    wrl::ComPtr<ID3D11VertexShader> pVertexShader;
    GFX_THROW_INFO_ONLY(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
    GFX_THROW_INFO_ONLY(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

    //Bind vertex shader to context
    pDeviceContext->VSSetShader(pVertexShader.Get(), 0, 0);

    //Set Input Layout
    wrl::ComPtr<ID3D11InputLayout> pInputLayout;
    //D3D11_APPEND_ALIGNED_ELEMENT is a way to let d3d calculate the offset from previous element
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    //Blob en este caso debe ser del vertex shader, debe hacer la comprobaci�n de si el layout coincide con el del shader
    GFX_THROW_INFO( pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));

    //Bind Input Layout to pipeline
    pDeviceContext->IASetInputLayout(pInputLayout.Get());

    //Set primitive topology to triangle
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Viewport
    D3D11_VIEWPORT viewport;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    pDeviceContext->RSSetViewports(1, &viewport);


    GFX_THROW_INFO_ONLY(pDeviceContext->DrawIndexed(std::size(indices), 0u, 0u));
}

//This method will be in charge of flipping (Taking the back buffer and presenting it as the front)
void D3DClass::EndScene()
{
    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set();
#endif 


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


