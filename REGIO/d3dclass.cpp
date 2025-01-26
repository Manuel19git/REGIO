#include <iostream>
#include "d3dclass.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


D3DClass::D3DClass()
{}


void DirectXError(HRESULT hr, const std::string& Msg, const std::string& File, int Line)
{
}

// Build the vertex and index buffer in an efficient way (every object vertex is in the same buffer)
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
    UINT* indices = new UINT[totalIndexCount];
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

            if (mesh->HasTextureCoords(0)) // Check if it has at least one texture coord
            {
                vertices[pVertexOffsets[meshId] + vertexId].tex.u = mesh->mTextureCoords[0][vertexId].x;
                vertices[pVertexOffsets[meshId] + vertexId].tex.v = mesh->mTextureCoords[0][vertexId].y;
            }
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
    indexDesc.ByteWidth = sizeof(UINT) * totalIndexCount;
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = sizeof(UINT);
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
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    pTechniqueLight->GetPassByIndex(0)->GetDesc(&passDesc);
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayout));
}

void D3DClass::BuildTextures(const aiScene* pScene)
{
    HRESULT hr;

    pTextures.resize(pScene->mNumMaterials);
    for (int i = 0; i < pScene->mNumMaterials; ++i)
	{
        aiString texturePath;
        pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
        
        if (texturePath.length != 0)
        {
            // Convert the string to the right type before feeding it to the following function
            const size_t pathSize = strlen(texturePath.C_Str()) + 1;
            wchar_t* pathWideString = new wchar_t[pathSize];
            size_t retVal;
            mbstowcs_s(&retVal, pathWideString, pathSize, texturePath.C_Str(), pathSize - 1);

            GFX_THROW_INFO(CreateWICTextureFromFile(
                pDevice.Get(),
                pathWideString,
                nullptr,
                pTextures[i].GetAddressOf()));
        }
    }
}

void D3DClass::BuildSkymap()
{
    HRESULT hr;

    // CreateDDSTextureFromFile sets automatically description and its properties. 
    // From the DDS file it should read metadata and wether it is a cubemap or not 
	GFX_THROW_INFO(CreateDDSTextureFromFile(
		pDevice.Get(),
		L"..\\output\\NIER\\Props\\textures\\otro_cielo.dds",
		nullptr,
		cubemapTexture.GetAddressOf()));
}

