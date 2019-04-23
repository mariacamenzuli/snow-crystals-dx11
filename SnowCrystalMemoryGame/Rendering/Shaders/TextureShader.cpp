#include <stdexcept>

#include "TextureShader.h"
#include "../../Util/FileReader.h"

TextureShader::TextureShader() {
}

TextureShader::~TextureShader() {
}

void TextureShader::initialize(ID3D11Device* device) {
    setupVertexShader(device);
    setupPixelShader(device);
}

void TextureShader::setActive(ID3D11DeviceContext* deviceContext) {
    deviceContext->IASetInputLayout(inputLayout.Get());
    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void TextureShader::updateTexture(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** texture) {
    deviceContext->PSSetShaderResources(0, 1, texture);
    deviceContext->PSSetSamplers(0, 1, textureSampler.GetAddressOf());
}

void TextureShader::updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix) {
    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(transformationMatricesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the transformation matrices buffer for the texture shader input.");
    }

    auto transformationMatrixData = static_cast<TransformationMatricesBuffer*>(mappedResource.pData);

    transformationMatrixData->worldMatrix = worldMatrix;
    transformationMatrixData->viewMatrix = viewMatrix;
    transformationMatrixData->projectionMatrix = projectionMatrix;

    deviceContext->Unmap(transformationMatricesBuffer.Get(), 0);

    deviceContext->VSSetConstantBuffers(0, 1, transformationMatricesBuffer.GetAddressOf());
}

void TextureShader::setupVertexShader(ID3D11Device* device) {
    HRESULT result;

    auto textureVertexBuffer = FileReader::readBinaryFile("Resources/Shaders/texture-vertex.cso");

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(textureVertexBuffer.data(), textureVertexBuffer.size(), nullptr, vertexShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture shader. Creation of vertex shader failed.");
    }

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[2];

    inputLayoutDesc[0].SemanticName = "POSITION";
    inputLayoutDesc[0].SemanticIndex = 0;
    inputLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputLayoutDesc[0].InputSlot = 0;
    inputLayoutDesc[0].AlignedByteOffset = 0;
    inputLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputLayoutDesc[0].InstanceDataStepRate = 0;

    inputLayoutDesc[1].SemanticName = "TEXCOORD";
    inputLayoutDesc[1].SemanticIndex = 0;
    inputLayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputLayoutDesc[1].InputSlot = 0;
    inputLayoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    inputLayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputLayoutDesc[1].InstanceDataStepRate = 0;

    result = device->CreateInputLayout(inputLayoutDesc,
                                       sizeof inputLayoutDesc / sizeof inputLayoutDesc[0],
                                       textureVertexBuffer.data(),
                                       textureVertexBuffer.size(), inputLayout.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture shader. Creation of instanced input layout failed.");
    }

    D3D11_BUFFER_DESC transformationMatricesBufferDesc;
    transformationMatricesBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    transformationMatricesBufferDesc.ByteWidth = sizeof(TransformationMatricesBuffer);
    transformationMatricesBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    transformationMatricesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    transformationMatricesBufferDesc.MiscFlags = 0;
    transformationMatricesBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&transformationMatricesBufferDesc, nullptr, transformationMatricesBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture shader. Creation of transformation matrices constant buffer failed.");
    }
}

void TextureShader::setupPixelShader(ID3D11Device* device) {
    HRESULT result;

    auto lightPixelBuffer = FileReader::readBinaryFile("Resources/Shaders/texture-pixel.cso");

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(lightPixelBuffer.data(), lightPixelBuffer.size(), nullptr, pixelShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture shader. Creation of pixel shader failed.");
    }

    D3D11_SAMPLER_DESC textureSamplerDesc;
    textureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    textureSamplerDesc.MipLODBias = 0.0f;
    textureSamplerDesc.MaxAnisotropy = 1;
    textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    textureSamplerDesc.BorderColor[0] = 0;
    textureSamplerDesc.BorderColor[1] = 0;
    textureSamplerDesc.BorderColor[2] = 0;
    textureSamplerDesc.BorderColor[3] = 0;
    textureSamplerDesc.MinLOD = 0;
    textureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&textureSamplerDesc, textureSampler.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture shader. Creation of texture sample state failed.");
    }
}
