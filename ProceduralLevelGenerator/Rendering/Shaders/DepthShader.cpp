#include "DepthShader.h"
#include "../../Util/FileReader.h"

DepthShader::DepthShader() = default;

DepthShader::~DepthShader() = default;

void DepthShader::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
    setupVertexShader(device);
    setupPixelShader(device);
}

void DepthShader::setActive(ID3D11DeviceContext* deviceContext) {
    deviceContext->IASetInputLayout(layout.Get());
    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void DepthShader::updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext, D3DXMATRIX objectWorldMatrix, D3DXMATRIX pointLightViewMatrix, D3DXMATRIX pointLightProjectionMatrix) {
    D3DXMatrixTranspose(&objectWorldMatrix, &objectWorldMatrix);
    D3DXMatrixTranspose(&pointLightViewMatrix, &pointLightViewMatrix);
    D3DXMatrixTranspose(&pointLightProjectionMatrix, &pointLightProjectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = deviceContext->Map(transformationMatricesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the transformation matrices buffer for the depth shader input.");
    }
    
    auto transformationMatrixData = static_cast<TransformationMatricesBuffer*>(mappedResource.pData);

    transformationMatrixData->worldMatrix = objectWorldMatrix;
    transformationMatrixData->viewMatrix = pointLightViewMatrix;
    transformationMatrixData->projectionMatrix = pointLightProjectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(transformationMatricesBuffer.Get(), 0);

    // Finally set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(0, 1, transformationMatricesBuffer.GetAddressOf());
}

void DepthShader::setupVertexShader(ID3D11Device* device) {
    HRESULT result;

    auto depthVertexBuffer = FileReader::readBinaryFile("Resources/Shaders/depth-vertex.cso");

    result = device->CreateVertexShader(depthVertexBuffer.data(), depthVertexBuffer.size(), nullptr, vertexShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create depth shader. Creation of vertex shader failed.");
    }

    D3D11_INPUT_ELEMENT_DESC polygonLayout[1];

    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;
    
    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    result = device->CreateInputLayout(polygonLayout, numElements, depthVertexBuffer.data(),
                                       depthVertexBuffer.size(), layout.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create depth shader. Creation of input layout failed.");
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
        throw std::runtime_error("Failed to create depth shader. Creation of transformation matrices constant buffer failed.");
    }
}

void DepthShader::setupPixelShader(ID3D11Device* device) {
    auto depthPixelBuffer = FileReader::readBinaryFile("Resources/Shaders/depth-pixel.cso");

    HRESULT result = device->CreatePixelShader(depthPixelBuffer.data(), depthPixelBuffer.size(), nullptr, pixelShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create depth shader. Creation of pixel shader failed.");
    }
}
