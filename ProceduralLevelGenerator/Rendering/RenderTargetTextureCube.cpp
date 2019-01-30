#include "RenderTargetTextureCube.h"

#include <stdexcept>

RenderTargetTextureCube::RenderTargetTextureCube() = default;

RenderTargetTextureCube::~RenderTargetTextureCube() = default;

void RenderTargetTextureCube::initialize(ID3D11Device* device, int cubeSize) {
    HRESULT result;

    D3D11_TEXTURE2D_DESC textureDesc;

    ZeroMemory(&textureDesc, sizeof(textureDesc));

    textureDesc.Width = cubeSize;
    textureDesc.Height = cubeSize;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    result = device->CreateTexture2D(&textureDesc, nullptr, renderTargetTextureCube.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture for render target texture.");
    }

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    renderTargetViewDesc.Texture2DArray.MipSlice = 0;
    renderTargetViewDesc.Texture2DArray.ArraySize = 1;

    for (int i = 0; i < 6; ++i) {
        renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
        result = device->CreateRenderTargetView(renderTargetTextureCube.Get(), &renderTargetViewDesc, renderTargetViews[i].GetAddressOf());
        if (FAILED(result)) {
            throw std::runtime_error("Failed to create render target view for render target texture.");
        }
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2DArray.MipLevels = 1;

    result = device->CreateShaderResourceView(renderTargetTextureCube.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create shader resource view for render target texture.");
    }

    for (int i = 0; i < 6; ++i) {
        D3D11_TEXTURE2D_DESC depthBufferDesc;
        ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
        depthBufferDesc.Width = cubeSize;
        depthBufferDesc.Height = cubeSize;
        depthBufferDesc.MipLevels = 1;
        depthBufferDesc.ArraySize = 1;
        depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDesc.SampleDesc.Count = 1;
        depthBufferDesc.SampleDesc.Quality = 0;
        depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthBufferDesc.CPUAccessFlags = 0;
        depthBufferDesc.MiscFlags = 0;

        result = device->CreateTexture2D(&depthBufferDesc, nullptr, depthStencilBuffer[i].GetAddressOf());
        if (FAILED(result)) {
            throw std::runtime_error("Failed to create depth stencil buffer for render target texture.");
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        // Create the depth stencil view.
        result = device->CreateDepthStencilView(depthStencilBuffer[i].Get(), &depthStencilViewDesc, depthStencilView[i].GetAddressOf());
        if (FAILED(result)) {
            throw std::runtime_error("Failed to create depth stencil view for render target texture.");
        }
    }

    // Setup the viewport for rendering.
    viewport.Width = static_cast<float>(cubeSize);
    viewport.Height = static_cast<float>(cubeSize);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
}

ID3D11ShaderResourceView** RenderTargetTextureCube::getTextureResource() {
    return shaderResourceView.GetAddressOf();
}

void RenderTargetTextureCube::setAsRenderTarget(ID3D11DeviceContext* deviceContext, int faceIndex) {
    deviceContext->OMSetRenderTargets(1, renderTargetViews[faceIndex].GetAddressOf(), depthStencilView[faceIndex].Get());
    deviceContext->RSSetViewports(1, &viewport);
}

void RenderTargetTextureCube::clearRenderTarget(ID3D11DeviceContext* deviceContext, int faceIndex) {
    float startingValues[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetViews[faceIndex].Get(), startingValues);
    deviceContext->ClearDepthStencilView(depthStencilView[faceIndex].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderTargetTextureCube::saveToFile(ID3D11DeviceContext* deviceContext) {
    HRESULT result = D3DX11SaveTextureToFile(deviceContext, renderTargetTextureCube.Get(), D3DX11_IFF_DDS, L"shadow-map.dds");
}
