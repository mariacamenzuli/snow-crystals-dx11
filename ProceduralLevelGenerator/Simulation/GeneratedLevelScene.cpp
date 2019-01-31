#include "GeneratedLevelScene.h"

GeneratedLevelScene::GeneratedLevelScene() : pointLight(D3DXVECTOR4(1.0f, 1.0f, 0.9f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 0.4f, 1.0f)) {
    rootSceneObject.reset(new SceneObject());

    auto world = rootSceneObject->attachChild(std::make_unique<SceneObject>(), "world");

    terrainModel = modelLoader.createProceduralTerrain(100, 100);
    auto terrain = world->attachChild(std::make_unique<SceneObject>(terrainModel), "terrain");
    terrain->translate(-50.0f, -9.0f, -50.0f);

    pointLight.translate(0.0f, 20.0f, 0.0f);
}

GeneratedLevelScene::~GeneratedLevelScene() = default;

SceneObject* GeneratedLevelScene::getRootSceneObject() {
    return rootSceneObject.get();
}

D3DXVECTOR4 GeneratedLevelScene::getAmbientLight() {
    return D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
}

PointLight* GeneratedLevelScene::getPointLight() {
    return &pointLight;
}

void GeneratedLevelScene::update(float deltaTime) {
    //no-op
}

void GeneratedLevelScene::mutateTerrain() {
    terrainModel->mutate();
}
