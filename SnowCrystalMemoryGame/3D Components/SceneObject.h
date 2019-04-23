#pragma once

#include <D3DX10.h>
#include <memory>
#include <vector>
#include <map>

#include "Models/Model.h"

class SceneObject {
public:
    SceneObject();
    SceneObject(Model* model);
    ~SceneObject();

    Model* getModel();
    D3DXMATRIX* getWorldMatrix();
    SceneObject* attachChild(std::unique_ptr<SceneObject> child);
    SceneObject* attachChild(std::unique_ptr<SceneObject> child, std::string label);
    SceneObject* getChild(std::string label);
    std::vector<SceneObject*> getChildren();
    bool isVisible();
    
    void hide();
    void show();
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);
    void rotateX(float angleInRadians);
    void rotateY(float angleInRadians);
    void rotateZ(float angleInRadians);

private:
    bool visible = true;
    Model* model = nullptr;
    D3DXMATRIX worldMatrix;
    SceneObject* parent = nullptr;
    std::vector<std::unique_ptr<SceneObject>> children;
    std::map<std::string, SceneObject*> labeledChildren;
};
