#pragma once

#include <D3D11.h>
#include <wrl.h>
#include <string>

class Texture {
public:
    Texture(ID3D11Device* device, std::string filename);
    ~Texture();

    ID3D11ShaderResourceView** getTextureResource();

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureResource;
};
