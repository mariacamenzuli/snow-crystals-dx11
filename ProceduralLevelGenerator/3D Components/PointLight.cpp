#include "PointLight.h"

PointLight::PointLight(const D3DXVECTOR4 diffuse, const D3DXVECTOR4 specular) : diffuse(diffuse), specular(specular) {
    D3DXMatrixIdentity(&worldMatrix);

    // Setup field of view and screen aspect for a square light source.
    float fieldOfView = static_cast<float>(D3DX_PI) / 2.0f; // 90 degree field of view
    float screenAspect = 1.0f;

    D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfView, screenAspect, 0.1f, 1000.0f);
};

PointLight::~PointLight() = default;

D3DXVECTOR4 PointLight::getDiffuse() {
    return diffuse;
}

D3DXVECTOR4 PointLight::getSpecular() {
    return specular;
}

D3DXMATRIX* PointLight::getWorldMatrix() {
    return &worldMatrix;
}

void PointLight::translate(float x, float y, float z) {
    D3DXMATRIX translationMatrix;
    D3DXMatrixTranslation(&translationMatrix, x, y, z);
    D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &translationMatrix);
}

void PointLight::rotateX(float angleInRadians) {
    D3DXMATRIX rotationMatrix;
    D3DXMatrixRotationX(&rotationMatrix, angleInRadians);
    D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotationMatrix);
}

void PointLight::rotateY(float angleInRadians) {
    D3DXMATRIX rotationMatrix;
    D3DXMatrixRotationY(&rotationMatrix, angleInRadians);
    D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotationMatrix);
}

void PointLight::rotateZ(float angleInRadians) {
    D3DXMATRIX rotationMatrix;
    D3DXMatrixRotationZ(&rotationMatrix, angleInRadians);
    D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotationMatrix);
}

D3DXVECTOR3 PointLight::getPosition() {
    D3DXVECTOR3 position3 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    D3DXVECTOR4 position4;
    D3DXVec3Transform(&position4, &position3, &worldMatrix);
    position3.x = position4.x / position4.w;
    position3.y = position4.y / position4.w;
    position3.z = position4.z / position4.w;

    return position3;
}

D3DXMATRIX PointLight::getViewMatrixPositiveX() {
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.x += 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX PointLight::getViewMatrixNegativeX() {
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    
    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.x -= 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX PointLight::getViewMatrixPositiveY() {
    D3DXVECTOR3 up(0.0f, 0.0f, -1.0f);

    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.y += 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX PointLight::getViewMatrixNegativeY() {
    D3DXVECTOR3 up(0.0f, 0.0f, 1.0f);

    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.y -= 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX PointLight::getViewMatrixPositiveZ() {
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    
    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.z += 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX PointLight::getViewMatrixNegativeZ() {
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
   
    D3DXVECTOR3 position = getPosition();
    D3DXVECTOR3 lookAtPoint = D3DXVECTOR3(position.x, position.y, position.z);
    lookAtPoint.z -= 1;

    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAtPoint, &up);

    return viewMatrix;
}

D3DXMATRIX* PointLight::getProjectionMatrix() {
    return &projectionMatrix;
}
