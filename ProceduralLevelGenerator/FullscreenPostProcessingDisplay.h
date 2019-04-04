#pragma once

#include <wrl/client.h>
#include <D3D11.h>

#include "3D Components/Models/Model.h"

class FullscreenPostProcessingDisplay {
public:
    FullscreenPostProcessingDisplay();
    ~FullscreenPostProcessingDisplay();

    void initialize(ID3D11Device* device, int windowWidth, int windowHeight);
    void setDisplayGeometryBuffersForIA(ID3D11DeviceContext* deviceContext);

private:
    struct DisplayVertex {
        D3DXVECTOR3 position;
        D3DXVECTOR2 texture;
    };

    const int vertexCount = 6;
    const int indexCount = 6;

    DisplayVertex vertices[6];
    unsigned long indices[36];

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
};