bool D3DClass::Initialize(HWND hWnd, const aiScene* pScene, Camera* mainCamera)
{
    // Camera
    camera = mainCamera;
    screenWidth = camera->getResolution().first;
    screenHeight = camera->getResolution().second;

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

    pShadowMap = new ShadowMap(pDevice.Get(), screenWidth, screenHeight);

    //Create rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
    GFX_THROW_INFO(pDevice->CreateRasterizerState(&rasterizerDesc, &pNoCullRS));

    //Create Stencil Buffer
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE; 
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthBuffer));
    
    //Create Depth Stencil Texture
    D3D11_TEXTURE2D_DESC depthTextureDesc = {};
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

    //Build Geometry
    BuildGeometry(pScene);

    //Build Effects
    GFX_THROW_INFO(D3DX11CreateEffectFromFile(L"../build/effects/LightEffect.fxo", 0, pDevice.Get(), pEffect.GetAddressOf()));
    pTechniqueLight = pEffect->GetTechniqueByName("LighTech");
    pTechniqueLightTex = pEffect->GetTechniqueByName("LighTechTex");
    pTechniqueDebug = pEffect->GetTechniqueByName("DebugTexture");
    pTechniqueSky = pEffect->GetTechniqueByName("Sky");
    pTechniqueShadow = pEffect->GetTechniqueByName("ShadowMap");


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


    //Create textures
    BuildTextures(pScene);
    shaderResource = pEffect->GetVariableByName("textureObject")->AsShaderResource();

    //Create skymap
    BuildSkymap();
    pEffect->GetVariableByName("textureCubemap")->AsShaderResource()->SetResource(cubemapTexture.Get());

    //Create light and material
    float down = 10.0f;
    dirLight.Ambient    = XMFLOAT4(0.6156f/down, 0.3568f/down, 0.6392f/down, 1.0f);
    dirLight.Diffuse    = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.Specular   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.Direction  = XMFLOAT3(0.57735f, -0.57735f, -0.57735f);
    dirLight.Intensity  = 1.0f;

    spotLight.Ambient   = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    spotLight.Diffuse   = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
    spotLight.Specular  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    spotLight.Att       = XMFLOAT3(1.0f, 0.8f, 0.0f);
    spotLight.Range     = 1000.0f;
    spotLight.Spot      = 40.0f;
    spotLight.Intensity = 0.0f;
    
    for (int i = 0; i < 6; ++i)
    {
        pointLights[i].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        pointLights[i].Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        pointLights[i].Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        pointLights[i].Att = XMFLOAT3(1.0f, 0.1f, 0.01f);
        pointLights[i].Range = 1000.0f;
        pointLights[i].Intensity = 1.0f;
    }
    pointLights[0].Position = XMFLOAT3(-32.2553, 1.85, -38.189022);
    pointLights[1].Position = XMFLOAT3(-32.2553, 1.85, -81.722275);
    pointLights[2].Position = XMFLOAT3(-32.2553, 1.85, -113.461594);
    pointLights[3].Position = XMFLOAT3(32.2553, 1.85, -38.189022);
    pointLights[4].Position = XMFLOAT3(32.2553, 1.85, -81.722275);
    pointLights[5].Position = XMFLOAT3(32.2553, 1.85, -113.461594);

    material.Ambient    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Diffuse    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Specular   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

    fxDirLight      = pEffect->GetVariableByName("gDirLight");
    fxPointLight    = pEffect->GetVariableByName("gPointLight");
    fxSpotLight     = pEffect->GetVariableByName("gSpotLight");
    fxEyePos        = pEffect->GetVariableByName("gEyePosW");
    fxTransform     = pEffect->GetVariableByName("gTransform");
    fxTransformSkybox = pEffect->GetVariableByName("gTransformSkybox");
    fxTransformSun  = pEffect->GetVariableByName("gTransformSun");
    fxMaterial      = pEffect->GetVariableByName("gMaterial");
    fxPointLights   = pEffect->GetVariableByName("gPointLights");

    fxPointLights->SetRawValue(&pointLights, 0, sizeof(PointLight) * 6);

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

// I may not need it
void D3DClass::DrawShadowMap(const aiScene* scene, Camera* sunCamera)
{
    HRESULT hr;

    //Bind Vertex Layout and Primitive Topology
    pDeviceContext->IASetInputLayout(pInputLayout.Get());
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Bind Vertex and Index buffer
    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0));


}

