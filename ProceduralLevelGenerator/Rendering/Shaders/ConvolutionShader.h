#pragma once

#include <D3D11.h>
#include <wrl/client.h>
#include <D3DX10.h>

class ConvolutionShader {
public:
    ConvolutionShader();
    ~ConvolutionShader();

    float embossDenominator = 1.0f;
    float embossOffset = 0.5f;
    D3DXMATRIX embossKernelMatrix;

    float sharpnessDenominator = 1.0f;
    float sharpnessOffset = 0.0f;
    D3DXMATRIX sharpnessKernelMatrix;

    float edgeDetectionDenominator = 0.1f;
    float edgeDetectionOffset = 0.0f;
    D3DXMATRIX edgeDetectionKernelMatrix;

    float blurDenominator = 16.0f;
    float blurOffset = 0.0f;
    D3DXMATRIX blurKernelMatrix;

    void initialize(ID3D11Device* device);
    void setActive(ID3D11DeviceContext* deviceContext);
    void updateTexture(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** texture);
    void updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext,
                                            D3DXMATRIX worldMatrix,
                                            D3DXMATRIX viewMatrix,
                                            D3DXMATRIX projectionMatrix);
    void updateConvolutionBuffer(ID3D11DeviceContext* deviceContext,
                                 D3DXMATRIX kernelMatrix,
                                 float screenWidth,
                                 float screenHeight,
                                 float denominator,
                                 float offset);;

private:
    struct TransformationMatricesBuffer {
        D3DXMATRIX worldMatrix;
        D3DXMATRIX viewMatrix;
        D3DXMATRIX projectionMatrix;
    };
    
    struct ConvolutionBuffer {
        D3DXMATRIX kernelMatrix;
        float screenWidth;
        float screenHeight;
        float denominator;
        float offset;
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

