#include <stdexcept>
#include <D3DX11async.h>

#include "LightShader.h"
#include "../../Util/FileReader.h"

LightShader::LightShader() = default;

LightShader::~LightShader() = default;

void LightShader::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
    setupVertexShader(device);
    setupPixelShader(device);
}

void LightShader::setActive(ID3D11DeviceContext* deviceContext) {
    deviceContext->IASetInputLayout(layout.Get());
    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void LightShader::updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext,
                                                     D3DXMATRIX objectWorldMatrix,
                                                     D3DXMATRIX cameraViewMatrix,
                                                     D3DXMATRIX cameraProjectionMatrix) {
    // Transpose the matrices to prepare them for the shader.
    D3DXMatrixTranspose(&objectWorldMatrix, &objectWorldMatrix);
    D3DXMatrixTranspose(&cameraViewMatrix, &cameraViewMatrix);
    D3DXMatrixTranspose(&cameraProjectionMatrix, &cameraProjectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Lock the constant buffer so it can be written to.
    HRESULT result = deviceContext->Map(transformationMatricesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the transformation matrices buffer for the light shader input.");
    }

    // Get a pointer to the data in the constant buffer.
    auto transformationMatrixData = static_cast<TransformationMatricesBuffer*>(mappedResource.pData);

    // Copy the matrices into the constant buffer.
    transformationMatrixData->objectWorldMatrix = objectWorldMatrix;
    transformationMatrixData->cameraViewMatrix = cameraViewMatrix;
    transformationMatrixData->cameraProjectionMatrix = cameraProjectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(transformationMatricesBuffer.Get(), 0);

    // Finally set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(0, 1, transformationMatricesBuffer.GetAddressOf());
}

void LightShader::updateCameraBuffer(ID3D11DeviceContext* deviceContext, D3DXVECTOR3 cameraPosition) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the camera buffer for the light shader input.");
    }

    auto cameraData = static_cast<CameraBuffer*>(mappedResource.pData);
    cameraData->cameraPosition = D3DXVECTOR4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
    deviceContext->Unmap(cameraBuffer.Get(), 0);
    deviceContext->VSSetConstantBuffers(1, 1, cameraBuffer.GetAddressOf());
}

void LightShader::updateAmbientLightBuffer(ID3D11DeviceContext* deviceContext, D3DXVECTOR4 ambientLightColor) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(ambientLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the ambient light buffer for the light shader input.");
    }

    auto ambientLightData = static_cast<AmbientLightBuffer*>(mappedResource.pData);
    ambientLightData->sceneAmbientColor = ambientLightColor;
    deviceContext->Unmap(ambientLightBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, ambientLightBuffer.GetAddressOf());
}

void LightShader::updatePointLightBuffer(ID3D11DeviceContext* deviceContext, D3DXVECTOR4 diffuse, D3DXVECTOR4 specular, D3DXMATRIX worldMatrix) {
    auto origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR4 pointLightPosition;
    D3DXVec3Transform(&pointLightPosition, &origin, &worldMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(pointLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the point light buffer for the light shader input.");
    }

    auto pointLightData = static_cast<PointLightBuffer*>(mappedResource.pData);
    pointLightData->pointLightPosition = pointLightPosition;
    pointLightData->pointLightDiffuse = diffuse;
    pointLightData->pointLightSpecular = specular;
    deviceContext->Unmap(pointLightBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(1, 1, pointLightBuffer.GetAddressOf());
}

void LightShader::updateMaterialBuffer(ID3D11DeviceContext* deviceContext, D3DXVECTOR4 materialAmbientColor, D3DXVECTOR4 materialDiffuseColor, D3DXVECTOR4 materialSpecularColor, bool isTextured) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(materialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the material buffer for the light shader input.");
    }

    auto materialData = static_cast<MaterialBuffer*>(mappedResource.pData);
    materialData->materialAmbientColor = materialAmbientColor;
    materialData->materialDiffuseColor = materialDiffuseColor;
    materialData->materialSpecularColor = materialSpecularColor;
    if (isTextured) {
        materialData->materialIsTextured = 1;
    } else {
        materialData->materialIsTextured = -1;
    }
    deviceContext->Unmap(materialBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(2, 1, materialBuffer.GetAddressOf());
}

void LightShader::updateTexture(ID3D11DeviceContext* deviceContext, Texture* texture) {
    deviceContext->PSSetShaderResources(0, 1, texture->getTextureResource());
    deviceContext->PSSetSamplers(0, 1, materialTextureSampler.GetAddressOf());
}

void LightShader::updateDepthMapTexture(ID3D11DeviceContext* deviceContext, RenderTargetTextureCube* depthMapTexture) {
    deviceContext->PSSetShaderResources(1, 1, depthMapTexture->getTextureResource());
    deviceContext->PSSetSamplers(1, 1, depthMapTextureSampler.GetAddressOf());
}

void LightShader::setupVertexShader(ID3D11Device* device) {
    HRESULT result;

    auto lightVertexBuffer = FileReader::readBinaryFile("Resources/Shaders/light-vertex.cso");

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(lightVertexBuffer.data(), lightVertexBuffer.size(), nullptr, vertexShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of vertex shader failed.");
    }

    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];

    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // Get a count of the elements in the layout.
    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, lightVertexBuffer.data(),
                                       lightVertexBuffer.size(), layout.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of input layout failed.");
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
        throw std::runtime_error("Failed to create light shader. Creation of transformation matrices constant buffer failed.");
    }

    D3D11_BUFFER_DESC cameraBufferDesc;
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&cameraBufferDesc, nullptr, cameraBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of camera constant buffer failed.");
    }
}

