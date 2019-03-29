#include "RenderTargetTexture.h"
#include <stdexcept>

RenderTargetTexture::RenderTargetTexture() {
}

RenderTargetTexture::~RenderTargetTexture() = default;

ID3D11ShaderResourceView** RenderTargetTexture::getTextureResource() {
    return shaderResourceView.GetAddressOf();
}

void RenderTargetTexture::initialize(ID3D11Device* device, int width, int height) {
    HRESULT result;

    D3D11_TEXTURE2D_DESC textureDesc;

    ZeroMemory(&textureDesc, sizeof(textureDesc));

    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    result = device->CreateTexture2D(&textureDesc, nullptr, renderTargetTexture.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture for render target texture cube.");
    }

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    result = device->CreateRenderTargetView(renderTargetTexture.Get(), &renderTargetViewDesc, renderTargetView.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create render target view for render target texture.");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    result = device->CreateShaderResourceView(renderTargetTexture.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create shader resource view for render target texture.");
    }
}

void RenderTargetTexture::setAsRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView) {
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView);
}

void RenderTargetTexture::clearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView) {
    float startingValues[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView.Get(), startingValues);
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
