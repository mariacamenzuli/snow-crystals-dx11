#pragma once

#include <D3DX10.h>

class PointLight {
public:
    PointLight(D3DXVECTOR4 diffuse, D3DXVECTOR4 specular);
    ~PointLight();

    D3DXVECTOR4 getDiffuse();
    D3DXVECTOR4 getSpecular();
    D3DXMATRIX* getWorldMatrix();
    D3DXMATRIX* getProjectionMatrix();

    void translate(float x, float y, float z);
    void rotateX(float angleInRadians);
    void rotateY(float angleInRadians);
    void rotateZ(float angleInRadians);

    D3DXVECTOR3 getPosition();

    D3DXMATRIX getViewMatrixPositiveX();
    D3DXMATRIX getViewMatrixNegativeX();
    D3DXMATRIX getViewMatrixPositiveY();
    D3DXMATRIX getViewMatrixNegativeY();
    D3DXMATRIX getViewMatrixPositiveZ();
    D3DXMATRIX getViewMatrixNegativeZ();

private:
    D3DXVECTOR4 diffuse;
    D3DXVECTOR4 specular;
    D3DXMATRIX worldMatrix;
    D3DXMATRIX projectionMatrix;
};
