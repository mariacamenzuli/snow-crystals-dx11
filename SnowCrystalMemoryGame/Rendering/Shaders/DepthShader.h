#pragma once

#include <D3D11.h>
#include <D3DX10math.h>
#include <wrl/client.h>

class DepthShader {
public:
    DepthShader();
    ~DepthShader();

    void initialize(ID3D11Device* device);
    void setActive(ID3D11DeviceContext* deviceContext);
    void updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext,
                                            D3DXMATRIX objectWorldMatrix,
                                            D3DXMATRIX pointLightViewMatrix,
                                            D3DXMATRIX pointLightProjectionMatrix,
                                            int isInstanced);

private:
    struct TransformationMatricesBuffer {
        D3DXMATRIX worldMatrix;
        D3DXMATRIX viewMatrix;
        D3DXMATRIX projectionMatrix;
        float isInstanced;
        float padding0;
        float padding1;
        float padding2;
    };
    
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> transformationMatricesBuffer;
    
    void setupVertexShader(ID3D11Device* device);
    void setupPixelShader(ID3D11Device* device);
};
