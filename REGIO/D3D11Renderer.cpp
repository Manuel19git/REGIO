#include "D3D11Renderer.h"

D3D11Renderer::D3D11Renderer()
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
void D3D11Renderer::EndScene()
{

}


void D3D11Renderer::DrawSky()
{

}
bool D3D11Renderer::ClearBuffer()
{
    return false;
}

