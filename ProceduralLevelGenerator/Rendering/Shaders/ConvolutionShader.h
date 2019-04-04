#pragma once

#include <D3D11.h>
#include <wrl/client.h>
#include <D3DX10.h>

class ConvolutionShader {
public:
    ConvolutionShader();
    ~ConvolutionShader();

    void initialize(ID3D11Device* device);
    void setActive(ID3D11DeviceContext* deviceContext);
    void updateTexture(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** texture);
    void updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext,
                                            D3DXMATRIX worldMatrix,
                                            D3DXMATRIX viewMatrix,
                                            D3DXMATRIX projectionMatrix);
    void updateScreenSizeBuffer(ID3D11DeviceContext* deviceContext,
                                float screenWidth,
                                float screenHeight);

private:
    struct TransformationMatricesBuffer {
        D3DXMATRIX worldMatrix;
        D3DXMATRIX viewMatrix;
        D3DXMATRIX projectionMatrix;
    };
    
    struct ScreenSizeBuffer {
        float screenWidth;
        float screenHeight;
        float padding0;
        float padding1;
    };

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> transformationMatricesBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> screenSizeBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSampler;

    void setupVertexShader(ID3D11Device* device);
    void setupPixelShader(ID3D11Device* device);

};

