#pragma once

#include <D3DX10.h>
#include <string>

class Material {
public:
    Material(D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR4 specularColor);
    Material(D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR4 specularColor, std::string textureFileName);
    ~Material();

    D3DXVECTOR4 getAmbientColor() const;
    D3DXVECTOR4 getDiffuseColor() const;
    D3DXVECTOR4 getSpecularColor() const;
    std::string getTextureFileName() const;
    bool isTextured() const;

private:
    D3DXVECTOR4 ambientColor;
    D3DXVECTOR4 diffuseColor; // used if there is no diffuse texture
    D3DXVECTOR4 specularColor;
    std::string textureFileName;
};