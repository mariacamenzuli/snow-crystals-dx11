#pragma once

#include "../3D Components/Scene.h"
#include "ModelLoader.h"
#include "../Util/UserInputReader.h"
#include "../3D Components/Models/ProcedurallyGeneratedTerrain.h"

class GeneratedLevelScene : public Scene {
public:
    GeneratedLevelScene();
    ~GeneratedLevelScene();

    SceneObject* getRootSceneObject() override;
    D3DXVECTOR4 getAmbientLight() override;
    PointLight* getPointLight() override;
    void update(float deltaTime);
    void mutateTerrain();

private:
    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;
    ProcedurallyGeneratedTerrain* terrainModel;
};
