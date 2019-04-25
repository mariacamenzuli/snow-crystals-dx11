#include "ConvolutionShader.h"
#include "../../Util/FileReader.h"

ConvolutionShader::ConvolutionShader() {
    // EMBOSS
    embossKernelMatrix._11 = 2.0f;
    embossKernelMatrix._12 = 0.0f;
    embossKernelMatrix._13 = 0.0f;
    embossKernelMatrix._14 = 0.0f;

    embossKernelMatrix._21 = 0.0f;
    embossKernelMatrix._22 = -1.0f;
    embossKernelMatrix._23 = 0.0f;
    embossKernelMatrix._24 = 0.0f;

    embossKernelMatrix._31 = 0.0f;
    embossKernelMatrix._32 = 0.0f;
    embossKernelMatrix._33 = -1.0f;
    embossKernelMatrix._34 = 0.0f;

    embossKernelMatrix._41 = 0.0f;
    embossKernelMatrix._42 = 0.0f;
    embossKernelMatrix._43 = 0.0f;
    embossKernelMatrix._44 = 1.0f; // indicate that image should be decolorized

    // BLUR
    blurKernelMatrix._11 = 1.0f;
    blurKernelMatrix._12 = 2.0f;
    blurKernelMatrix._13 = 1.0f;
    blurKernelMatrix._14 = 0.0f;

    blurKernelMatrix._21 = 2.0f;
    blurKernelMatrix._22 = 4.0f;
    blurKernelMatrix._23 = 2.0f;
    blurKernelMatrix._24 = 0.0f;

    blurKernelMatrix._31 = 1.0f;
    blurKernelMatrix._32 = 2.0f;
    blurKernelMatrix._33 = 1.0f;
    blurKernelMatrix._34 = 0.0f;

    blurKernelMatrix._41 = 0.0f;
    blurKernelMatrix._42 = 0.0f;
    blurKernelMatrix._43 = 0.0f;
    blurKernelMatrix._44 = 0.0f; // indicate that image should not be decolorized

    // SHARPNESS
    sharpnessKernelMatrix._11 = -1.0f;
    sharpnessKernelMatrix._12 = -1.0f;
    sharpnessKernelMatrix._13 = -1.0f;
    sharpnessKernelMatrix._14 = 0.0f;

    sharpnessKernelMatrix._21 = -1.0f;
    sharpnessKernelMatrix._22 = 9.0f;
    sharpnessKernelMatrix._23 = -1.0f;
    sharpnessKernelMatrix._24 = 0.0f;

    sharpnessKernelMatrix._31 = -1.0f;
    sharpnessKernelMatrix._32 = -1.0f;
    sharpnessKernelMatrix._33 = -1.0f;
    sharpnessKernelMatrix._34 = 0.0f;

    sharpnessKernelMatrix._41 = 0.0f;
    sharpnessKernelMatrix._42 = 0.0f;
    sharpnessKernelMatrix._43 = 0.0f;
    sharpnessKernelMatrix._44 = 0.0f;

    // EDGE DETECTION
    edgeDetectionKernelMatrix._11 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._12 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._13 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._14 = 0.0f;

    edgeDetectionKernelMatrix._21 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._22 = 1.0f;
    edgeDetectionKernelMatrix._23 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._24 = 0.0f;

    edgeDetectionKernelMatrix._31 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._32 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._33 = -1.0f / 8.0f;
    edgeDetectionKernelMatrix._34 = 0.0f;

    edgeDetectionKernelMatrix._41 = 0.0f;
    edgeDetectionKernelMatrix._42 = 0.0f;
    edgeDetectionKernelMatrix._43 = 0.0f;
    edgeDetectionKernelMatrix._44 = 1.0f;
}

ConvolutionShader::~ConvolutionShader() {
}

void ConvolutionShader::initialize(ID3D11Device* device) {
    setupVertexShader(device);
    setupPixelShader(device);
}

void ConvolutionShader::setActive(ID3D11DeviceContext* deviceContext) {
    deviceContext->IASetInputLayout(inputLayout.Get());
    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void ConvolutionShader::updateTexture(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** texture) {
    deviceContext->PSSetShaderResources(0, 1, texture);
    deviceContext->PSSetSamplers(0, 1, textureSampler.GetAddressOf());
}

void ConvolutionShader::updateTransformationMatricesBuffer(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix) {
    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(transformationMatricesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the transformation matrices buffer for the convolution shader input.");
    }

    auto transformationMatrixData = static_cast<TransformationMatricesBuffer*>(mappedResource.pData);

    transformationMatrixData->worldMatrix = worldMatrix;
    transformationMatrixData->viewMatrix = viewMatrix;
    transformationMatrixData->projectionMatrix = projectionMatrix;

    deviceContext->Unmap(transformationMatricesBuffer.Get(), 0);

    deviceContext->VSSetConstantBuffers(0, 1, transformationMatricesBuffer.GetAddressOf());
}

void ConvolutionShader::updateConvolutionBuffer(ID3D11DeviceContext* deviceContext,
                                                D3DXMATRIX kernelMatrix,
                                                float screenWidth,
                                                float screenHeight,
                                                float denominator,
                                                float offset) {
    D3DXMatrixTranspose(&kernelMatrix, &kernelMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT result = deviceContext->Map(screenSizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to lock the screen size buffer for the convolution shader input.");
    }

    auto convolutionData = static_cast<ConvolutionBuffer*>(mappedResource.pData);
    convolutionData->kernelMatrix = kernelMatrix;
    convolutionData->screenWidth = screenWidth;
    convolutionData->screenHeight = screenHeight;
    convolutionData->denominator = denominator;
    convolutionData->offset = offset;

    deviceContext->Unmap(screenSizeBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, screenSizeBuffer.GetAddressOf());
}

void ConvolutionShader::setupVertexShader(ID3D11Device* device) {
    HRESULT result;

    auto textureVertexBuffer = FileReader::readBinaryFile("Resources/Shaders/convolution-vertex.cso");

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(textureVertexBuffer.data(), textureVertexBuffer.size(), nullptr, vertexShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create convolution shader. Creation of vertex shader failed.");
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
        throw std::runtime_error("Failed to create convolution shader. Creation of instanced input layout failed.");
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
        throw std::runtime_error("Failed to create convolution shader. Creation of transformation matrices constant buffer failed.");
    }
}

void ConvolutionShader::setupPixelShader(ID3D11Device* device) {
    HRESULT result;

    auto lightPixelBuffer = FileReader::readBinaryFile("Resources/Shaders/convolution-pixel.cso");

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(lightPixelBuffer.data(), lightPixelBuffer.size(), nullptr, pixelShader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create convolution shader. Creation of pixel shader failed.");
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
        throw std::runtime_error("Failed to create convolution shader. Creation of texture sample state failed.");
    }

    D3D11_BUFFER_DESC screenSizeBufferDesc;
    screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    screenSizeBufferDesc.ByteWidth = sizeof(ConvolutionBuffer);
    screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    screenSizeBufferDesc.MiscFlags = 0;
    screenSizeBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&screenSizeBufferDesc, nullptr, screenSizeBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create convolution shader. Creation of screen size constant buffer failed.");
    }
}
