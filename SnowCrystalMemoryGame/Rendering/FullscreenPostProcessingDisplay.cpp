#include "FullscreenPostProcessingDisplay.h"
#include <stdexcept>

FullscreenPostProcessingDisplay::FullscreenPostProcessingDisplay() {
}

FullscreenPostProcessingDisplay::~FullscreenPostProcessingDisplay() {
}

void FullscreenPostProcessingDisplay::initialize(ID3D11Device* device, int windowWidth, int windowHeight) {
    float left, right, top, bottom;

    // Calculate the screen coordinates of the render window.
    left = (float) ((windowWidth / 2) * -1);
    right = left + (float) windowWidth;
    top = (float) (windowHeight / 2);
    bottom = top - (float) windowHeight;

    // First triangle
    vertices[0].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
    vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
    vertices[1].texture = D3DXVECTOR2(1.0f, 1.0f);

    vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
    vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

    // Second triangle.
    vertices[3].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
    vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[4].position = D3DXVECTOR3(right, top, 0.0f);  // Top right.
    vertices[4].texture = D3DXVECTOR2(1.0f, 0.0f);

    vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
    vertices[5].texture = D3DXVECTOR2(1.0f, 1.0f);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;
    indices[4] = 4;
    indices[5] = 5;

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(DisplayVertex) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create vertex buffer for post processing display.");
    }

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create index buffer for post processing display.");
    }
}

void FullscreenPostProcessingDisplay::setDisplayGeometryBuffersForIA(ID3D11DeviceContext* deviceContext) {
    unsigned int stride;
    unsigned int offset = 0;
    stride = sizeof(DisplayVertex);

    deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
