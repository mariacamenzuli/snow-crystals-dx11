#pragma once

#include <D3DX11tex.h>
#include <wrl/client.h>

class RenderTargetTextureCube {
public:
    RenderTargetTextureCube();
    ~RenderTargetTextureCube();

    void initialize(ID3D11Device* device, int cubeSize);
    ID3D11ShaderResourceView** getTextureResource();
    void setAsRenderTarget(ID3D11DeviceContext* deviceContext, int faceIndex);
    void clearRenderTarget(ID3D11DeviceContext* deviceContext, int faceIndex);
    void saveToFile(ID3D11DeviceContext* deviceContext);

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTextureCube;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetViews[6];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer[6];
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView[6];
    D3D11_VIEWPORT viewport;
};

