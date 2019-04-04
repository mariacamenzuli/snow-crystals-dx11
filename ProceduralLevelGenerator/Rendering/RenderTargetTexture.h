#pragma once

#include <D3DX11tex.h>
#include <wrl/client.h>

class RenderTargetTexture {
public:
    RenderTargetTexture();
    ~RenderTargetTexture();

    ID3D11ShaderResourceView** getTextureResource();
    void initialize(ID3D11Device* device, int width, int height);
    void setAsRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
    void clearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
    void saveToFile(ID3D11DeviceContext* deviceContext);

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    D3D11_VIEWPORT viewport;
};