void LightShader::setupPixelShader(ID3D11Device* device) {
    HRESULT result;

    auto lightPixelBuffer = FileReader::readBinaryFile("Resources/Shaders/light-pixel.cso");

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(lightPixelBuffer.data(), lightPixelBuffer.size(), nullptr, pixelShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of pixel shader failed.");
    }

    D3D11_BUFFER_DESC ambientLightBufferDesc;
    ambientLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ambientLightBufferDesc.ByteWidth = sizeof(AmbientLightBuffer);
    ambientLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ambientLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ambientLightBufferDesc.MiscFlags = 0;
    ambientLightBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&ambientLightBufferDesc, nullptr, ambientLightBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of ambient light constant buffer failed.");
    }

    D3D11_BUFFER_DESC pointLightBufferDesc;
    pointLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    pointLightBufferDesc.ByteWidth = sizeof(PointLightBuffer);
    pointLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pointLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pointLightBufferDesc.MiscFlags = 0;
    pointLightBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&pointLightBufferDesc, nullptr, pointLightBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of point light constant buffer failed.");
    }

    D3D11_BUFFER_DESC materialBufferDesc;
    materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    materialBufferDesc.ByteWidth = sizeof(MaterialBuffer);
    materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    materialBufferDesc.MiscFlags = 0;
    materialBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&materialBufferDesc, nullptr, materialBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of material constant buffer failed.");
    }

    D3D11_SAMPLER_DESC materialSamplerDesc;
    materialSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    materialSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    materialSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    materialSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    materialSamplerDesc.MipLODBias = 0.0f;
    materialSamplerDesc.MaxAnisotropy = 1;
    materialSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    materialSamplerDesc.BorderColor[0] = 0;
    materialSamplerDesc.BorderColor[1] = 0;
    materialSamplerDesc.BorderColor[2] = 0;
    materialSamplerDesc.BorderColor[3] = 0;
    materialSamplerDesc.MinLOD = 0;
    materialSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&materialSamplerDesc, materialTextureSampler.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of texture sample state failed.");
    }

    D3D11_SAMPLER_DESC depthMapSamplerDesc;
    depthMapSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    depthMapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    depthMapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    depthMapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    depthMapSamplerDesc.MipLODBias = 0.0f;
    depthMapSamplerDesc.MaxAnisotropy = 1;
    depthMapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthMapSamplerDesc.BorderColor[0] = 0;
    depthMapSamplerDesc.BorderColor[1] = 0;
    depthMapSamplerDesc.BorderColor[2] = 0;
    depthMapSamplerDesc.BorderColor[3] = 0;
    depthMapSamplerDesc.MinLOD = 0;
    depthMapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&depthMapSamplerDesc, depthMapTextureSampler.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create light shader. Creation of depth map sample state failed.");
    }
}
