#include "Texture.h"

#include <D3DX11tex.h>

Texture::Texture(ID3D11Device* device, std::string filename) {
    HRESULT result = D3DX11CreateShaderResourceViewFromFile(device, std::wstring(filename.begin(), filename.end()).c_str(), nullptr, nullptr, textureResource.GetAddressOf(), nullptr);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to load texture [" + filename + "]");
    }
}

Texture::~Texture() = default;

ID3D11ShaderResourceView** Texture::getTextureResource() {
    return textureResource.GetAddressOf();
}