void D3DClass::DrawScene(const aiScene* scene, Camera* camera)
{
    HRESULT hr;

    //Bind Vertex Layout and Primitive Topology
    pDeviceContext->IASetInputLayout(pInputLayout.Get());
    pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Bind Vertex and Index buffer
    const UINT strides = sizeof(Vertex);
    const UINT offset = 0;
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &strides, &offset));
    GFX_THROW_INFO_ONLY(pDeviceContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0));

    //Set constant buffers
    XMFLOAT3 eyePos = sunCamera->getPosition();
    DirectX::XMMATRIX transformation = sunCamera->getTransform(true);
    DirectX::XMMATRIX transformationSun = sunCamera->getTransform(true); // true means orthographic camera

    spotLight.Position = eyePos;
    pointLight.Position = eyePos;
    XMStoreFloat3(&spotLight.Direction, XMVector3Normalize(camera->getLookAt()));
    fxSpotLight->SetRawValue(&spotLight, 0, sizeof(SpotLight));
    fxPointLight->SetRawValue(&pointLight, 0, sizeof(PointLight));
    fxDirLight->SetRawValue(&dirLight, 0, sizeof(DirectionalLight));
    fxEyePos->SetRawValue(&eyePos, 0, sizeof(XMFLOAT3));
    fxTransform->SetRawValue(&transformation, 0, sizeof(XMMATRIX));
    fxTransformSun->SetRawValue(&transformationSun, 0, sizeof(Material));
    fxMaterial->SetRawValue(&material, 0, sizeof(Material));

    //Viewport
    D3D11_VIEWPORT viewport;
    viewport.Width = screenWidth;
    viewport.Height = screenHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    pDeviceContext->RSSetViewports(1, &viewport);

    //Rasterizer State
    //pDeviceContext->RSSetState(pNoCullRS.Get());

    // Shadow map pass
	pShadowMap->BindDSVandNullTarget(pDeviceContext.Get());
	pTechniqueLight->GetPassByIndex(1)->Apply(0, pDeviceContext.Get()); // 1 is the shadow map pass fix in the future to something more readable
    for (int meshId = 0; meshId < scene->mNumMeshes; ++meshId)
    {
		// Draw Scene
		pDeviceContext->DrawIndexed(pIndexCount[meshId], pIndexOffsets[meshId], pVertexOffsets[meshId]);
    }
	// Restore
	pDeviceContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());


    //Set constant buffers
    eyePos = camera->getPosition();
    transformation = camera->getTransform();
    transformationSun = sunCamera->getTransform(true); // true means orthographic camera

    spotLight.Position = eyePos;
    pointLight.Position = eyePos;
    XMStoreFloat3(&spotLight.Direction, XMVector3Normalize(camera->getLookAt()));
    fxSpotLight->SetRawValue(&spotLight, 0, sizeof(SpotLight));
    fxPointLight->SetRawValue(&pointLight, 0, sizeof(PointLight));
    fxDirLight->SetRawValue(&dirLight, 0, sizeof(DirectionalLight));
    fxEyePos->SetRawValue(&eyePos, 0, sizeof(XMFLOAT3));
    fxTransform->SetRawValue(&transformation, 0, sizeof(XMMATRIX));
    fxTransformSun->SetRawValue(&transformationSun, 0, sizeof(Material));
    fxMaterial->SetRawValue(&material, 0, sizeof(Material));

	for (int meshId = 0; meshId < scene->mNumMeshes; ++meshId)
	{
        if (pTextures[scene->mMeshes[meshId]->mMaterialIndex] != nullptr)
        {
            D3DX11_TECHNIQUE_DESC techDesc;
            pTechniqueLightTex->GetDesc(&techDesc);
            for (UINT32 p = 0; p < techDesc.Passes; ++p)
            {
				aiString name = scene->mMaterials[scene->mMeshes[meshId]->mMaterialIndex]->GetName();
				scene->mMeshes[meshId]->mName;
				// Gotta test this
				shaderResource->SetResource(pTextures[scene->mMeshes[meshId]->mMaterialIndex].Get());

				pTechniqueLightTex->GetPassByIndex(p)->Apply(0, pDeviceContext.Get());
				pDeviceContext->DrawIndexed(pIndexCount[meshId], pIndexOffsets[meshId], pVertexOffsets[meshId]);
			}
        }
        else
        {
			D3DX11_TECHNIQUE_DESC techDesc;
			pTechniqueLight->GetDesc(&techDesc);
			for (UINT32 p = 0; p < techDesc.Passes; ++p)
			{
                if (p == 0)
                {
     //               pShadowMap->BindDSVandNullTarget(pDeviceContext.Get());

     //               // Draw Scene
					//pTechniqueLightTex->GetPassByIndex(p)->Apply(0, pDeviceContext.Get());
					//pDeviceContext->DrawIndexed(pIndexCount[meshId], pIndexOffsets[meshId], pVertexOffsets[meshId]);

     //               // Restore
     //               pDeviceContext->RSSetState(0);
				 //   pDeviceContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());
     //               pDeviceContext->RSSetViewports(1, &viewport); // Right now this doesn't make sense because shadow map viewport is the same as this one

                }
				else
                {
                    pTechniqueLight->GetPassByIndex(p)->Apply(0, pDeviceContext.Get());
                    pDeviceContext->DrawIndexed(pIndexCount[meshId], pIndexOffsets[meshId], pVertexOffsets[meshId]);
                }

			}
        }
	}
}

