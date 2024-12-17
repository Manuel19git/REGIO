#include "ShadowMap.h"

// Initialize here viewport, SRV, DSV
ShadowMap::ShadowMap(ID3D11Device* device, float width, float height) : width(width), height(height)
{
    HRESULT hr;

    // Viewport
    viewport.Width = width;
    viewport.Height = height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;


	// Create Depth Stencil Texture
    D3D11_TEXTURE2D_DESC depthTextureDesc = {};
    depthTextureDesc.Width = width; 
    depthTextureDesc.Height = height; 
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; //Typeless bcause SRV and DSV have different formats
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL |
                                 D3D11_BIND_SHADER_RESOURCE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthTexture;
    GFX_THROW_INFO(device->CreateTexture2D(&depthTextureDesc, nullptr, &pDepthTexture));

    // Create depth stencil view of depth texture
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    GFX_THROW_INFO(device->CreateDepthStencilView(pDepthTexture.Get(), &depthStencilViewDesc, &pDepthStencilView));

    // Create shader resource view of depth texture
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = depthTextureDesc.MipLevels;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    GFX_THROW_INFO(device->CreateShaderResourceView(pDepthTexture.Get(), &shaderResourceViewDesc, &pShaderResourceView));
}

void ShadowMap::BindDSVandNullTarget(ID3D11DeviceContext* deviceContext)
{
    deviceContext->RSSetViewports(1, &viewport);

    // We set pTargets to null because we are not going to render anything. We are only going to write to the depth buffer texture
    ID3D11RenderTargetView* pTargets[1] = {0};
    GFX_THROW_INFO_ONLY(deviceContext->OMSetRenderTargets(1, pTargets, pDepthStencilView.Get()));

    deviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ShadowMap::~ShadowMap()
{
    if (pDepthStencilView != nullptr)
    {
        pDepthStencilView->Release();
    }
    if (pShaderResourceView != nullptr)
    {
        pShaderResourceView->Release();
    }
}


