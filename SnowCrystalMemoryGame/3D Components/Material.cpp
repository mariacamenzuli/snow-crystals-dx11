#include "Material.h"

Material::Material(D3DXVECTOR4 ambientColor,
                   D3DXVECTOR4 diffuseColor,
                   D3DXVECTOR4 specularColor) : ambientColor(ambientColor),
                                                diffuseColor(diffuseColor),
                                                specularColor(specularColor),
                                                textureFileName("") {
}

Material::Material(D3DXVECTOR4 ambientColor,
                   D3DXVECTOR4 diffuseColor,
                   D3DXVECTOR4 specularColor,
                   std::string textureFileName) : ambientColor(ambientColor),
                                                  diffuseColor(diffuseColor),
                                                  specularColor(specularColor),
                                                  textureFileName(textureFileName) {
}

Material::~Material() = default;

D3DXVECTOR4 Material::getAmbientColor() const {
    return ambientColor;
}

D3DXVECTOR4 Material::getDiffuseColor() const {
    return diffuseColor;
}

D3DXVECTOR4 Material::getSpecularColor() const {
    return specularColor;
}

std::string Material::getTextureFileName() const {
    return textureFileName;
}

bool Material::isTextured() const {
    return textureFileName != "";
}