//Method to draw the sky
void D3DClass::DrawSky(const aiScene* scene, Camera* camera)
{
	HRESULT hr;

    float scale = 1000.0f;
	struct Vertex {
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	// Cube vertices data
	Vertex vertices[] = {
		// Back Face
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		// Front Face
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		// Left Face
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

		// Right Face
		{ XMFLOAT3(1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

		// Top Face
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f * scale,  1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },


		// Bottom Face
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f * scale, -1.0f * scale,  1.0f * scale), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	//Set constant buffers
    DirectX::XMMATRIX viewMatrix = camera->getViewMatrix();
	viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // Remove translation part of matrix
    DirectX::XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    DirectX::XMMATRIX transformSkybox = DirectX::XMMatrixTranspose(viewMatrix * projectionMatrix);
    fxTransformSkybox->SetRawValue(&transformSkybox, 0, sizeof(XMMATRIX));

	unsigned int indices[] = {
		// Back Face
		0, 1, 2,
		0, 2, 3,

		// Front Face
		4, 6, 5,
		4, 7, 6,

		// Left Face
		8, 9, 10,
		8, 10, 11,

		// Right Face
		12, 14, 13,
		12, 15, 14,

		// Top Face
		16, 17, 18,
		16, 18, 19,

		// Bottom Face
		20, 22, 21,
		20, 23, 22
	};

	// Vertex buffer description
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices;

	// Create the vertex buffer
	ID3D11Buffer* vertexBuffer;
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Index buffer description
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    // Create the index buffer
    ID3D11Buffer* indexBuffer;
    pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    // Set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Set the index buffer
    pDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);


    // Set the primitive topology (triangle list)
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the input layout (assuming you have created an input layout for the shader)
    wrl::ComPtr<ID3D11InputLayout> pInputLayoutSky;
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    pTechniqueSky->GetPassByIndex(0)->GetDesc(&passDesc);
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayoutSky));

    pDeviceContext->IASetInputLayout(pInputLayoutSky.Get());

	// Draw the quad
	pTechniqueSky->GetPassByIndex(0)->Apply(0, pDeviceContext.Get());
    pDeviceContext->DrawIndexed(36, 0, 0);
}


//Method to draw anything to debug
void D3DClass::DrawDebug(const aiScene* scene, Camera* camera)
{
	HRESULT hr;

    //// Vertex structure
    //struct Vertex
    //{
    //    DirectX::XMFLOAT3 position;
    //    DirectX::XMFLOAT4 color;
    //};
    struct TexVertex
    {
		DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 uv;
    };


    float nearPlane = camera->getNear();
    float farPlane = camera->getFar() - 10.0f;

    float cameraZ = camera->getPosition().z;
    float scale = 1.0f;
    float distance = 1.5f;
	//// Assuming you have the camera's position and direction, and far plane distance
	//DirectX::XMFLOAT3 cameraPosition = camera->getPosition(); // Camera position in world space
 //   DirectX::XMFLOAT3 cameraForward;
 //   XMStoreFloat3(&cameraForward, camera->getForward());   // Camera forward direction (view direction)
 //   DirectX::XMFLOAT3 cameraRight;
 //   XMStoreFloat3(&cameraRight, camera->getRight());       // Camera right direction (strafe)
 //   DirectX::XMFLOAT3 cameraUp;
 //   XMStoreFloat3(&cameraUp, camera->getUp());             // Camera up direction

	//float farPlaneDistance = camera->getFar() - 10.0f; // Distance to the far plane

	//// Define the scale for the quad size (how large the far plane should appear)
	//float quadScale = 10.0f; // Adjust based on how large you want the quad to be

	//// Calculate the center position of the far plane
	//DirectX::XMFLOAT3 farPlaneCenter =
	//{
	//	cameraPosition.x + cameraForward.x * farPlaneDistance,
	//	cameraPosition.y + cameraForward.y * farPlaneDistance,
	//	cameraPosition.z + cameraForward.z * farPlaneDistance
	//};

	//// Define the vertices for the quad, oriented relative to the camera's right and up directions
	//Vertex vertices[] =
	//{
	//	// Top-left
	//	{ DirectX::XMFLOAT3(farPlaneCenter.x - cameraRight.x * quadScale + cameraUp.x * quadScale,
	//						farPlaneCenter.y - cameraRight.y * quadScale + cameraUp.y * quadScale,
	//						farPlaneCenter.z - cameraRight.z * quadScale + cameraUp.z * quadScale),
	//						DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // Red color

	//				// Bottom-left
	//				{ DirectX::XMFLOAT3(farPlaneCenter.x - cameraRight.x * quadScale - cameraUp.x * quadScale,
	//									farPlaneCenter.y - cameraRight.y * quadScale - cameraUp.y * quadScale,
	//									farPlaneCenter.z - cameraRight.z * quadScale - cameraUp.z * quadScale),
	//									DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // Red color

	//										// Bottom-right
	//										{ DirectX::XMFLOAT3(farPlaneCenter.x + cameraRight.x * quadScale - cameraUp.x * quadScale,
	//															farPlaneCenter.y + cameraRight.y * quadScale - cameraUp.y * quadScale,
	//															farPlaneCenter.z + cameraRight.z * quadScale - cameraUp.z * quadScale),
	//															DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // Red color

	//															// Top-right
	//															{ DirectX::XMFLOAT3(farPlaneCenter.x + cameraRight.x * quadScale + cameraUp.x * quadScale,
	//																				farPlaneCenter.y + cameraRight.y * quadScale + cameraUp.y * quadScale,
	//																				farPlaneCenter.z + cameraRight.z * quadScale + cameraUp.z * quadScale),
	//																				DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // Red color
	//};
    float factor = screenWidth / screenHeight;
    TexVertex quadVertices[] =
    {
        // Bottom-left corner of the screen (in NDC space) with correct texture coordinates
        { DirectX::XMFLOAT3(-1.4f, -1.0f, distance), DirectX::XMFLOAT2(0.0f, 1.0f) },  // Bottom-left
        { DirectX::XMFLOAT3(-0.9f, -1.0f, distance), DirectX::XMFLOAT2(1.0f, 1.0f) },  // Bottom-right
        { DirectX::XMFLOAT3(-1.4f, -0.5f, distance), DirectX::XMFLOAT2(0.0f, 0.0f) },  // Top-left
        { DirectX::XMFLOAT3(-0.9f, -0.5f, distance), DirectX::XMFLOAT2(1.0f, 0.0f) },  // Top-right
    };

	unsigned int indices[] =
	{
		0, 2, 1,  // First triangle (top-left, bottom-left, bottom-right)
		1, 2, 3   // Second triangle (top-left, bottom-right, top-right)
	};

	//unsigned int indices[] =

	//{
	//	0, 1, 2,  // First triangle (top-left, bottom-left, bottom-right)
	//	0, 2, 3   // Second triangle (top-left, bottom-right, top-right)
	//};
    //Create Stencil Buffer
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> noDepthBuffer;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE; 
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&depthStencilDesc, &noDepthBuffer));

	// Vertex buffer description
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(quadVertices) ;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = quadVertices;

	// Create the vertex buffer
	ID3D11Buffer* vertexBuffer;
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Index buffer description
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    // Create the index buffer
    ID3D11Buffer* indexBuffer;
    pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    // Set the vertex buffer
    UINT stride = sizeof(TexVertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Set the index buffer
    pDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Set constant buffers
    DirectX::XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    DirectX::XMMATRIX transformSkybox = DirectX::XMMatrixTranspose(projectionMatrix);
    fxTransformSkybox->SetRawValue(&transformSkybox, 0, sizeof(XMMATRIX));

    // Set the input layout (assuming you have created an input layout for the shader)
    wrl::ComPtr<ID3D11InputLayout> pInputLayoutSimple;
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    pEffect->GetVariableByName("shadowMap")->AsShaderResource()->SetResource(pShadowMap->pShaderResourceView.Get());
    //pEffect->GetVariableByName("shadowMap")->AsShaderResource()->SetResource(pShadowMap->pSunShaderResourceView.Get());
    pTechniqueDebug->GetPassByIndex(0)->GetDesc(&passDesc);
    GFX_THROW_INFO(pDevice->CreateInputLayout(inputLayoutDesc, std::size(inputLayoutDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayoutSimple));

    pDeviceContext->IASetInputLayout(pInputLayoutSimple.Get());

    // I put this here because its the last draw call, meaning that everything will be set as it was in the EndScene call
    GFX_THROW_INFO_ONLY(pDeviceContext->OMSetDepthStencilState(noDepthBuffer.Get(), 1u)); // Don't take into account this quad in the depth texture

	// Draw the quad
	pTechniqueDebug->GetPassByIndex(0)->Apply(0, pDeviceContext.Get());
    pDeviceContext->DrawIndexed(6, 0, 0);
    
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
